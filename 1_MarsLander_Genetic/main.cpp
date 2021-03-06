// Include standard headers
#include <chrono>
#include <iostream>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

// WinApi header
#include <windows.h>

// Include MarsLander headers
#include "Genetic.hpp"
#include "Rocket.hpp"
#include "Utils.hpp"
#include "levels.hpp"
#include "visualize.hpp"

extern const float _w{6999.f}; //!< Map width.
extern const float _h{2999.f}; //!< Map height.

extern bool _pause; //!< Play/pause status of the program.
extern bool _close; //!< Close or not the main window.

#define GREEN(hConsole) SetConsoleTextAttribute(hConsole, 2);
#define BLUE(hConsole) SetConsoleTextAttribute(hConsole, 9);
#define TURQUOISE(hConsole) SetConsoleTextAttribute(hConsole, 11);
#define RED(hConsole) SetConsoleTextAttribute(hConsole, 12);
#define MAGENTA(hConsole) SetConsoleTextAttribute(hConsole, 13);
#define YELLOW(hConsole) SetConsoleTextAttribute(hConsole, 14);
#define WHITE(hConsole) SetConsoleTextAttribute(hConsole, 15);

void status(HANDLE &hConsole, const bool boolean) {
  SetConsoleTextAttribute(hConsole, (boolean ? 2 : 12));
  std::cout << (boolean ? "ON" : "OFF");
  WHITE(hConsole)
}
void result(HANDLE &hConsole, const bool boolean) {
  SetConsoleTextAttribute(hConsole, (boolean ? 2 : 12));
  std::cout << (boolean ? "GOOD" : "FAILURE");
  WHITE(hConsole)
}

// #######################################################
//
//                        GENETIC
//
// #######################################################

