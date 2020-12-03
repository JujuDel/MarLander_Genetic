
//#define CODING_GAME
#ifdef CODING_GAME

#define _USE_MATH_DEFINES

// Include standard headers
#include <algorithm>
#include <chrono>
#include <iostream>
#include <math.h>

// #######################################################
//
//                        UTILS
//
// #######################################################

struct Coord_d {
  double x;
  double y;
};

struct Line_d {
  Coord_d p1;
  Coord_d p2;
};

//! @brief  Apply the given rotation on the point `P`.
//!
//! @param[out] P  The point to update.
//! @param[in]  c  cos(angle).
//! @param[in]  v  sin(angle).
void applyRotation(Coord_d &P, const double c, const double s) {
  const double x{P.x};
  const double y{P.y};
  P.x = x * c - y * s;
  P.y = x * s + y * c;
}

bool onLine(Line_d l1, Coord_d p) {
  return (
      p.x <= std::max(l1.p1.x, l1.p2.x) && p.x >= std::min(l1.p1.x, l1.p2.x) &&
      (p.y <= std::max(l1.p1.y, l1.p2.y) && p.y >= std::min(l1.p1.y, l1.p2.y)));
}

int direction(Coord_d a, Coord_d b, Coord_d c) {
  double val{(b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y)};
  if (val == 0) {
    return 0; // colinear
  } else if (val < 0) {
    return 2; // anti-clockwise direction
  } else {
    return 1; // clockwise direction
  }
}

//! @brief  Given two lines, check if they intersect.
//!
//! @param[in] l1, l2  The lines to check.
//!
//! @return True if l1 and l2 intersects, else false.
bool isIntersect(Line_d l1, Line_d l2) {
  // Four direction for one lines and a point of the other line
  int dir1 = direction(l1.p1, l1.p2, l2.p1);
  int dir2 = direction(l1.p1, l1.p2, l2.p2);
  int dir3 = direction(l2.p1, l2.p2, l1.p1);
  int dir4 = direction(l2.p1, l2.p2, l1.p2);

  if (dir1 != dir2 && dir3 != dir4)
    return true; // they are intersecting

  if (dir1 == 0 && onLine(l1, l2.p1)) // when p2 of line2 is on line1
    return true;

  if (dir2 == 0 && onLine(l1, l2.p2)) // when p1 of line2 is on line1
    return true;

  if (dir3 == 0 && onLine(l2, l1.p1)) // when p2 of line1 is on line2
    return true;

  if (dir4 == 0 && onLine(l2, l1.p2)) // when p1 of line1 is on line2
    return true;

  return false;
}

// #######################################################
//
//                        ROCKET
//
// #######################################################

constexpr double _g{-3.711}; //!< Gravity, in m/s-2
constexpr float _w{6999.f};  //!< Map width.
constexpr float _h{2999.f};  //!< Map height.

//! @brief  Rocket's struct
struct Rocket {
  double pX, pY;      //!< Previous coordinates.
  double x, y;        //!< Current coordinates.
  double vx, vy;      //!< Rocket's velocity.
  double ax, ay;      //!< Rocket's acceleration.
  std::int8_t angle;  //!< Rocket's angle.
  std::int8_t thrust; //!< Rocket's thrust power.
  int fuel;           //!< Rocket's remaining fuel.
  bool isAlive;       //!< Whether or not the Rocket is alive.
  int floor_id_crash; //!< ID of the floor where the Rocket crashed.

  //! @brief  C'tor.
  //!
  //! @param[in] f_x, f_y    Initial coordinates.
  //! @param[in] f_vx, f_vy  Initial velocity.
  //! @param[in] f_angle     Initial angle.
  //! @param[in] f_thrust    Initial thrust power.
  //! @param[in] f_fuel      Initial fuel quantity.
  Rocket(const double f_x = 0., const double f_y = 0., const double f_vx = 0.,
         const double f_vy = 0., const std::int8_t f_angle = 0,
         const std::int8_t f_thrust = 0, const int f_fuel = 0)
      : pX{f_x}, pY{f_y}, x{f_x}, y{f_y}, vx{f_vx}, vy{f_vy},
        ax{f_thrust * sin(-f_angle * M_PI / 180.)},
        ay{f_thrust * cos(-f_angle * M_PI / 180.) + _g}, angle{f_angle},
        thrust{f_thrust}, fuel{f_fuel}, isAlive{true}, floor_id_crash{-1} {}

