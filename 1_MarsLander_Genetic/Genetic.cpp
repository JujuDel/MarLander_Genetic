
#include <algorithm>
#include <iostream>

#include "Genetic.hpp"

// #######################################################
//
//                        GENE
//
// #######################################################

Gene::Gene(const std::int8_t f_angle, const std::int8_t f_thrust)
    : angle{f_angle}, thrust{f_thrust} {}

// #######################################################
//
//                      CHROMOSOME
//
// #######################################################

//! @brief  Get a random angle in the range f_angle +/- 15.
//!         The value is clamped between -90 and 90.
//!
//! @param[in] f_angle  The current angle value.
//!
//! @return The next random angle value.
std::int8_t getRandAngle(const int f_angle) {
  const int maxRand{std::min(90 - f_angle, 15) + std::min(f_angle + 90, 15) +
                    1};

  return static_cast<std::int8_t>(rand() % maxRand -
                                  std::min(f_angle + 90, 15));
}

//! @brief  Get a random thrust power in the range f_thrust +/- 1.
//!         The value is clamped between 0 and 4.
//!
//! @param[in] f_thrust  The current thrust power value.
//!
//! @return The next random thrust power value.
std::int8_t getRandThrust(const int f_thrust) {
  const int maxRandPower{std::min(4 - f_thrust, 1) + std::min(f_thrust, 1) + 1};

  return static_cast<std::int8_t>(rand() % maxRandPower -
                                  std::min(f_thrust, 1));
}

/************************************************************/
Chromosome::Chromosome(const int f_angle, const int f_thrust) : fitness{0} {
  int angle = f_angle;
  int thrust = f_thrust;
  for (int i = 0; i < _CHROMOSOME_SIZE; ++i) {
    chromosome[i] = {getRandAngle(angle), getRandThrust(thrust)};
    angle += chromosome[i].angle;
    thrust += chromosome[i].thrust;
  }
}

/************************************************************/
Gene *Chromosome::getGene(const std::uint8_t i) {
  if (i < _CHROMOSOME_SIZE) {
    return &chromosome[i];
  }
  return nullptr;
}
/************************************************************/
const Gene *Chromosome::getGene(const std::uint8_t i) const {
  if (i < _CHROMOSOME_SIZE) {
    return &chromosome[i];
  }
  return nullptr;
}

/************************************************************/
bool Chromosome::chromosome_sorter(Chromosome const &lhs,
                                   Chromosome const &rhs) {
  return lhs.fitness > rhs.fitness;
}

// #######################################################
//
//                      POPULATION
//
// #######################################################

GeneticPopulation::GeneticPopulation(const Rocket &f_rocket,
                                     const int *f_floor_buffer,
                                     const int f_size_floor)
    : rocket_save{f_rocket}, floor_buffer{f_floor_buffer},
      size_floor{f_size_floor}, population{&populationA[0]},
      new_population{&populationB[0]} {
  for (int i = 1; i < size_floor; ++i) {
    // Statements constraints: Only 1 landing zone
    if (floor_buffer[2 * (i - 1) + 1] == floor_buffer[2 * i + 1]) {
      landing_zone_id = i;
      break;
    }
  }

  initRockets();
  initChromosomes();
}

/************************************************************/
void GeneticPopulation::initRockets() {
  for (int i = 0; i < _POPULATION_SIZE; ++i) {
    rockets_gen[i].init(rocket_save);
  }
}

/************************************************************/
void GeneticPopulation::initChromosomes() {
  for (int i = 0; i < _POPULATION_SIZE; ++i) {
    population[i] = Chromosome(rocket_save.angle, rocket_save.thrust);
  }
}

/************************************************************/
Chromosome *GeneticPopulation::getChromosome(const std::uint8_t i) {
  if (i < _POPULATION_SIZE) {
    return &population[i];
  }
  return nullptr;
}

