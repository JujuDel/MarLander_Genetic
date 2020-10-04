

# MarLander - Genetic Algorithm

Resolve the series of CodinGame's problem called Mars Lander using a Genetic Algorithm.

![gif](data/gif/level4.gif)

Links of the problems:
 - [Mars Lander - Episode 1](https://www.codingame.com/ide/puzzle/mars-lander-episode-1)
 - [Mars Lander - Episode 2](https://www.codingame.com/ide/puzzle/mars-lander-episode-2)
 - [Mars Lander - Episode 3](https://www.codingame.com/ide/puzzle/mars-lander-episode-3)
 - [Mars Lander - Optimization](https://www.codingame.com/ide/puzzle/mars-lander)

Solution developed in ***C++*** and using ***modern OpenGL*** for the visualization part.

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



## Genetic Algorithm