  //! @brief  Re-init the rocket value.
  //!
  //! @param[in] f_rocket  The rocket to re-init the values with.
  void init(const Rocket &f_rocket) {
    pX = f_rocket.x;
    pY = f_rocket.y;
    x = f_rocket.x;
    y = f_rocket.y;
    vx = f_rocket.vx;
    vy = f_rocket.vy;
    ax = f_rocket.ax;
    ay = f_rocket.ay;
    angle = f_rocket.angle;
    thrust = f_rocket.thrust;
    fuel = f_rocket.fuel;
    isAlive = true;
  }

  //! @brief  Apply the next angle and thrust requests.
  //!
  //! @param[in] f_angle  The angle request.
  //! @param[in] f_thrust The thrust power request.
  void updateRocket(const std::int8_t f_angle, const std::int8_t f_thrust) {
    // Previous position
    pX = x;
    pY = y;

    // Update angle and power
    angle = std::min(static_cast<std::int8_t>(90),
                     std::max(static_cast<std::int8_t>(-90),
                              static_cast<std::int8_t>(angle + f_angle)));
    const double angle_rad{-angle * M_PI / 180.};

    if (fuel == 0) {
      thrust = 0;
    } else {
      thrust = std::min(static_cast<std::int8_t>(4),
                        std::max(static_cast<std::int8_t>(0),
                                 static_cast<std::int8_t>(thrust + f_thrust)));
    }

    // Update fuel
    fuel = std::max(0, fuel - thrust);

    /* Update Acc, speed and position:
     *    acc.x = power * sin(-angle)
     *    acc.y = power * cos(angle) + _g
     *
     *    v.x = acc.x * t + vx0
     *    v.y = acc.y * t + vy0
     *
     *    pos.x = acc.x * t^2 / 2 + vx0 * t + x0
     *    pos.y = acc.y * t^2 / 2 + vy0 * t + y0
     *
     *    with t = 1 !
     */
    ax = thrust * sin(angle_rad);
    ay = thrust * cos(angle_rad) + _g;

    x += 0.5 * ax + vx;
    y += 0.5 * ay + vy;
    y = std::max(0., y);

    vx += ax;
    vy += ay;
  }

  //! @brief  Whether or not the current angle and speeds are in the good range
  //!         to land.
  //!
  //! @return True if landing could be successful, else False.
  bool isParamSuccess() const {
    return abs(angle) <= 15 && abs(vy) <= 40 && abs(vx) <= 20;
  }
};

// #######################################################
//
//                        GENETIC
//
// #######################################################

// -----------------------------------------------------
//                      GLOBALS
// -----------------------------------------------------

constexpr int _CHROMOSOME_SIZE{200}; //!< Amount of gene per chromosome.
constexpr int _POPULATION_SIZE{100}; //!< Amount of chromosome per population.

constexpr double _ELITISM_RATIO{0.1}; //!< Elitism rate.
constexpr double _MUTATION_RATE{0.2}; //!< Mutation rate.
constexpr int _ELITISM_IDX{static_cast<int>(
    _ELITISM_RATIO * _POPULATION_SIZE)}; //!< Idx of the last elitism selection.

// -----------------------------------------------------
//                        GENE
// -----------------------------------------------------

//! @brief  Gene struct
struct Gene {
  std::int8_t angle;  //!< Angle: clamped between -15 and 15.
  std::int8_t thrust; //!< Thrust power: clamped between -1 and 1.

  //! @brief  C'tor.
  //!
  //! @param[in] f_angle  Angle. Default is 0.
  //! @param[in] f_thrust Thrust power. Default is 0.
  Gene(const std::int8_t f_angle = 0, const std::int8_t f_thrust = 0)
      : angle{f_angle}, thrust{f_thrust} {}
};

// -----------------------------------------------------
//                       CHROMOSOME
// -----------------------------------------------------

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

//! @brief  Chromosome struct
struct Chromosome {
  Gene chromosome[_CHROMOSOME_SIZE]; //!< Genes of the chromosomes.
  double fitness;                    //!< Fitness score of the chromosome.

  //! @brief  C'tor.
  //!
  //! @param[in] f_angle  Initial angle of the genes. Default is 0.
  //! @param[in] f_thrust Initial thrust power of the genes. Default is 0.
  Chromosome(const int f_angle = 0, const int f_thrust = 0) : fitness{0} {
    int angle = f_angle;
    int thrust = f_thrust;
    for (int i = 0; i < _CHROMOSOME_SIZE; ++i) {
      chromosome[i] = {getRandAngle(f_angle), getRandThrust(f_thrust)};
      angle += chromosome[i].angle;
      thrust += chromosome[i].thrust;
    }
  }

