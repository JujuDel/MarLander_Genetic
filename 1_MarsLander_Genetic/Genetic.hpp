#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "Rocket.hpp"

// #######################################################
//
//                       GLOBALS
//
// #######################################################

constexpr int _CHROMOSOME_SIZE{200}; //!< Amount of gene per chromosome.
constexpr int _POPULATION_SIZE{100}; //!< Amount of chromosome per population.

constexpr double _ELITISM_RATIO{0.1}; //!< Elitism rate.
constexpr double _MUTATION_RATE{0.2}; //!< Mutation rate.
constexpr int _ELITISM_IDX{static_cast<int>(
    _ELITISM_RATIO * _POPULATION_SIZE)}; //!< Idx of the last elitism selection.

// #######################################################
//
//                        GENE
//
// #######################################################

//! @brief  Gene struct
struct Gene {
  std::int8_t angle;  //!< Angle: clamped between -15 and 15.
  std::int8_t thrust; //!< Thrust power: clamped between -1 and 1.

  //! @brief  C'tor.
  //!
  //! @param[in] f_angle  Angle. Default is 0.
  //! @param[in] f_thrust Thrust power. Default is 0.
  Gene(const std::int8_t f_angle = 0, const std::int8_t f_thrust = 0);
};

// #######################################################
//
//                      CHROMOSOME
//
// #######################################################

//! @brief  Chromosome struct
struct Chromosome {
  Gene chromosome[_CHROMOSOME_SIZE]; //!< Genes of the chromosomes.
  double fitness;                    //!< Fitness score of the chromosome.

  //! @brief  C'tor.
  //!
  //! @param[in] f_angle  Initial angle of the genes. Default is 0.
  //! @param[in] f_thrust Initial thrust power of the genes. Default is 0.
  Chromosome(const int f_angle = 0, const int f_thrust = 0);

  //! @brief  Get a pointer to the i-th gene of the chromosome.
  //!
  //! @param[in] i The index of the gene.
  //!
  //! @ return A pointer to the gene.
  Gene *getGene(const std::uint8_t i);

  //! @brief  Sort method for the STL sort function.
  static bool chromosome_sorter(Chromosome const &lhs, Chromosome const &rhs);
};

// #######################################################
//
//                      POPULATION
//
// #######################################################

//! @brief  Population class
class GeneticPopulation {
public:
  //! @brief  C'tor.
  //!
  //! @param[in] f_rocket       Initial rocket.
  //! @param[in] f_floor_buffer Floor buffer data.
  //! @param[in] f_size_floor   Size of the floor buffer.
  GeneticPopulation(const Rocket &f_rocket, const int *f_floor_buffer,
                    const int f_size_floor);

  //! @brief  Initialize all the rocket with the initial rocket.
  void initRockets();

  //! @brief  Initialize all the chromosomes with some random values
  void initChromosomes();

  //! @brief  Get a pointer to the i-th chromosome of the population.
  //!
  //! @param[in] i The index of the chromosome.
  //!
  //! @ return A pointer to the chromosome.
  Chromosome *getChromosome(const std::uint8_t i);

  //! @brief  Get a pointer to the i-th rocket of the population.
  //!
  //! @param[in] i The index of the rocket.
  //!
  //! @ return A pointer to the rocket.
  Rocket *getRocket(const std::uint8_t i);

  //! @brief  Perform the mutation on the whole population.
  //!
  //! @param[in] idxStart  Incremental index where to start the mutation.
  void mutate(const int idxStart);

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

#endif
