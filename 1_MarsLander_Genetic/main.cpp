// Include standard headers
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
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

void status(HANDLE &hConsole, const bool boolean) {
  SetConsoleTextAttribute(hConsole, (boolean ? 2 : 12));
  std::cout << (boolean ? "ON" : "OFF");
  SetConsoleTextAttribute(hConsole, 15);
}
void result(HANDLE &hConsole, const bool boolean) {
  SetConsoleTextAttribute(hConsole, (boolean ? 2 : 12));
  std::cout << (boolean ? "GOOD" : "FAILURE");
  SetConsoleTextAttribute(hConsole, 15);
}

// #######################################################
//
//                        GENETIC
//
// #######################################################

bool solve(const Rocket &rocket, const int *level, const int size_level,
           const bool visualize, const bool verbose, double &elapsedSec,
           int &fuelLeft) {
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
    if (elapsed_seconds.count() > 0.15) {
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

int main() {
  std::srand(time(NULL));

  const Levels levels;

  bool withVisu = true;
  bool verbose = false;

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

    // ...................................................
    //                      TITLE
    // ...................................................
    SetConsoleTextAttribute(hConsole, 9); // Blue color
    std::cout << " __       __                                      __         "
                 "                        __\n"
              << "|  \\     /  \\                                    |  \\     "
                 "                          |  \\\n"
              << "| $$\\   /  $$  ______    ______    _______       | $$       "
                 "______   _______    ____| $$  ______    ______\n"
              << "| $$$\\ /  $$$ |      \\  /      \\  /       \\      | $$    "
                 "  |      \\ |       \\  /      $$ /      \\  /      \\\n"
              << "| $$$$\\  $$$$  \\$$$$$$\\|  $$$$$$\\|  $$$$$$$      | $$    "
                 "   \\$$$$$$\\| $$$$$$$\\|  $$$$$$$|  $$$$$$\\|  $$$$$$\\\n"
              << "| $$\\$$ $$ $$ /      $$| $$   \\$$ \\$$    \\       | $$    "
                 "  /      $$| $$  | $$| $$  | $$| $$    $$| $$   \\$$\n"
              << "| $$ \\$$$| $$|  $$$$$$$| $$       _\\$$$$$$\\      | "
                 "$$_____|  $$$$$$$| $$  | $$| $$__| $$| $$$$$$$$| $$\n"
              << "| $$  \\$ | $$ \\$$    $$| $$      |       $$      | $$     "
                 "\\\\$$    $$| $$  | $$ \\$$    $$ \\$$     \\| $$\n"
              << " \\$$      \\$$  \\$$$$$$$ \\$$       \\$$$$$$$        "
                 "\\$$$$$$$$ \\$$$$$$$ \\$$   \\$$  \\$$$$$$$  \\$$$$$$$ \\$$\n"
              << "                      ______                                 "
                 " __      __\n"
              << "                     /      \\                               "
                 " |  \\    |  \\\n"
              << "                    |  $$$$$$\\  ______   _______    ______  "
                 "_| $$_    \$$  _______\n"
              << "                    | $$ __\\$$ /      \\ |       \\  /      "
                 "\\|   $$ \\  |  \\ /       \\\n"
              << "                    | $$|    \\|  $$$$$$\\| $$$$$$$\\|  "
                 "$$$$$$\\\\$$$$$$  | $$|  $$$$$$$\n"
              << "                    | $$ \\$$$$| $$    $$| $$  | $$| $$    "
                 "$$ | $$ __ | $$| $$\n"
              << "                    | $$__| $$| $$$$$$$$| $$  | $$| $$$$$$$$ "
                 "| $$|  \\| $$| $$_____\n"
              << "                     \\$$    $$ \\$$     \\| $$  | $$ \\$$   "
                 "  \\  \\$$  $$| $$ \\$$     \\\n"
              << "                      \\$$$$$$   \\$$$$$$$ \\$$   \\$$  "
                 "\\$$$$$$$   \\$$$$  \\$$  \\$$$$$$$\n\n";
    SetConsoleTextAttribute(hConsole, 15); // White color

    // ...................................................
    //                  DISPLAY MESSAGE
    // ...................................................
    if (message.size() > 0) {
      // Status update
      if (message[0] == '=') {
        SetConsoleTextAttribute(hConsole, 11); // Turquoise color
        std::cout << message << std::endl;
        SetConsoleTextAttribute(hConsole, 15); // White color
      }
      // Wrong input
      else if (message[0] == '/') {
        SetConsoleTextAttribute(hConsole, 12); // Red color
        std::cout << message << std::endl;
        SetConsoleTextAttribute(hConsole, 15); // White color
      }
      // Test result
      else {
        std::cout << "TEST RESULTS:" << std::endl;
        // Single test
        if (message.size() == 1) {
          std::cout << "  Test on ";
          SetConsoleTextAttribute(hConsole, 11); // Turquoise color
          std::cout << "level " << idxLevel;
          SetConsoleTextAttribute(hConsole, 15); // White color
          std::cout << ": [";
          result(hConsole, message == "Y");
          std::cout << "] - ";
          if (elapsed[0] == -1) {
            SetConsoleTextAttribute(hConsole, 12); // Red color
            std::cout << "Error on OpenGL Init...";
          } else {
            SetConsoleTextAttribute(hConsole, 13); // Magenta color
            std::cout << fuels[0] << "L";
            SetConsoleTextAttribute(hConsole, 15); // White color
            std::cout << " of fuel left - ";
            SetConsoleTextAttribute(hConsole, 14); // Yellow color
            std::cout << elapsed[0] << "s";
          }
          SetConsoleTextAttribute(hConsole, 15); // White color
          std::cout << std::endl;
        }
        // Full test
        else {
          for (int i = 0; i < message.size(); ++i) {
            std::cout << "  Test on ";
            SetConsoleTextAttribute(hConsole, 11); // Turquoise color
            std::cout << "level " << i + 1;
            SetConsoleTextAttribute(hConsole, 15); // White color
            std::cout << ": [";
            result(hConsole, message[i] == 'Y');
            std::cout << "] - ";
            if (elapsed[i] == -1) {
              SetConsoleTextAttribute(hConsole, 12); // Red color
              std::cout << "Error on OpenGL Init...";
            } else {
              SetConsoleTextAttribute(hConsole, 13); // Magenta color
              std::cout << fuels[i] << "L";
              SetConsoleTextAttribute(hConsole, 15); // White color
              std::cout << " of fuel left - ";
              SetConsoleTextAttribute(hConsole, 14); // Yellow color
              std::cout << elapsed[i] << "s";
            }
            SetConsoleTextAttribute(hConsole, 15); // White color
            std::cout << std::endl;
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
    std::cout << "  -        '0': Run the algorithm on all the levels"
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
        if (idxLevel < 8) {
          elapsed.clear();
          fuels.clear();
          double elapsedSec;
          int fuel;
          // User wants full test
          if (idxLevel == 0) {
            for (int i = 1; i < 8; ++i) {
              const Rocket rocket = levels.getRocket(i);
              const std::vector<int> floor = levels.getFloor(i);
              const int size_level = levels.getSizeFloor(i);

              std::cout << "Testing on ";
              SetConsoleTextAttribute(hConsole, 11); // Turquoise color
              std::cout << "level " << i;
              SetConsoleTextAttribute(hConsole, 15); // White color
              std::cout << "... ";

              bool isSolved = solve(rocket, floor.data(), size_level, withVisu,
                                    verbose, elapsedSec, fuel);

              std::cout << "[";
              result(hConsole, isSolved);
              std::cout << "]" << std::endl;

              elapsed.push_back(elapsedSec);
              fuels.push_back(fuel);
              if (isSolved) {
                message += "Y";
              } else if (elapsedSec == -1) {
                message += "?";
              } else {
                message += "N";
              }
            }
          }
          // User wants single test
          else {
            const Rocket rocket = levels.getRocket(idxLevel);
            const std::vector<int> floor = levels.getFloor(idxLevel);
            const int size_level = levels.getSizeFloor(idxLevel);

            std::cout << "Testing on ";
            SetConsoleTextAttribute(hConsole, 11); // Turquoise color
            std::cout << "level " << idxLevel;
            SetConsoleTextAttribute(hConsole, 15); // White color
            std::cout << "... " << std::endl;

            bool isSolved = solve(rocket, floor.data(), size_level, withVisu,
                                  verbose, elapsedSec, fuel);

            elapsed.push_back(elapsedSec);
            fuels.push_back(fuel);
            if (isSolved) {
              message += "Y";
            } else if (elapsedSec == -1) {
              message += "?";
            } else {
              message += "N";
            }
          }
          continue;
        }
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