  //! @brief  Get a pointer to the i-th gene of the chromosome.
  //!
  //! @param[in] i The index of the gene.
  //!
  //! @ return A pointer to the gene.
  Gene *getGene(const std::uint8_t i) {
    if (i < _CHROMOSOME_SIZE) {
      return &chromosome[i];
    }
    return nullptr;
  }

  //! @brief  Sort method for the STL sort function.
  static bool chromosome_sorter(Chromosome const &lhs, Chromosome const &rhs) {
    return lhs.fitness > rhs.fitness;
  }
};

// -----------------------------------------------------
//                      POPULATION
// -----------------------------------------------------

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
      std::cerr << "YOOOUUUUHHHHOOOOOUUUU" << std::endl;
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

//! @brief  Population class
class GeneticPopulation {
public:
  //! @brief  C'tor.
  //!
  //! @param[in] f_rocket       Initial rocket.
  //! @param[in] f_floor_buffer Floor buffer data.
  //! @param[in] f_size_floor   Size of the floor buffer.
  GeneticPopulation(const Rocket &f_rocket, const int *f_floor_buffer,
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

  //! @brief  Initialize all the rocket with the initial rocket.
  void initRockets() {
    for (int i = 0; i < _POPULATION_SIZE; ++i) {
      rockets_gen[i].init(rocket_save);
    }
  }

  //! @brief  Initialize all the chromosomes with some random values
  void initChromosomes() {
    for (int i = 0; i < _POPULATION_SIZE; ++i) {
      population[i] = Chromosome(rocket_save.angle, rocket_save.thrust);
    }
  }

  //! @brief  Get a pointer to the i-th chromosome of the population.
  //!
  //! @param[in] i The index of the chromosome.
  //!
  //! @ return A pointer to the chromosome.
  Chromosome *getChromosome(const std::uint8_t i) {
    if (i < _POPULATION_SIZE) {
      return &population[i];
    }
    return nullptr;
  }

  //! @brief  Get a pointer to the i-th rocket of the population.
  //!
  //! @param[in] i The index of the rocket.
  //!
  //! @ return A pointer to the rocket.
  Rocket *getRocket(const std::uint8_t i) {
    if (i < _POPULATION_SIZE) {
      return &rockets_gen[i];
    }
    return nullptr;
  }

  //! @brief  Perform the mutation on the whole population.
  //!
  //! @param[in] idxStart  Incremental index where to start the mutation.
  void mutate(const int idxStart) {
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

    // Cumulative fitness for the selection
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
          new_population[i].chromosome[g].angle =
              getRandAngle(rocket_save.angle);
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

  Rocket rocket_save;  //!< Initial rocket.
  int landing_zone_id; //!< ID of the landing_zone among the floor segments.

private:
  Chromosome populationA[_POPULATION_SIZE]; //!< A population of chromosome.
  Chromosome populationB[_POPULATION_SIZE]; //!< A population of chromosome.

  Chromosome *population;     //!< Pointer to the current population.
  Chromosome *new_population; //!< Pointer to the next population.

  Rocket rockets_gen[_POPULATION_SIZE]; //!< Rockets of the population.

  const int *floor_buffer; //!< Floor buffer data.
  const int size_floor;    //!< Size of the floor buffer data.
};

// #######################################################
//
//                          MAIN
//
// #######################################################

int main() {
  srand(time(NULL));

  int size_level; // the number of points used to draw the surface of Mars.
  std::cin >> size_level;
  std::cin.ignore();
  int level[30 * 2]; // No level has more than 22 points
  for (int i = 0; i < size_level; i++) {
    int landX; // X coordinate of a surface point. (0 to 6999)
    int landY; // Y coordinate of a surface point. By linking all the points
               // together in a sequential fashion, you form the surface of
               // Mars.
    std::cin >> landX >> landY;
    std::cin.ignore();

    level[2 * i + 0] = landX;
    level[2 * i + 1] = landY;
  }

  int X;
  int Y;
  int hSpeed; // the horizontal speed (in m/s), can be negative.
  int vSpeed; // the vertical speed (in m/s), can be negative.
  int fuel;   // the quantity of remaining fuel in liters.
  int rotate; // the rotation angle in degrees (-90 to 90).
  int power;  // the thrust power (0 to 4).
  std::cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power;
  std::cin.ignore();

  Rocket rocket(X, Y, hSpeed, vSpeed, rotate, power, fuel);

  // Rocket, level and size_level are defined in `level.hpp`
  GeneticPopulation population(rocket, level, size_level);

  bool solutionFound{false};
  int generation{0};
  int idxChromosome{0};
  int idxGene{0};
  int prevGeneration{0};

  std::chrono::high_resolution_clock::time_point start{
      std::chrono::high_resolution_clock::now()};

  std::chrono::high_resolution_clock::time_point start_loop{
      std::chrono::high_resolution_clock::now()};
  int idxStart{0};

  // -----------------------------------------------------
  //                  MAIN GENETIC LOOP
  // -----------------------------------------------------
  while (!solutionFound) {
    // ...................................................
    //                 INCREMENTAL SEARCH
    // ...................................................
    std::chrono::duration<double> elapsed_seconds{
        std::chrono::high_resolution_clock::now() - start_loop};
    if (elapsed_seconds.count() > 0.15) {
      Gene *bestGen{population.getChromosome(0)->getGene(idxStart)};

      std::cerr << "Approx done at generation " << generation << "  -  "
                << generation - prevGeneration << std::endl;
      std::cerr << "Idx: " << idxStart << std::endl;
      std::cerr << "  Angle: " << (int)bestGen->angle << std::endl;
      std::cerr << "  Power: " << (int)bestGen->thrust << std::endl;

      population.rocket_save.updateRocket(bestGen->angle, bestGen->thrust);
      std::cout << (int)population.rocket_save.angle << " "
                << (int)population.rocket_save.thrust << std::endl;

      std::cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power;
      std::cin.ignore();

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
            const Line_d floor{
                {(double)level[2 * (k - 1)], (double)level[2 * (k - 1) + 1]},
                {(double)level[2 * k], (double)level[2 * k + 1]}};
            if (rocket->x < 0 || rocket->x > _w || rocket->y < 0 ||
                rocket->y > _h) {
              rocket->isAlive = false;
              rocket->floor_id_crash = -1;
            } else if (isIntersect(prev_curr, floor)) {
              rocket->isAlive = false;
              rocket->floor_id_crash = k;

              // Landing successful!
              if (k == population.landing_zone_id && rocket->isParamSuccess()) {
                std::cerr << "Landing SUCCESS!" << std::endl << std::endl;
                solutionFound = true;
                idxChromosome = chrom;
                idxGene = gen;
              }
              break;
            }
          }
        } else {
          break;
        }
      }
    }