/************************************************************/
Rocket *GeneticPopulation::getRocket(const std::uint8_t i) {
  if (i < _POPULATION_SIZE) {
    return &rockets_gen[i];
  }
  return nullptr;
}

//! @brief  Given a rocket, compute its distance fitness score.
//!
//! The socre is calculated following the formulae:
//!   f(d) = 1000. / (1 + 0.009999 * d)
//!
//!   f(0)      = 1000.000
//!   f(10)     =  909.099
//!   f(100)    =  500.025
//!   f(1000)   =   90.917
//!   f(10000)  =    9.902
//!   f(100000) =    0.999
//!
//! @param[in] f_thrust  The current thrust power value.
//!
//! @return The next random thrust power value.
double distance(const Rocket &rocket, const int *floor_buffer,
                const int landing_zone_id) {
  if (rocket.floor_id_crash == -1)
    return 0.;

  double dist{0.};
  // Crash distance
  if (rocket.floor_id_crash == landing_zone_id) {
    if (rocket.isParamSuccess()) {
      std::cout << "YOOOUUUUHHHHOOOOOUUUU" << std::endl;
      return 99999.;
    }
  } else if (rocket.floor_id_crash < landing_zone_id) {
    dist = sqrt(pow(rocket.x - floor_buffer[2 * rocket.floor_id_crash + 0], 2) +
                pow(rocket.y - floor_buffer[2 * rocket.floor_id_crash + 1], 2));
    for (int k = rocket.floor_id_crash + 1; k < landing_zone_id; ++k) {
      dist += sqrt(pow(static_cast<double>(floor_buffer[2 * (k - 1) + 0]) +
                           floor_buffer[2 * k + 0],
                       2) +
                   pow(static_cast<double>(floor_buffer[2 * (k - 1) + 1]) +
                           floor_buffer[2 * k + 1],
                       2));
    }
  } else {
    dist = sqrt(
        pow(rocket.x - floor_buffer[2 * (rocket.floor_id_crash - 1) + 0], 2) +
        pow(rocket.y - floor_buffer[2 * (rocket.floor_id_crash - 1) + 1], 2));

    for (int k = rocket.floor_id_crash - 1; k > landing_zone_id; --k) {
      dist += sqrt(pow(static_cast<double>(floor_buffer[2 * (k - 1) + 0]) +
                           floor_buffer[2 * k + 0],
                       2) +
                   pow(static_cast<double>(floor_buffer[2 * (k - 1) + 1]) +
                           floor_buffer[2 * k + 1],
                       2));
    }
  }
  return 1000. / (1. + 0.009999 * dist);
}

//! @brief  Given horizontal and vertical speeds, compute the fitness penalty.
//!
//! The fitness penalty is calculated following the formulaes:
//!   Horizontal speed: s >= 0 in m/s-1
//!     h(s) = 0.00036057692307692 * s^2 + 0.069711538461538 * s
//!            - 3.3653846153846
//!
//!     h(0)   = 3.3653846153846
//!     h(40)  = 0
//!     h(200) = -25
//!     h(300) = -50
//!
//!   Vertical speed: s >= 0 in m/s-1
//!     v(s) = 0.0003968253968254 * s ^ 2 + 0.051587301587302 * s
//!            - 1.1904761904762
//!
//!     v(0)   = 1.1904761904762
//!     v(25)  = 0
//!     v(200) = -25
//!     v(300) = -50
//!
//!   Speed fitness penalty: sx, sy >= 0 in m/s-1
//!     f(sx, sy) = -5 * (h(sx) + v(sy))
//!
//! @param[in] vx  The horizontal speed.
//! @param[in] vy  The vertical speed.
//!
//! @return The speed fitness penalty.
double speed(const double vx, const double vy) {
  double scoreX =
      0.00036057692307692 * vx * vx + 0.069711538461538 * vx - 3.3653846153846;
  double scoreY =
      0.0003968253968254 * vy * vy + 0.051587301587302 * vy - 1.1904761904762;

  return -5. * (scoreX + scoreY);
}

