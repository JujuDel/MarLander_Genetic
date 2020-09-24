
#include <algorithm>
#include <iostream>

#include <Genetic.hpp>

  /*******************************/
 /*            Gene             */
/*******************************/
Gene::Gene(const std::int8_t f_angle, const std::int8_t f_power) :
    angle{ f_angle },
    power{ f_power }
{}

  /*******************************/
 /*         Chromosome          */
/*******************************/
Chromosome::Chromosome() :
    fitness{ 0 }
{
    for (int i = 0; i < _CHROMOSOME_SIZE; ++i)
    {
        chromosome[i] = { rand() % 31 - 15, rand() % 3 - 1 };
    }
}

Gene* Chromosome::getGene(const std::uint8_t i)
{
    if (i < _CHROMOSOME_SIZE)
    {
        return &chromosome[i];
    }
    return nullptr;
}

bool chromosome_sorter(Chromosome const& lhs, Chromosome const& rhs)
{
    return lhs.fitness > rhs.fitness;
}

  /*******************************/
 /*         Population          */
/*******************************/
GeneticPopulation::GeneticPopulation(const Rocket& f_rocket, const int* f_floor_buffer, const int f_size_floor) :
    rocket_save{ f_rocket },
    floor_buffer{ f_floor_buffer},
    size_floor{ f_size_floor },
    population{ &populationA[0] },
    new_population{ &populationB[0] }
{
    for (int i = 1; i < size_floor; ++i)
    {
        // Statements constraints: Only 1 landing zone
        if (floor_buffer[2 * (i - 1) + 1] == floor_buffer[2 * i + 1])
        {
            std::cout << "LANDING=" << i << std::endl;
            landing_zone_id = i;
            break;
        }
    }

    initRockets();
}

void GeneticPopulation::initRockets()
{
    for (int i = 0; i < _POPULATION_SIZE; ++i)
    {
        rockets_gen[i].init(rocket_save);
    }
}

Chromosome* GeneticPopulation::getChromosome(const std::uint8_t i)
{
    if (i < _POPULATION_SIZE)
    {
        return &population[i];
    }
    return nullptr;
}

Rocket* GeneticPopulation::getRocket(const std::uint8_t i)
{
    if (i < _POPULATION_SIZE)
    {
        return &rockets_gen[i];
    }
    return nullptr;
}

double distance(const Rocket& rocket, const int* floor_buffer, const int landing_zone_id)
{
    if (rocket.floor_id_crash == -1)
        return 0.;

    double dist{ 0. };
    // Crash distance
    if (rocket.floor_id_crash == landing_zone_id)
    {
        if (rocket.isParamSuccess())
        {
            std::cout << "YOOOUUUUHHHHOOOOOUUUU" << std::endl;
            return 99999.;
        }
    }
    else if (rocket.floor_id_crash < landing_zone_id)
    {
        dist = sqrt(
            pow(rocket.x - floor_buffer[2 * rocket.floor_id_crash + 0], 2) +
            pow(rocket.y - floor_buffer[2 * rocket.floor_id_crash + 1], 2));
        for (int k = rocket.floor_id_crash + 1; k < landing_zone_id; ++k)
        {
            dist += sqrt(
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 0]) + floor_buffer[2 * k + 0], 2) +
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 1]) + floor_buffer[2 * k + 1], 2));
        }
    }
    else
    {
        dist = sqrt(
            pow(rocket.x - floor_buffer[2 * (rocket.floor_id_crash - 1) + 0], 2) +
            pow(rocket.y - floor_buffer[2 * (rocket.floor_id_crash - 1) + 1], 2));

        for (int k = rocket.floor_id_crash - 1; k > landing_zone_id; --k)
        {
            dist += sqrt(
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 0]) + floor_buffer[2 * k + 0], 2) +
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 1]) + floor_buffer[2 * k + 1], 2));
        }
    }
    return 1. / (0.02 + 0.001 * dist); // dist = 0 -> return 25
}

