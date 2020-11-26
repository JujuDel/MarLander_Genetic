// Include standard headers
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

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

// #######################################################
//
//                        MAIN
//
// #######################################################

int main() {
  // -----------------------------------------------------
  //               SOME INITIALIZATION
  // -----------------------------------------------------
  std::srand(time(NULL));

  // Rocket, level and size_level are defined in `level.hpp`
  GeneticPopulation population(rocket, level, size_level);

  Visualization_OpenGL visualization(rocket);

  if (visualization.initOpenGL() == -1)
    return -1;

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
         glfwWindowShouldClose(visualization.getWindow()) == 0) {
    std::cout << "Generation " << generation << std::endl;

    // ...................................................
    //                 INCREMENTAL SEARCH
    // ...................................................
    std::chrono::duration<double> elapsed_seconds{
        std::chrono::high_resolution_clock::now() - start_loop};
    if (elapsed_seconds.count() > 0.15) {
      Gene *bestGen{population.getChromosome(0)->getGene(idxStart)};

      std::cerr << "Approx done at generation " << generation << " - "
                << generation - prevGeneration << std::endl;
      std::cerr << "Idx: " << idxStart << std::endl;
      std::cerr << "  Angle: " << (int)bestGen->angle << std::endl;
      std::cerr << "  Power: " << (int)bestGen->thrust << std::endl;

      population.rocket_save.updateRocket(bestGen->angle, bestGen->thrust);
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
                std::cout << "Landing SUCCESS!" << std::endl << std::endl;
                solutionFound = true;
                idxChromosome = chrom;
                idxGene = gen;
              }
              break;
            }
          }
        }
        visualization.updateRocketLine(rocket, gen, chrom);
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
    visualization.drawPopulation();
  }

  std::chrono::duration<double> elapsed_seconds{
      std::chrono::high_resolution_clock::now() - start};
  std::cout << "Execution time: " << elapsed_seconds.count() << "s"
            << std::endl;

  // -----------------------------------------------------
  //                 DISPLAY THE SOLUTION
  // -----------------------------------------------------
  if (solutionFound) {
    std::cout << std::endl;
    std::cout << "Solution found at:" << std::endl;
    std::cout << "  Generation: " << generation << std::endl;
    std::cout << "  Child: " << idxChromosome << std::endl;
    std::cout << "  Gene: " << idxGene << std::endl << std::endl;

    Rocket rocket_res{rocket};

    Chromosome *chromosome{population.getChromosome(idxChromosome)};

    const int number_loop_within_1_sec{2};
    double number_loop{1};
    int number_second{0};
    rocket_res.debug();

    // Check if the ESC key was pressed or the window was closed
    while (!_close && glfwWindowShouldClose(visualization.getWindow()) == 0) {

      if (!_pause && rocket_res.isAlive) {
        if (number_loop >= number_loop_within_1_sec) {
          number_loop = 0;

          rocket_res.debug(number_second);

          if (number_second < idxStart) {
            rocket_res.updateRocket(solutionIncremental[number_second].angle,
                                    solutionIncremental[number_second].thrust);
          } else {
            rocket_res.updateRocket(chromosome->getGene(number_second)->angle,
                                    chromosome->getGene(number_second)->thrust);
          }
          std::cout << "  Request: " << (int)rocket_res.angle << " "
                    << (int)rocket_res.thrust << std::endl
                    << std::endl;

          rocket_res.debug();
          number_second++;
        }

        number_loop++;
        visualization.updateSingleRocket(
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

      visualization.drawSingleRocket();
    }
  }
  // No solution
  else {
    std::cout << "No solution found..." << std::endl;
  }

  visualization.end();

  return 0;
}
