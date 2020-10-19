
//#define CODING_GAME
#ifdef CODING_GAME

// Include standard headers
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cmath>

/* CONSTS */

const float _w{ 6999.f };
const float _h{ 2999.f };

static const int _CHROMOSOME_SIZE{ 200 };
static const int _POPULATION_SIZE{ 100 };

static const double _ELITISM_RATIO{ 0.1 };
static const double _MUTATION_RATE{ 0.1 };
static const int _ELITISM_IDX{ static_cast<int>(_ELITISM_RATIO * _POPULATION_SIZE) };

static const double _PI{ 3.14159265 };
static const double _g{ -3.711 }; // m/s-2

/* HELPING STRUCTS AND METHODS */

struct Coord_d {
    double x;
    double y;
};

struct Line_d {
    Coord_d p1;
    Coord_d p2;
};

void applyRotation(Coord_d& P, const double c, const double s) {
    const double x{ P.x };
    const double y{ P.y };
    P.x = x * c - y * s;
    P.y = x * s + y * c;
}

bool onLine(Line_d l1, Coord_d p)
{
    return (p.x <= std::max(l1.p1.x, l1.p2.x) && p.x >= std::min(l1.p1.x, l1.p2.x) &&
        (p.y <= std::max(l1.p1.y, l1.p2.y) && p.y >= std::min(l1.p1.y, l1.p2.y)));
}

int direction(Coord_d a, Coord_d b, Coord_d c) {
    double val{ (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y) };
    if (val == 0) {
        return 0;       // colinear
    }
    else if (val < 0) {
        return 2;       // anti-clockwise direction
    }
    else {
        return 1;       // clockwise direction
    }
}

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

/* ROCKET */

class Rocket
{
public:
    Rocket(const double f_x = 0., const double f_y = 0., const double f_vx = 0., const double f_vy = 0., const std::int8_t f_angle = 0, const std::int8_t f_power = 0, const int f_fuel = 0) :
        pX{ f_x },
        pY{ f_y },
        x{ f_x },
        y{ f_y },
        vx{ f_vx },
        vy{ f_vy },
        ax{ f_power * sin(-f_angle * _PI / 180.) },
        ay{ f_power * cos(-f_angle * _PI / 180.) + _g },
        angle{ f_angle },
        power{ f_power },
        fuel{ f_fuel },
        isAlive{ true },
        floor_id_crash{ -1 }
    {}

    void init(const Rocket& f_rocket) {
        pX = f_rocket.x;
        pY = f_rocket.y;
        x = f_rocket.x;
        y = f_rocket.y;
        vx = f_rocket.vx;
        vy = f_rocket.vy;
        ax = f_rocket.ax;
        ay = f_rocket.ay;
        angle = f_rocket.angle;
        power = f_rocket.power;
        fuel = f_rocket.fuel;
        isAlive = true;
    }

    void updateRocket(const std::int8_t f_angle, const std::int8_t f_power) {
        // Previous position
        pX = x;
        pY = y;

        // Update angle and power
        angle = std::min(static_cast<std::int8_t>(90), std::max(static_cast<std::int8_t>(-90), static_cast<std::int8_t>(angle + f_angle)));
        const double angle_rad{ -angle * _PI / 180. };

        if (fuel == 0) {
            power = 0;
        }
        else {
            power = std::min(static_cast<std::int8_t>(4), std::max(static_cast<std::int8_t>(0), static_cast<std::int8_t>(power + f_power)));
        }

        // Update fuel
        fuel = std::max(0, fuel - power);

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
        ax = power * sin(angle_rad);
        ay = power * cos(angle_rad) + _g;

        x += 0.5 * ax + vx;
        y += 0.5 * ay + vy;
        y = std::max(0., y);

        vx += ax;
        vy += ay;
    }

    bool isParamSuccess() const {
        return abs(angle) <= 15 && abs((int)vy) <= 40 && abs((int)vx) <= 20;
    }

    /* Params */
    double pX;
    double pY;
    double x;
    double y;
    double vx;
    double vy;
    double ax;
    double ay;
    std::int8_t angle;
    std::int8_t power;
    int fuel;
    bool isAlive;
    int floor_id_crash;
};

/* GENE */

// 2 bytes
class Gene
{
public:
    Gene(const std::int8_t f_angle = 0, const std::int8_t f_power = 0) :
        angle{ f_angle },
        power{ f_power }
    {}