bool solve(const Rocket &rocket, const int *level, const int size_level,
           const bool visualize, const bool verbose, const int timer,
           double &elapsedSec, int &fuelLeft) {
  // -----------------------------------------------------
  //                   INITIALIZATION
  // -----------------------------------------------------

  // Rocket, level and size_level are defined in `level.hpp`
  GeneticPopulation population(rocket, level, size_level);

  Visualization_OpenGL *visualization = Visualization_OpenGL::GetInstance();

  if (visualization->initOpenGL(visualize) != 0) {
    elapsedSec = -1;
    return false;
  }
  visualization->set(rocket, level, size_level, visualize);

  bool solutionFound{false};
  int generation{0};
  int idxChromosome{0};
  int idxGene{0};
  int prevGeneration{0};

  std::vector<Gene> solutionIncremental;

  std::chrono::high_resolution_clock::time_point start{
      std::chrono::high_resolution_clock::now()};

  std::chrono::high_resolution_clock::time_point start_loop{
      std::chrono::high_resolution_clock::now()};
  int idxStart{0};

  // -----------------------------------------------------
  //                  MAIN GENETIC LOOP
  // -----------------------------------------------------
  while (!solutionFound && !_close &&
         glfwWindowShouldClose(visualization->getWindow()) == 0) {
    if (verbose)
      std::cout << "Generation " << generation << std::endl;

    // ...................................................
    //                 INCREMENTAL SEARCH
    // ...................................................
    std::chrono::duration<double> elapsed_seconds{
        std::chrono::high_resolution_clock::now() - start_loop};
    if (timer > 0 && elapsed_seconds.count() > timer) {
      Gene *bestGen{population.getChromosome(0)->getGene(idxStart)};

      if (verbose) {
        std::cout << "Approx done at generation " << generation << " - "
                  << generation - prevGeneration << std::endl;
        std::cout << "Idx: " << idxStart << std::endl;
        std::cout << "  Angle: " << (int)bestGen->angle << std::endl;
        std::cout << "  Power: " << (int)bestGen->thrust << std::endl;
      }

      population.rocket_save.updateRocket(bestGen->angle, bestGen->thrust);
      if (verbose)
        std::cout << (int)population.rocket_save.angle << " "
                  << (int)population.rocket_save.thrust << std::endl;

      solutionIncremental.push_back({bestGen->angle, bestGen->thrust});

      const Line_d prev_curr{
          {population.rocket_save.pX, population.rocket_save.pY},
          {population.rocket_save.x, population.rocket_save.y}};
      for (int k = 1; k < size_level; ++k) {
        const Line_d floor{{level[2 * (k - 1)], level[2 * (k - 1) + 1]},
                           {level[2 * k], level[2 * k + 1]}};
        if (population.rocket_save.x < 0 || population.rocket_save.x > _w ||
            population.rocket_save.y < 0 || population.rocket_save.y > _h ||
            isIntersect(prev_curr, floor)) {
          population.rocket_save.isAlive = false;
        }
      }

      if (!population.rocket_save.isAlive)
        break;

      prevGeneration = generation;
      idxStart++;
      start_loop = std::chrono::high_resolution_clock::now();
    }

    generation++;

    // ...................................................
    //         ONE POPULATION: from birth to death
    // ...................................................
    population.initRockets();

    // For every Rocket and their associated chromosome
    for (int chrom = 0; !solutionFound && chrom < _POPULATION_SIZE; ++chrom) {
      // For every possible moves, i.e., for every genes
      for (int gen = idxStart; !solutionFound && gen < _CHROMOSOME_SIZE;
           ++gen) {
        Rocket *rocket{population.getRocket(chrom)};
        if (rocket->isAlive) {
          Gene *gene{population.getChromosome(chrom)->getGene(gen)};

          rocket->updateRocket(gene->angle, gene->thrust);

          const Line_d prev_curr{{rocket->pX, rocket->pY},
                                 {rocket->x, rocket->y}};
          for (int k = 1; k < size_level; ++k) {
            const Line_d floor{{level[2 * (k - 1)], level[2 * (k - 1) + 1]},
                               {level[2 * k], level[2 * k + 1]}};
            if (rocket->x < 0 || rocket->x > _w || rocket->y < 0 ||
                rocket->y > _h) {
              rocket->isAlive = false;
              rocket->floor_id_crash = -1;
            } else if (isIntersect(prev_curr, floor)) {
              rocket->isAlive = false;
              rocket->floor_id_crash = k;

              // Landing successful!
              if (k == population.landing_zone_id && rocket->isParamSuccess()) {
                if (verbose)
                  std::cout << "Landing SUCCESS!" << std::endl << std::endl;
                solutionFound = true;
                idxChromosome = chrom;
                idxGene = gen;
                fuelLeft = rocket->fuel;
              }
              break;
            }
          }
        }
        visualization->updateRocketLine(rocket, gen, chrom);
      }
    }

    // ...................................................
    //                     MUTATION
    // ...................................................
    if (!solutionFound)
      population.mutate(idxStart);
    else
      break;

    // ...................................................
    //                  OPENGL DISPLAY
    // ...................................................
    visualization->drawPopulation();
  }

  std::chrono::duration<double> elapsed_seconds{
      std::chrono::high_resolution_clock::now() - start};
  elapsedSec = elapsed_seconds.count();
  if (verbose)
    std::cout << "Execution time: " << elapsedSec << "s" << std::endl;

  // -----------------------------------------------------
  //                 DISPLAY THE SOLUTION
  // -----------------------------------------------------
  if (visualize && solutionFound) {
    if (verbose) {
      std::cout << std::endl;
      std::cout << "Solution found at:" << std::endl;
      std::cout << "  Generation: " << generation << std::endl;
      std::cout << "  Child: " << idxChromosome << std::endl;
      std::cout << "  Gene: " << idxGene << std::endl << std::endl;
    }

    Rocket rocket_res{rocket};

    Chromosome *chromosome{population.getChromosome(idxChromosome)};

    const int number_loop_within_1_sec{50};
    double number_loop{1};
    int number_second{0};
    if (verbose)
      rocket_res.debug();

    // Check if the ESC key was pressed or the window was closed
    while (!_close && rocket_res.isAlive &&
           glfwWindowShouldClose(visualization->getWindow()) == 0) {

      if (!_pause) {
        if (number_loop >= number_loop_within_1_sec) {
          number_loop = 0;

          if (verbose)
            rocket_res.debug(number_second);

          if (number_second < idxStart) {
            rocket_res.updateRocket(solutionIncremental[number_second].angle,
                                    solutionIncremental[number_second].thrust);
          } else {
            rocket_res.updateRocket(chromosome->getGene(number_second)->angle,
                                    chromosome->getGene(number_second)->thrust);
          }

          if (verbose)
            std::cout << "  Request: " << (int)rocket_res.angle << " "
                      << (int)rocket_res.thrust << std::endl
                      << std::endl;

          if (verbose)
            rocket_res.debug();
          number_second++;
        }

        number_loop++;
        visualization->updateSingleRocket(
            rocket_res, number_loop / number_loop_within_1_sec);

        const Line_d prev_curr{{rocket_res.pX, rocket_res.pY},
                               {rocket_res.x, rocket_res.y}};
        for (int k = 1; k < size_level; ++k) {
          const Line_d floor{{level[2 * (k - 1)], level[2 * (k - 1) + 1]},
                             {level[2 * k], level[2 * k + 1]}};
          if (rocket_res.x < 0 || rocket_res.x > _w || rocket_res.y < 0 ||
              rocket_res.y > _h || isIntersect(prev_curr, floor)) {
            rocket_res.isAlive = false;
            break;
          }
        }
      }

      visualization->drawSingleRocket();
    }
  }

  return solutionFound;
}