    // ...................................................
    //                     MUTATION
    // ...................................................
    if (!solutionFound)
      population.mutate(idxStart);
    else
      break;
  }

  std::chrono::duration<double> elapsed_seconds{
      std::chrono::high_resolution_clock::now() - start};
  std::cerr << "Execution time: " << elapsed_seconds.count() << "s"
            << std::endl;

  // -----------------------------------------------------
  //                  POPULATION FOUND
  // -----------------------------------------------------
  if (solutionFound) {
    std::cerr << std::endl;
    std::cerr << "Solution found at:" << std::endl;
    std::cerr << "  Generation: " << generation << std::endl;
    std::cerr << "  Child: " << idxChromosome << std::endl;
    std::cerr << "  Gene: " << idxGene << std::endl << std::endl;

    Chromosome *solutionPtr = population.getChromosome(idxChromosome);

    int rotateSol = rotate;
    int powerSol = power;

    // game loop
    while (1) {
      std::cerr << "Idx: " << idxStart << std::endl;
      std::cerr << "  Angle: " << (int)solutionPtr->getGene(idxStart)->angle
                << std::endl;
      std::cerr << "  Power: " << (int)solutionPtr->getGene(idxStart)->thrust
                << std::endl;

      if (idxStart < idxGene - 1) {
        rotateSol += solutionPtr->getGene(idxStart)->angle;
        powerSol += solutionPtr->getGene(idxStart)->thrust;

        rotateSol = std::min(90, std::max(-90, rotateSol));
        powerSol = std::min(4, std::max(0, powerSol));

        std::cout << rotateSol << " " << powerSol << std::endl;
      } else {
        powerSol += solutionPtr->getGene(idxStart)->thrust;
        powerSol = std::min(4, std::max(0, powerSol));

        std::cout << 0 << " " << 4 << std::endl;
      }
      idxStart++;

      int X;
      int Y;
      int hSpeed; // the horizontal speed (in m/s), can be negative.
      int vSpeed; // the vertical speed (in m/s), can be negative.
      int fuel;   // the quantity of remaining fuel in liters.
      int rotate; // the rotation angle in degrees (-90 to 90).
      int power;  // the thrust power (0 to 4).
      std::cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power;
      std::cin.ignore();
    }

  } else {
    std::cout << "No solution found..." << std::endl;
  }

  return 0;
}

#endif