    std::int8_t angle;
    std::int8_t power;
};

/* CHROMOSOME */

// _CHROMOSOME_SIZE * 2 + 4 bytes
class Chromosome
{
public:
    Chromosome() : fitness{ 0 }
    {
        for (int i = 0; i < _CHROMOSOME_SIZE; ++i) {
            chromosome[i] = { std::int8_t(rand() % 31 - 15), std::int8_t(rand() % 3 - 1) };
        }
    }

    Gene* getGene(const std::uint8_t i) {
        if (i < _CHROMOSOME_SIZE) {
            return &chromosome[i];
        }
        return nullptr;
    }

    double fitness;
    Gene chromosome[_CHROMOSOME_SIZE];
};

bool chromosome_sorter(Chromosome const& lhs, Chromosome const& rhs) {
    return lhs.fitness > rhs.fitness;
}

/* FITNESS */

/* f(d) = 1000. / (1 + 0.009999 * d)
*
* f(0)      = 1000.000
* f(10)     =  909.099
* f(100)    =  500.025
* f(1000)   =   90.917
* f(10000)  =    9.902
* f(100000) =    0.999
*/
double distance(const Rocket& rocket, const int* floor_buffer, const int landing_zone_id) {
    if (rocket.floor_id_crash == -1)
        return 0.;

    double dist{ 0. };
    // Crash distance
    if (rocket.floor_id_crash == landing_zone_id) {
        if (rocket.isParamSuccess()) {
            std::cerr << "YOOOUUUUHHHHOOOOOUUUU" << std::endl;
            return 99999.;
        }
    }
    else if (rocket.floor_id_crash < landing_zone_id) {
        dist = sqrt(
            pow(rocket.x - floor_buffer[2 * rocket.floor_id_crash + 0], 2) +
            pow(rocket.y - floor_buffer[2 * rocket.floor_id_crash + 1], 2));
        for (int k = rocket.floor_id_crash + 1; k < landing_zone_id; ++k) {
            dist += sqrt(
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 0]) + floor_buffer[2 * k + 0], 2) +
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 1]) + floor_buffer[2 * k + 1], 2));
        }
    }
    else {
        dist = sqrt(
            pow(rocket.x - floor_buffer[2 * (rocket.floor_id_crash - 1) + 0], 2) +
            pow(rocket.y - floor_buffer[2 * (rocket.floor_id_crash - 1) + 1], 2));

        for (int k = rocket.floor_id_crash - 1; k > landing_zone_id; --k) {
            dist += sqrt(
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 0]) + floor_buffer[2 * k + 0], 2) +
                pow(static_cast<double>(floor_buffer[2 * (k - 1) + 1]) + floor_buffer[2 * k + 1], 2));
        }
    }
    return 1000. / (1. + 0.009999 * dist);
}

/* Horizontal speed: s >= 0 in m/s-1
*   f(s) = 0.00036057692307692 * s^2 + 0.069711538461538 * s - 3.3653846153846
*
* f(0)   = 3.3653846153846
* f(40)  = 0
* f(200) = -25
* f(300) = -50
*
*  Vertical speed: s >= 0 in m/s-1
*   f(s) = 0.0003968253968254 * s^2 + 0.051587301587302 * s - 1.1904761904762
*
* fy(0)   = 1.1904761904762
* fy(25)  = 0
* f(200) = -25
* f(300) = -50
*/
double speed(const double vx, const double vy) {
    double scoreX = 0.00036057692307692 * vx * vx + 0.069711538461538 * vx - 3.3653846153846;
    double scoreY = 0.0003968253968254 * vy * vy + 0.051587301587302 * vy - 1.1904761904762;

    return -5. * (scoreX + scoreY);
}

/* POPULATION */

class GeneticPopulation
{
public:
    GeneticPopulation(const Rocket& f_rocket, const int* f_floor_buffer, const int f_size_floor) :
        rocket_save{ f_rocket },
        floor_buffer{ f_floor_buffer },
        size_floor{ f_size_floor },
        population{ &populationA[0] },
        new_population{ &populationB[0] }
    {
        for (int i = 1; i < size_floor; ++i) {
            // Statements constraints: Only 1 landing zone
            if (floor_buffer[2 * (i - 1) + 1] == floor_buffer[2 * i + 1]) {
                landing_zone_id = i;
                break;
            }
        }

        initRockets();
    }

