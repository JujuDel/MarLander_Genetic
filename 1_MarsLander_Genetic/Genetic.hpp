#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "Rocket.hpp"

static const int _CHROMOSOME_SIZE{200};
static const int _POPULATION_SIZE{100};

static const double _ELITISM_RATIO{0.1};
static const double _MUTATION_RATE{0.1};
static const int _ELITISM_IDX{
    static_cast<int>(_ELITISM_RATIO * _POPULATION_SIZE)};

// 2 bytes
class Gene {
public:
  Gene(const std::int8_t f_angle = 0, const std::int8_t f_power = 0);

  std::int8_t angle;
  std::int8_t power;
};

// _CHROMOSOME_SIZE * 2 + 4 bytes
class Chromosome {
public:
  Chromosome(const int f_angle = 0, const int f_power = 0);

  double fitness;

  Gene *getGene(const std::uint8_t i);

  Gene chromosome[_CHROMOSOME_SIZE];
};

bool chromosome_sorter(Chromosome const &lhs, Chromosome const &rhs);

class GeneticPopulation {
public:
  GeneticPopulation(const Rocket &f_rocket, const int *f_floor_buffer,
                    const int f_size_floor);

  void initRockets();
  void initChromosomes();

  Chromosome *getChromosome(const std::uint8_t i);
  Rocket *getRocket(const std::uint8_t i);

  void mutate(const int idxStart);

  Rocket rocket_save;

  int landing_zone_id; // ID of the landing_zone among the floor segments

private:
  Chromosome populationA[_POPULATION_SIZE]; // _POPULATION_SIZE *
                                            // (_CHROMOSOME_SIZE * 2 + 4 bytes)
  Chromosome populationB[_POPULATION_SIZE]; // _POPULATION_SIZE *
                                            // (_CHROMOSOME_SIZE * 2 + 4 bytes)

  Chromosome *population;
  Chromosome *new_population;

  Rocket rockets_gen[_POPULATION_SIZE]; // _POPULATION_SIZE * 9 bytes

  const int *floor_buffer;
  const int size_floor;
};

#endif
