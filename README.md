

# MarLander - Genetic Algorithm

Resolve the series of CodinGame's problem called Mars Lander using a Genetic Algorithm.

![gifcmd](data/gif/cmd.gif)

Links of the problems:
 - [Mars Lander - Episode 1](https://www.codingame.com/ide/puzzle/mars-lander-episode-1) - Easy - 100%
 - [Mars Lander - Episode 2](https://www.codingame.com/ide/puzzle/mars-lander-episode-2) - Medium - 100%
 - [Mars Lander - Episode 3](https://www.codingame.com/ide/puzzle/mars-lander-episode-3) - Very hard - 100%
 - [Mars Lander - Optimization](https://www.codingame.com/ide/puzzle/mars-lander) - 310th
 
Some resolutions | The last level
:---: | :---:
![giflvls](data/gif/levels.gif) | ![gif7](data/gif/level7.gif)

Solution developed in ***C++*** and using ***modern OpenGL*** for the visualization.

Level1 | Level2 | Level3
:---: | :---: | :---:
<img height="256" alt="in" src="data/images/achievements_level1.png"> | <img height="256" alt="out" src="data/images/achievements_level2.png"> | <img height="256" alt="out" src="data/images/achievements_level3.png">

## How-to compile and use

This project uses CMake and this how-to shows how-to use it with Visual Studio.

1) Launch your CMake-gui application
2) *"Where is the source code:"* is the current repo
3) *"Where to build the binaries:"* is usually a build folder in the current repo
4) Press *"Configure"* and then *"Generate"*
5) Close your CMake-gui application and launch the *"MarsLander_Genetic.sln"* in the build folder

Then, under Visual Studio:

1) Build -> Build Solution (once done you can play around with OpenGL with the project cube3D, my *Hello World* to check that everything is fine)
2) On the *"Solution Explorer"* tab, right click on the *"MarsLander_Genetic"* project then *"Set as Startup Project"*

CMake-gui | Visual Studio
:---: | :---:
![cmake](data/images/cmake.png) | ![visualStudio](data/images/visualStudio.png)

## Statements of the problem

On a *limited zone* of Mars, there is ***a unique*** area of flat ground on the surface.

***Every second***, depending on the current flight parameters (location, speed, fuel ...), the program must provide the new desired tilt angle and thrust power of the ship.

![physics](data/images/physics.png)

The game simulates a free fall without atmosphere. Gravity on Mars is 3.711 m/s².
For a thrust power of X, a push force equivalent to X m/s² is generated and X liters of fuel are consumed. As such, a thrust power of 4 in an almost vertical position is needed to compensate for the gravity on Mars.

![controls](data/images/controls.png)

For a landing to be successful, the ship must:
 - land on flat ground
 - land in a vertical position (tilt angle = 0°)
 - vertical speed must be limited ( ≤ 40m/s in absolute value)
 - horizontal speed must be limited ( ≤ 20m/s in absolute value)

## Environment of Simulation

Using modern OpenGL for the visualization, the first step was to simulate the Mars environment and the evolution of the ship's position, velocity and acceleration.

By just letting the ship falling with and without initial horizontal velocity and comparing with CodinGame, it ensures proper application of basic physics:

Without initial horizontal velocity | With initial horizontal velocity
:---: | :---:
![level1_falling](data/gif/level1_falling.gif) | ![level2_falling](data/gif/level2_falling.gif)

Then manually adding rotation and thrust power requests, and comparing with CodinGame, it ensures proper application of the overall problem's physics:

With rotation and thrust power requests |
:---: |
![level1_landing](data/gif/level1_landing.gif) |

We are now ready to go with the Genetic Algorithm!

## Genetic Algorithm

### Architecture

The architecture is as follow:

```cpp
/* Rocket struct
*
* A rocket object used to simulate the movements in the atmosphere of Mars.
*/
struct Rocket {
  double pX, pY;      //!< Previous coordinates.
  double x, y;        //!< Current coordinates.
  double vx, vy;      //!< Rocket's velocity.
  double ax, ay;      //!< Rocket's acceleration.
  std::int8_t angle;  //!< Rocket's angle.
  std::int8_t thrust; //!< Rocket's thrust power.
  int fuel;           //!< Rocket's remaining fuel.
  bool isAlive;       //!< Whether or not the Rocket is alive.

  //! @brief  C'tor.
  //!
  //! @param[in] f_x, f_y    Initial coordinates.
  //! @param[in] f_vx, f_vy  Initial velocity.
  //! @param[in] f_angle     Initial angle.
  //! @param[in] f_thrust    Initial thrust power.
  //! @param[in] f_fuel      Initial fuel quantity.
  Rocket(const double f_x = 0., const double f_y = 0., const double f_vx = 0.,
         const double f_vy = 0., const std::int8_t f_angle = 0,
         const std::int8_t f_thrust = 0, const int f_fuel = 0);

  //! @brief  Apply the next angle and thrust requests.
  //!
  //! @param[in] f_angle  The angle request.
  //! @param[in] f_thrust The thrust power request.
  void updateRocket(const std::int8_t f_angle, const std::int8_t f_thrust);

  //! @brief  Whether or not the current angle and speeds are in the good range
  //!         to land.
  //!
  //! @return True if landing could be successful, else False.
  bool isParamSuccess() const;
};

/* Gene struct
*
* Contains an angle/thrust request.
*/
struct Gene {
  std::int8_t angle;
  std::int8_t thrust;
};

/* Chromosome struct
*
* Contains a set of angle/thrust requests and the fitness score obtained by following them.
*/
struct Chromosome {
  Gene chromosome[_CHROMOSOME_SIZE];
  double fitness;
};

/* Population class
*
* Contains two arrays of population to optimize the run-time performances.
* The two pointers 'population' and 'new_population' will alternatively point to 'populationA' and 'populationB'.
* In that way, we reduce the amount of memory allocation/de-allocation and just stick with memory access.
*/
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
```

### Fitness

#### Step 1: The closer from the landing zone, the better!

A good metric to have a higher score when you are closer is the inverse of the distance: `f(x) = 1 / x`.

However, accordingly to your use cases distribution, it might work or not. Let's then generalize it a bit try to find the best score function with our data:

 - The score will be something like `f(x) = a / (b + c * x)`,
 - Let's find `a`, `b` and `c`.
 
By running a good number of random parameters on all the cases and measuring the distance between the crash zones and the landing zones, we can see that the distances vary from `0 meter` to almost `100 000 meters`. Watching at the diversity, let's define `n points` with the score that *would represent how good they are*. Thanks to those `n points`, one can easily compute the `a`, `b` and `c` that fits them the best.

In my case, I found that `f(x) = 1000 / (1 + 0.009999 * x)` works well, with its scores in the range `]0, 1000]`.

#### Step 2: Once on the landing zone, penalize fast landers!

Here, we can take into account the speed of the landers within the score calculus in two cases:

 1. Always.
 2. Only for the landers which crashed into the landing zone.

And to take into account the speed, two strategies are possible:

 1. The lower the speed is, the more we increase the score.
 2. The higher the speed is, the more we penalize the score.

I decided to **penalized the fast landers which crashed into the landing zone**. In that way, landers who crashed close to the landing zone could still have a good chance of being selected as parents.

Here, I thought about using two quadratics function (one for the vertical speed and one the horizontal speed): we are looking for `fx(vx) = ax * vx^2 + bx * vx + cx` and `fy(vy) = ay * vy^2 + by * vy + cy`.

As before, by watching at the data on random examples, by selecting a few points and by computing the best parameters:

 - Horizontal speed penality: `fx(vx) = 0.00036057692307692 * vx^2 + 0.069711538461538 * vx - 3.3653846153846`
 - Vertical speed penality: `fy(vy) = 0.0003968253968254 * vy^2 + 0.051587301587302 * vy - 1.1904761904762`

### Selection

The selection is done using ***Fitness proportionate selection*** (also known as *roulette wheel*):

![fitnessSelection](data/images/fitness_proportionate_selection.png)

### Crossover

The crossover is a weighted average sum:

![crossover](data/images/crossover.png)

<!-- Given two parents $P_1=[Gp^1_1, Gp^1_2, ..., Gp^1_n]$ and $P_2=[Gp^2_1, Gp^2_2, ..., Gp^2_n]$, the creation of two children $C_1$ and $C_2$  is done as follow:

For every pair of genes $(Gp^1_i, Gp^2_i)$, take a random number $r \in [0, 1]$ and then:
 - $Gc^1_i = r * Gp^1_i + (1-r) * Gp^2_i$
 - $Gc^2_i = (1 - r) * Gp^1_i + r * Gp^2_i$

At the end, you obtain $C_1=[Gc^1_1, Gc^1_2, ..., Gc^1_n]$ and $C_2=[Gc^2_1, Gc^2_2, ..., Gc^2_n]$. -->

### Mutation and Elitism

For every children, each gene has 1% chance of mutating.

The top 10% of the parents is automatically copied, as it, in the next generation.