/************************************************************/
void GeneticPopulation::mutate(const int idxStart) {
  double sum_fitness{0.};
  // Compute every fitness
  for (int i = 0; i < _POPULATION_SIZE; ++i) {
    if (rockets_gen[i].isAlive) {
      population[i].fitness = 0;
    } else {
      population[i].fitness =
          distance(rockets_gen[i], floor_buffer, landing_zone_id);
    }

    if (rockets_gen[i].floor_id_crash == landing_zone_id) {
      population[i].fitness +=
          speed(abs(rockets_gen[i].vx), abs(rockets_gen[i].vy));
    }

    sum_fitness += population[i].fitness;
  }

  // Sort the fitness
  std::sort(population,
            population + sizeof(populationA) / sizeof(populationA[0]),
            &Chromosome::chromosome_sorter);

  double cum_sum{0.};
  for (int i = _POPULATION_SIZE - 1; i >= 0; --i) {
    population[i].fitness = cum_sum + population[i].fitness / sum_fitness;
    cum_sum = population[i].fitness;
  }

  // Elitism
  for (int i = 0; i < _ELITISM_IDX; ++i) {
    new_population[i] = population[i];
  }

  // Continuous Genetic Algorithm
  for (int i = _ELITISM_IDX; i < _POPULATION_SIZE; i += 2) {
    int idxParent1{_POPULATION_SIZE};
    while (idxParent1 == _POPULATION_SIZE) {
      idxParent1 = 1;
      const double choice1{rand() / static_cast<double>(RAND_MAX)};
      for (; idxParent1 < _POPULATION_SIZE; ++idxParent1) {
        if (population[idxParent1].fitness < choice1) {
          idxParent1--;
          break;
        }
      }
    }

    int idxParent2{_POPULATION_SIZE};
    while (idxParent2 == idxParent1 || idxParent2 == _POPULATION_SIZE) {
      idxParent2 = 1;
      const double choice2{rand() / static_cast<double>(RAND_MAX)};
      for (; idxParent2 < _POPULATION_SIZE; ++idxParent2) {
        if (population[idxParent2].fitness < choice2) {
          idxParent2--;
          break;
        }
      }
    }

    for (int g = idxStart; g < _CHROMOSOME_SIZE; ++g) {
      const double r{rand() / static_cast<double>(RAND_MAX)};
      if (r > _MUTATION_RATE) {
        const double angleP0 = population[idxParent1].chromosome[g].angle;
        const double angleP1 = population[idxParent2].chromosome[g].angle;
        const double powerP0 = population[idxParent1].chromosome[g].thrust;
        const double powerP1 = population[idxParent2].chromosome[g].thrust;

        new_population[i].chromosome[g].angle =
            static_cast<std::int8_t>(r * angleP0 + (1 - r) * angleP1);
        new_population[i].chromosome[g].thrust =
            static_cast<std::int8_t>(r * powerP0 + (1 - r) * powerP1);
        if (i != _POPULATION_SIZE - 1) {
          new_population[i + 1].chromosome[g].angle =
              static_cast<std::int8_t>((1 - r) * angleP0 + r * angleP1);
          new_population[i + 1].chromosome[g].thrust =
              static_cast<std::int8_t>((1 - r) * powerP0 + r * powerP1);
        }
      } else {
        new_population[i].chromosome[g].angle = getRandAngle(rocket_save.angle);
        new_population[i].chromosome[g].thrust =
            getRandThrust(rocket_save.thrust);
        if (i != _POPULATION_SIZE - 1) {
          new_population[i + 1].chromosome[g].angle =
              getRandAngle(rocket_save.angle);
          new_population[i + 1].chromosome[g].thrust =
              getRandThrust(rocket_save.thrust);
        }
      }
    }
  }

  Chromosome *tmp = population;
  population = new_population;
  new_population = tmp;
}