// #######################################################
//
//                        MAIN
//
// #######################################################

// -----------------------------------------------------
//                         TITLE
// -----------------------------------------------------
void title(HANDLE &hConsole) {
  WHITE(hConsole)
  std::cout << " __       __                                      __       "
               "               "
               "           __\n|  \\     /  \\                             "
               "       |  \\   "
               "                            |  \\\n| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "\\   /  ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  ______    ______    _______       | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "       ______   _______    ____| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  ______    ______\n| ";
  BLUE(hConsole) std::cout << "$$$";
  WHITE(hConsole) std::cout << "\\ /  ";
  BLUE(hConsole) std::cout << "$$$";
  WHITE(hConsole) std::cout << " |      \\  /      \\  /       \\      | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "      |      \\ |       \\  /      ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " /      \\  /      \\\n| ";
  BLUE(hConsole) std::cout << "$$$$";
  WHITE(hConsole) std::cout << "\\  ";
  BLUE(hConsole) std::cout << "$$$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\|  ";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\|  ";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "      | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "       \\";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\| ";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "\\|  ";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "|  ";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\|  ";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\\n| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "\\";
  BLUE(hConsole) std::cout << "$$ $$ $$";
  WHITE(hConsole) std::cout << " /      ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "   \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "    \\       | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "      /      ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$    $$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "   \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "\n| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "|  ";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "       _\\";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\      | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "_____|  ";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "__| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$$$$$$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "\n| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$";
  WHITE(hConsole) std::cout << " | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$    $$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "      |       ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "      | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "     \\\\";
  BLUE(hConsole) std::cout << "$$    $$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$    $$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "     \\| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "\n \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "      \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "       \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "        \\";
  BLUE(hConsole) std::cout << "$$$$$$$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "   \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole)
  std::cout << "\n                      ______                             "
               "     __      "
               "__\n                     /      \\                         "
               "       |  \\   "
               " |  \\\n                    |  ";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\  ______   _______    ______  _| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "_     ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  _______\n                    | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " __\\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " /      \\ |       \\  /      \\|   ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\  |  \\ /       \\\n                    | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "|    \\|  ";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\| ";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "\\|  ";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "\\\\";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "|  ";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "\n                    | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$    $$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$    $$";
  WHITE(hConsole) std::cout << " | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " __ | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "\n                    | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "__| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$$$$$$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$$$$$$$";
  WHITE(hConsole) std::cout << " | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "|  \\| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "_____\n                     \\";
  BLUE(hConsole) std::cout << "$$    $$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "     \\| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  | ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "     \\  \\";
  BLUE(hConsole) std::cout << "$$  $$";
  WHITE(hConsole) std::cout << "| ";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "     \\\n                      \\";
  BLUE(hConsole) std::cout << "$$$$$$";
  WHITE(hConsole) std::cout << "   \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << " \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "   \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "   \\";
  BLUE(hConsole) std::cout << "$$$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$$";
  WHITE(hConsole) std::cout << "  \\";
  BLUE(hConsole) std::cout << "$$$$$$$";
  WHITE(hConsole) std::cout << "\n\n";
}