    void initRockets() {
        for (int i = 0; i < _POPULATION_SIZE; ++i) {
            rockets_gen[i].init(rocket_save);
        }
    }

    Chromosome* getChromosome(const std::uint8_t i) {
        if (i < _POPULATION_SIZE) {
            return &population[i];
        }
        return nullptr;
    }

    Rocket* getRocket(const std::uint8_t i) {
        if (i < _POPULATION_SIZE) {
            return &rockets_gen[i];
        }
        return nullptr;
    }

    void mutate() {
        double sum_fitness{ 0. };
        // Compute every fitness
        for (int i = 0; i < _POPULATION_SIZE; ++i) {
            if (rockets_gen[i].isAlive) {
                population[i].fitness = 0;
            }
            else {
                population[i].fitness = distance(rockets_gen[i], floor_buffer, landing_zone_id);
            }

            if (rockets_gen[i].floor_id_crash == landing_zone_id) {
                population[i].fitness += speed(abs((int)rockets_gen[i].vx), abs((int)rockets_gen[i].vy));
            }

            sum_fitness += population[i].fitness;
        }

        // Sort the fitness
        std::sort(population, population + sizeof(populationA) / sizeof(populationA[0]), &chromosome_sorter);

        // Cumulative fitness for the selection
        double cum_sum{ 0. };
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
            const double choice1{ rand() / static_cast<double>(RAND_MAX) };
            int idxParent1{ 1 };
            for (; idxParent1 < _POPULATION_SIZE; ++idxParent1) {
                if (population[idxParent1].fitness < choice1)
                {
                    idxParent1--;
                    break;
                }
            }

            int idxParent2{ idxParent1 };
            while (idxParent2 == idxParent1) {
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

            for (int g = 0; g < _CHROMOSOME_SIZE; ++g) {
                const double r{ rand() / static_cast<double>(RAND_MAX) };
                if (r > _MUTATION_RATE) {
                    const double angleP0 = population[idxParent1].chromosome[g].angle;
                    const double angleP1 = population[idxParent2].chromosome[g].angle;
                    const double powerP0 = population[idxParent1].chromosome[g].power;
                    const double powerP1 = population[idxParent2].chromosome[g].power;

                    new_population[i].chromosome[g].angle = static_cast<std::int8_t>(r * angleP0 + (1 - r) * angleP1);
                    new_population[i].chromosome[g].power = static_cast<std::int8_t>(r * powerP0 + (1 - r) * powerP1);
                    if (i != _POPULATION_SIZE - 1) {
                        new_population[i + 1].chromosome[g].angle = static_cast<std::int8_t>((1 - r) * angleP0 + r * angleP1);
                        new_population[i + 1].chromosome[g].power = static_cast<std::int8_t>((1 - r) * powerP0 + r * powerP1);
                    }
                }
                else {
                    new_population[i].chromosome[g].angle = rand() % 31 - 15;
                    new_population[i].chromosome[g].power = rand() % 3 - 1;
                    if (i != _POPULATION_SIZE - 1) {
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

    const Rocket rocket_save;

    int landing_zone_id; // ID of the landing_zone among the floor segments

private:
    Chromosome populationA[_POPULATION_SIZE]; // _POPULATION_SIZE * (_CHROMOSOME_SIZE * 2 + 4 bytes)
    Chromosome populationB[_POPULATION_SIZE]; // _POPULATION_SIZE * (_CHROMOSOME_SIZE * 2 + 4 bytes)

    Chromosome* population;
    Chromosome* new_population;

    Rocket rockets_gen[_POPULATION_SIZE]; // _POPULATION_SIZE * 9 bytes

    const int* floor_buffer;
    const int size_floor;
};

/* MAIN */

int main()
{
    srand(time(NULL));

    int size_level; // the number of points used to draw the surface of Mars.
    std::cin >> size_level; std::cin.ignore();
    int level[30 * 2]; // No level has more than 22 points
    for (int i = 0; i < size_level; i++) {
        int landX; // X coordinate of a surface point. (0 to 6999)
        int landY; // Y coordinate of a surface point. By linking all the points together in a sequential fashion, you form the surface of Mars.
        std::cin >> landX >> landY; std::cin.ignore();

        level[2 * i + 0] = landX;
        level[2 * i + 1] = landY;
    }

    int X;
    int Y;
    int hSpeed; // the horizontal speed (in m/s), can be negative.
    int vSpeed; // the vertical speed (in m/s), can be negative.
    int fuel; // the quantity of remaining fuel in liters.
    int rotate; // the rotation angle in degrees (-90 to 90).
    int power; // the thrust power (0 to 4).
    std::cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power; std::cin.ignore();

    Rocket rocket(X, Y, hSpeed, vSpeed, rotate, power, fuel);

    // Rocket, level and size_level are defined in `level.hpp`
    GeneticPopulation population(rocket, level, size_level);

    bool solutionFound{ false };
    int generation{ 0 };
    int idxChromosome{ 0 };
    int idxGene{ 0 };

    std::chrono::steady_clock::time_point start{ std::chrono::steady_clock::now() };

    while (!solutionFound) {
        generation++;
        population.initRockets();

        // For every possible moves, i.e., for every genes
        for (int gen = 0; !solutionFound && gen < _CHROMOSOME_SIZE; ++gen) {
            // For every Rocket and their associated chromosome
            for (int chrom = 0; chrom < _POPULATION_SIZE; ++chrom) {
                Rocket* rocket{ population.getRocket(chrom) };
                if (rocket->isAlive) {
                    Gene* gene{ population.getChromosome(chrom)->getGene(gen) };

                    rocket->updateRocket(gene->angle, gene->power);

                    const Line_d prev_curr{ {rocket->pX, rocket->pY}, {rocket->x, rocket->y} };
                    for (int k = 1; k < size_level; ++k) {
                        const Line_d floor{ {(double)level[2 * (k - 1)], (double)level[2 * (k - 1) + 1]}, {(double)level[2 * k], (double)level[2 * k + 1]} };
                        if (rocket->x < 0 || rocket->x > _w || rocket->y < 0 || rocket->y > _h) {
                            rocket->isAlive = false;
                            rocket->floor_id_crash = -1;
                        }
                        else if (isIntersect(prev_curr, floor)) {
                            rocket->isAlive = false;
                            rocket->floor_id_crash = k;

                            // Landing successful!
                            if (k == population.landing_zone_id && rocket->isParamSuccess())
                            {
                                std::cerr << "Landing SUCCESS!" << std::endl << std::endl;
                                solutionFound = true;
                                idxChromosome = chrom;
                                idxGene = gen;
                            }
                            break;
                        }
                    }
                }
            }
        }
        if (!solutionFound)
            population.mutate();
        else
            break;
    }

    std::chrono::duration<double> elapsed_seconds{ std::chrono::steady_clock::now() - start };
    std::cerr << "Execution time: " << elapsed_seconds.count() << "s" << std::endl;

    if (solutionFound) {
        std::cerr << std::endl;
        std::cerr << "Solution found at:" << std::endl;
        std::cerr << "  Generation: " << generation << std::endl;
        std::cerr << "  Child: " << idxChromosome << std::endl;
        std::cerr << "  Gene: " << idxGene << std::endl << std::endl;

        Chromosome* solutionPtr = population.getChromosome(idxChromosome);

        int rotateSol = rotate;
        int powerSol = power;
        int idx = 0;
        // game loop
        while (1) {
            std::cerr << "Idx: " << idx << std::endl;
            std::cerr << "  Angle: " << (int)solutionPtr->getGene(idx)->angle << std::endl;
            std::cerr << "  Power: " << (int)solutionPtr->getGene(idx)->power << std::endl;

            if (idx < idxGene - 1) {
                rotateSol += solutionPtr->getGene(idx)->angle;
                powerSol += solutionPtr->getGene(idx)->power;

                rotateSol = std::min(90, std::max(-90, rotateSol));
                powerSol = std::min(4, std::max(0, powerSol));

                std::cout << rotateSol << " " << powerSol << std::endl;
            }
            else {
                powerSol += solutionPtr->getGene(idx)->power;
                powerSol = std::min(4, std::max(0, powerSol));

                std::cout << 0 << " " << powerSol << std::endl;
            }
            idx++;

            int X;
            int Y;
            int hSpeed; // the horizontal speed (in m/s), can be negative.
            int vSpeed; // the vertical speed (in m/s), can be negative.
            int fuel; // the quantity of remaining fuel in liters.
            int rotate; // the rotation angle in degrees (-90 to 90).
            int power; // the thrust power (0 to 4).
            std::cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power; std::cin.ignore();
        }

    }
    else {
        std::cout << "No solution found..." << std::endl;
    }

    return 0;
}

#endif