void GeneticPopulation::mutate()
{
    double sum_fitness{ 0. };
    int idxBest{ 0 };
    int fitnessBest{ 0 };

    // Compute every fitness
    for (int i = 0; i < _POPULATION_SIZE; ++i)
    {
        if (rockets_gen[i].isAlive)
        {
            population[i].fitness = 0;
        }
        else
        {
            population[i].fitness = distance(rockets_gen[i], floor_buffer, landing_zone_id);
        }

        if (rockets_gen[i].floor_id_crash == landing_zone_id)
        {
            population[i].fitness -= abs(rockets_gen[i].vx) / 8.;
            population[i].fitness -= abs(rockets_gen[i].vy) / 8.;
        }
        sum_fitness += population[i].fitness;

        if (population[i].fitness > fitnessBest)
        {
            fitnessBest = population[i].fitness;
            idxBest = i;
        }
        //std::cout << "  Fitness=" << population[i].fitness << std::endl;
    }

    //std::cout << std::endl << "Best is " << idxBest << " -> " << population[idxBest].fitness << std::endl;
    //rockets_gen[idxBest].debug();

    // Sort the fitness
    std::sort(population, population + sizeof(populationA) / sizeof(populationA[0]), &chromosome_sorter);

    double cum_sum{ 0. };
    for (int i = _POPULATION_SIZE - 1; i >= 0; --i)
    {
        //std::cout << "  Sorted Fitness=" << population[i].fitness / sum_fitness << std::endl;
        population[i].fitness = cum_sum + population[i].fitness / sum_fitness;
        /*
        if (i < _ELITISM_IDX)
        {
            std::cout << "  Sorted Fitness=" << population[i].fitness << " ELITE" << std::endl;
        }
        else
        {
            std::cout << "  Sorted Fitness=" << population[i].fitness << std::endl;
        }
        */
        cum_sum = population[i].fitness;
    }

    // Elitism
    for (int i = 0; i < _ELITISM_IDX; ++i)
    {
        new_population[i] = population[i];
    }

    // Continuous Genetic Algorithm
    for (int i = _ELITISM_IDX; i < _POPULATION_SIZE; i+=2)
    {
        const double choice1{ rand() / static_cast<double>(RAND_MAX) };
        int idxParent1{ 1 };
        for (; idxParent1 < _POPULATION_SIZE; ++idxParent1)
        {
            if (population[idxParent1].fitness < choice1)
            {
                idxParent1--;
                break;
            }
        }

        int idxParent2{ idxParent1 };
        while (idxParent2 == idxParent1)
        {
            idxParent2 = 1;
            const double choice2{ rand() / static_cast<double>(RAND_MAX) };
            for (; idxParent2 < _POPULATION_SIZE; ++idxParent2)
            {
                if (population[idxParent2].fitness < choice2)
                {
                    idxParent2--;
                    break;
                }
            }
        }
        
        for (int g = 0; g < _CHROMOSOME_SIZE; ++g)
        {
            const double r{ rand() / static_cast<double>(RAND_MAX) };
            if (r > _MUTATION_RATE)
            {
                const double angleP0 = population[idxParent1].chromosome[g].angle;
                const double angleP1 = population[idxParent2].chromosome[g].angle;
                const double powerP0 = population[idxParent1].chromosome[g].power;
                const double powerP1 = population[idxParent2].chromosome[g].power;

                new_population[i].chromosome[g].angle = static_cast<std::int8_t>(r * angleP0 + (1 - r) * angleP1);
                new_population[i].chromosome[g].power = static_cast<std::int8_t>(r * powerP0 + (1 - r) * powerP1);
                if (i != _POPULATION_SIZE - 1)
                {
                    new_population[i + 1].chromosome[g].angle = static_cast<std::int8_t>((1 - r) * angleP0 + r * angleP1);
                    new_population[i + 1].chromosome[g].power = static_cast<std::int8_t>((1 - r) * powerP0 + r * powerP1);
                }
            }
            else
            {
                new_population[i].chromosome[g].angle = rand() % 31 - 15;
                new_population[i].chromosome[g].power = rand() % 3 - 1;
                if (i != _POPULATION_SIZE - 1)
                {
                    new_population[i + 1].chromosome[g].angle = rand() % 31 - 15;
                    new_population[i + 1].chromosome[g].power = rand() % 3 - 1;
                }
            }
        }
    }

    Chromosome* tmp = population;
    population = new_population;
    new_population = tmp;

}