int main() {
  std::srand(time(NULL));

  const Levels levels;

  bool withVisu = true;
  bool verbose = false;

  double timer = 0.15;

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

  std::string message;
  std::vector<double> elapsed;
  std::vector<int> fuels;
  int idxLevel;

  // -----------------------------------------------------
  //                TOOL INFINITE LOOP
  // -----------------------------------------------------
  while (1) {
    system("CLS");

    // Display the title
    title(hConsole);

    // ...................................................
    //                  DISPLAY MESSAGE
    // ...................................................
    if (message.size() > 0) {
      // Status update
      if (message[0] == '=') {
        TURQUOISE(hConsole)
        std::cout << message << std::endl;
        WHITE(hConsole)
      }
      // Wrong input
      else if (message[0] == '/') {
        RED(hConsole)
        std::cout << message << std::endl;
        WHITE(hConsole)
      }
      // Test result
      else {
        std::cout << "TESTS RESULTS:" << std::endl;
        // Single test
        if (message.size() == 1) {
          std::cout << "  Test on ";
          TURQUOISE(hConsole)
          std::cout << "level " << idxLevel;
          WHITE(hConsole)
          std::cout << ": [";
          result(hConsole, message == "Y");
          std::cout << "] - ";
          if (elapsed[0] == -1) {
            RED(hConsole)
            std::cout << "Error on OpenGL Init...";
          } else {
            MAGENTA(hConsole)
            std::cout << fuels[0] << "L";
            WHITE(hConsole)
            std::cout << " of fuel left - ";
            YELLOW(hConsole)
            std::cout << elapsed[0] << "s";
          }
          WHITE(hConsole)
          std::cout << std::endl;
        }
        // Full test
        else {
          for (int i = 0; i < message.size(); ++i) {
            std::cout << "  Test on ";
            TURQUOISE(hConsole)
            std::cout << "level " << i + 1;
            WHITE(hConsole)
            std::cout << ": [";
            result(hConsole, message[i] == 'Y');
            std::cout << "] - ";
            if (elapsed[i] == -1) {
              RED(hConsole)
              std::cout << "Error on OpenGL Init...";
            } else {
              MAGENTA(hConsole)
              std::cout << fuels[i] << "L";
              WHITE(hConsole)
              std::cout << " of fuel left - ";
              YELLOW(hConsole)
              std::cout << elapsed[i] << "s";
            }
            WHITE(hConsole)
            std::cout << std::endl;
          }
          if (message.size() == 5) {
            std::cout << "Optimization final score: ";
            MAGENTA(hConsole)
            double sumFuel = std::accumulate(fuels.begin(), fuels.end(), 0);
            std::cout << sumFuel << std::endl;
            WHITE(hConsole)
          }
        }
      }
      message = "";
    }

    // ...................................................
    //             DISPLAY CHOICES OF ACTIONS
    // ...................................................
    std::cout << "" << std::endl;
    std::cout << "Select an action: " << std::endl;
    std::cout << "  -        'Q': Quit the tool" << std::endl;
    std::cout << "  -        'D': Change the display status, current is [";
    status(hConsole, withVisu);
    std::cout << "] (it's faster when it's OFF)" << std::endl;
    std::cout << "  -        'V': Change the verbose status, current is [";
    status(hConsole, verbose);
    std::cout << "] ("
              << (verbose ? "Do you really know what you are doing?"
                          : "Please, don't change that")
              << ")" << std::endl;
    std::cout << "  -        'T': Timer (in sec) for the incremental search. 0 "
                 "to disable the incremental search, current is [";
    TURQUOISE(hConsole) std::cout << timer;
    WHITE(hConsole)
    std::cout << "]" << std::endl;
    std::cout << "  -        'F': Run the algorithm on all the levels"
              << std::endl;
    std::cout << "  -        'O': Run the algorithm on the optimization levels"
              << std::endl;
    std::cout << "  - '1' -> '7': Level to run the algorithm on" << std::endl;
    std::cout << std::endl;

    // ...................................................
    //                 READ THE USER INPUT
    // ...................................................
    std::string input;
    std::cout << "? ";
    std::cin >> input;

    // User pressed only 1 char
    if (input.size() == 1) {
      // User pressed a digit
      if (isdigit(input[0])) {
        idxLevel = input[0] - '0';
        // User pressed a valid digit
        if (0 < idxLevel && idxLevel < 8) {
          elapsed.clear();
          fuels.clear();
          const Rocket rocket = levels.getRocket(idxLevel);
          const std::vector<int> floor = levels.getFloor(idxLevel);
          const int size_level = levels.getSizeFloor(idxLevel);

          std::cout << "Testing on ";
          TURQUOISE(hConsole)
          std::cout << "level " << idxLevel;
          WHITE(hConsole)
          std::cout << "... " << std::endl;

          double elapsedSec;
          int fuel;
          bool isSolved = solve(rocket, floor.data(), size_level, withVisu,
                                verbose, timer, elapsedSec, fuel);

          elapsed.push_back(elapsedSec);
          fuels.push_back(fuel);
          if (isSolved) {
            message += "Y";
          } else if (elapsedSec == -1) {
            message += "?";
          } else {
            message += "N";
          }
          continue;
        }
      }
      // User wants a series of test
      else if (input == "F" || input == "f" || input == "O" || input == "o") {
        int nbLevel = (input == "F" || input == "f") ? 7 : 5;
        for (int i = 1; i <= nbLevel; ++i) {
          const Rocket rocket = levels.getRocket(i);
          const std::vector<int> floor = levels.getFloor(i);
          const int size_level = levels.getSizeFloor(i);

          std::cout << "Testing on ";
          TURQUOISE(hConsole)
          std::cout << "level " << i;
          WHITE(hConsole)
          std::cout << "... ";

          double elapsedSec;
          int fuel;
          bool isSolved = solve(rocket, floor.data(), size_level, withVisu,
                                verbose, timer, elapsedSec, fuel);
          elapsed.push_back(elapsedSec);
          if (isSolved) {
            message += "Y";
            fuels.push_back(fuel);
          } else if (elapsedSec == -1) {
            message += "?";
            fuels.push_back(0);
          } else {
            message += "N";
            fuels.push_back(0);
          }

          std::cout << "[";
          result(hConsole, isSolved);
          std::cout << "] - ";
          if (elapsedSec == -1) {
            RED(hConsole)
            std::cout << "Error on OpenGL Init...";
          } else {
            MAGENTA(hConsole)
            std::cout << fuels[i - 1] << "L";
            WHITE(hConsole)
            std::cout << " of fuel left - ";
            YELLOW(hConsole)
            std::cout << elapsed[i - 1] << "s";
          }
          WHITE(hConsole)
          std::cout << std::endl;
        }
        continue;
      }
      // User updates verbose status
      else if (input == "V" || input == "v") {
        message = "=> Verbose status successfully changed!";
        verbose = !verbose;
        continue;
      }
      // User updates display status
      else if (input == "D" || input == "d") {
        message = "=> Display status successfully changed!";
        withVisu = !withVisu;
        if (!withVisu)
          Visualization_OpenGL::GetInstance()->end();
        continue;
      }
      // User wants to update timer
      else if (input == "T" || input == "t") {
        while (1) {
          std::cout << "The timer should be ";
          GREEN(hConsole) std::cout << ">= 0";
          WHITE(hConsole) std::cout << ". Which timer do you want? ";
          std::cin >> input;
          try {
            double new_timer = std::stod(input);
            if (new_timer >= 0) {
              timer = new_timer;
              break;
            } else {
              std::cout << "The new timer should be ";
              RED(hConsole) std::cout << ">= 0";
              WHITE(hConsole) std::cout << " but is '";
              TURQUOISE(hConsole) std::cout << new_timer;
              WHITE(hConsole) std::cout << "'" << std::endl;
              continue;
            }
          } catch (...) {
            RED(hConsole) std::cout << "An error";
            WHITE(hConsole) std::cout << " occured with your input '";
            TURQUOISE(hConsole) std::cout << input;
            WHITE(hConsole) std::cout << "'" << std::endl;
            continue;
          }
        }
        message = "=> Timer successfully changed!";
        continue;
      }
      // User wants to quit the tool
      else if (input == "Q" || input == "q") {
        break;
      }
    }
    message = "/!\\ Invalid input: `" + input + "`... Please try again /!\\";
  }

  // -----------------------------------------------------
  //                END OF THE TOOL
  // -----------------------------------------------------
  Visualization_OpenGL::GetInstance()->end();

  return 0;
}
