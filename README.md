
# MarLander - Genetic Algorithm

Resolve the series of CodinGame's problem called Mars Lander using a Genetic Algorithm.

![gif](data/gif/level4.gif)

Links of the problems:
 - [Mars Lander - Episode 1](https://www.codingame.com/ide/puzzle/mars-lander-episode-1)
 - [Mars Lander - Episode 2](https://www.codingame.com/ide/puzzle/mars-lander-episode-2)
 - [Mars Lander - Episode 3](https://www.codingame.com/ide/puzzle/mars-lander-episode-3)
 - [Mars Lander - Optimization](https://www.codingame.com/ide/puzzle/mars-lander)

Solution developed in ***C++*** and using ***modern OpenGL*** for the visualization part.

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
3) Line 60 to 62, you can modify the initialization of the variables *"size_level"*, *"level"* and *"population"* to play around with different levels, available in *"levels.hpp"*

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

Doc / dev - ongoing

Selection -> Roulette Wheel

Crossover -> Weighted sum of the parents

Mutation -> classiq mutation

Elitism