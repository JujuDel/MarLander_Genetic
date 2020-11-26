// Include standard headers
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Include MarsLander headers
#include "Genetic.hpp"
#include "Rocket.hpp"
#include "Utils.hpp"
#include "common/shader.hpp"
#include "levels.hpp"

using namespace glm;
using namespace std;

// #######################################################
//
//                        GLOBALS
//
// #######################################################

//!< Size if of the buffer containing the genes of a chromosome.
static const size_t _SIZE_BUFFER_CHROMOSOME{3 * 2 * _CHROMOSOME_SIZE};

GLFWwindow *window; //!< The main window.

bool _pause{false}; //!< Play/pause status of the program.
bool _close{false}; //!< Close or not the main window.

extern const float _w{6999.f}; //!< Map width.
extern const float _h{2999.f}; //!< Map height.

// #######################################################
//
//                        OPENGL
//
// #######################################################

//! @brief  Callback functions on keypress events. Handles:
//!     - Spacebar: pause
//!     - Escape: close
//!
//! @param[in] window   The window in which the event happened.
//! @param[in] key      Keyboard key which has been pressed.
//! @param[in] scancode Platform-specific scancode.
//! @param[in] action   The key action.
//! @param[in] mods     The modifier key flags.
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  if (key == GLFW_KEY_ESCAPE) {
    _close = true;
  }
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    if (_pause) {
      cout << "PLAY" << endl << endl;
    } else {
      cout << "PAUSE" << endl;
    }
    _pause = !_pause;
  }
}

//! @brief  Initialize the environment:
//!     - Initialise GLFW
//!     - Open a window and create its OpenGL context
//!     - Initialize GLEW
//!     - Handles keypress event
//!     - Create and compile the GLSL program from the shaders
//!     - Prepare the data buffers
//!
//! @param[out] VertexArrayID    ID of the vertex array.
//! @param[out] programIDFloor   Red floor program
//! @param[out] programIDLine    Blue lines program.
//! @param[out] programIDRocket  Green rocket program.
//! @param[out] floorbuffer      Buffer containing the floor data.
//!
//! @return 0 on success, -1 on failure.
int initOpenGL(GLuint &VertexArrayID, GLuint &programIDFloor,
               GLuint &programIDLine, GLuint &programIDRocket,
               GLuint &floorbuffer) {
  // Initialise GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);               // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE,
                 GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(875, 375, "Mars Lander", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, "
                    "they are not 3.3 compatible.\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed in core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    int k{getchar()};
    glfwTerminate();
    return -1;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetKeyCallback(window, keyCallback);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.2f, 0.0f);

  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // Create and compile our GLSL program from the shaders
  programIDFloor = LoadShaders(
      "../1_MarsLander_Genetic/shaders/FloorVertexShader.vertexshader",
      "../1_MarsLander_Genetic/shaders/FloorFragmentShader.fragmentshader");
  programIDLine = LoadShaders(
      "../1_MarsLander_Genetic/shaders/RocketFireVertexShader.vertexshader",
      "../1_MarsLander_Genetic/shaders/"
      "RocketFireFragmentShader.fragmentshader");
  programIDRocket = LoadShaders(
      "../1_MarsLander_Genetic/shaders/RocketVertexShader.vertexshader",
      "../1_MarsLander_Genetic/shaders/RocketFragmentShader.fragmentshader");

  // Convert from [0, _w] x [0, _h] to [-1, 1] x [-1, 1]
  GLfloat floor_buffer_data[3 * (2 * (size_level - 1))];
  for (int i = 0; i < size_level; ++i) {
    if (i == 0) {
      floor_buffer_data[0] = 2 * level[0] / _w - 1;
      floor_buffer_data[1] = 2 * level[1] / _h - 1;
      floor_buffer_data[2] = 0.f;
    } else if (i == size_level - 1) {
      const int idx{3 * (2 * size_level - 3)};
      floor_buffer_data[idx + 0] = 2 * level[2 * i + 0] / _w - 1;
      floor_buffer_data[idx + 1] = 2 * level[2 * i + 1] / _h - 1;
      floor_buffer_data[idx + 2] = 0.f;
    } else {
      const int idx{3 * (2 * i - 1)};
      floor_buffer_data[idx + 0] = 2 * level[2 * i + 0] / _w - 1;
      floor_buffer_data[idx + 1] = 2 * level[2 * i + 1] / _h - 1;
      floor_buffer_data[idx + 2] = 0.f;
      floor_buffer_data[idx + 3] = 2 * level[2 * i + 0] / _w - 1;
      floor_buffer_data[idx + 4] = 2 * level[2 * i + 1] / _h - 1;
      floor_buffer_data[idx + 5] = 0.f;
    }
  }

  glGenBuffers(1, &floorbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, floorbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(floor_buffer_data), floor_buffer_data,
               GL_STATIC_DRAW);

  return 0;
}

// #######################################################
//
//                        MAIN
//
// #######################################################

int main() {
  // -----------------------------------------------------
  //               SOME INITIALIZATION
  // -----------------------------------------------------
  srand(time(NULL));

  // Rocket, level and size_level are defined in `level.hpp`
  GeneticPopulation population(rocket, level, size_level);

  GLuint VertexArrayID, programIDFloor, programIDLine, programIDRocket,
      floorbuffer;
  if (initOpenGL(VertexArrayID, programIDFloor, programIDLine, programIDRocket,
                 floorbuffer) == -1)
    return -1;

  GLfloat rockets_line[_POPULATION_SIZE * _SIZE_BUFFER_CHROMOSOME];
  for (int chrom = 0; chrom < _POPULATION_SIZE; ++chrom) {
    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + 0] =
        2.f * static_cast<GLfloat>(population.rocket_save.x) / _w - 1;
    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + 1] =
        2.f * static_cast<GLfloat>(population.rocket_save.y) / _h - 1;
  }

  bool solutionFound{false};
  int generation{0};
  int idxChromosome{0};
  int idxGene{0};
  int prevGeneration{0};

  vector<Gene> solutionIncremental;

  chrono::high_resolution_clock::time_point start{
      std::chrono::high_resolution_clock::now()};

  chrono::high_resolution_clock::time_point start_loop{
      chrono::high_resolution_clock::now()};
  int idxStart{0};

  // -----------------------------------------------------
  //                  MAIN GENETIC LOOP
  // -----------------------------------------------------
  while (!solutionFound && !_close && glfwWindowShouldClose(window) == 0) {
    cout << "Generation " << generation << endl;

    // ...................................................
    //                 INCREMENTAL SEARCH
    // ...................................................
    chrono::duration<double> elapsed_seconds{
        chrono::high_resolution_clock::now() - start_loop};
    if (elapsed_seconds.count() > 0.15) {
      Gene *bestGen{population.getChromosome(0)->getGene(idxStart)};

      cerr << "Approx done at generation " << generation << " - "
           << generation - prevGeneration << endl;
      cerr << "Idx: " << idxStart << endl;
      cerr << "  Angle: " << (int)bestGen->angle << endl;
      cerr << "  Power: " << (int)bestGen->thrust << endl;

      population.rocket_save.updateRocket(bestGen->angle, bestGen->thrust);
      cout << (int)population.rocket_save.angle << " "
           << (int)population.rocket_save.thrust << endl;

      solutionIncremental.push_back({bestGen->angle, bestGen->thrust});

      prevGeneration = generation;
      idxStart++;
      start_loop = chrono::high_resolution_clock::now();
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
                cout << "Landing SUCCESS!" << endl << endl;
                solutionFound = true;
                idxChromosome = chrom;
                idxGene = gen;
              }
              break;
            }
          }
          const int idx{3 * (2 * gen + 1)};

          rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0] =
              static_cast<GLfloat>(2.f * rocket->x / _w - 1);

          rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1] =
              static_cast<GLfloat>(2.f * rocket->y / _h - 1);

          rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 2] = 0.f;

          if (gen != _CHROMOSOME_SIZE - 1) {
            rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 3] =
                rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0];

            rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 4] =
                rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1];

            rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 5] = 0.f;
          }
        } else {
          if (gen < 2)
            continue;
          const int idx{3 * (2 * gen + 1)};

          rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0] =
              static_cast<GLfloat>(2.f * rocket->x / _w - 1);

          rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1] =
              static_cast<GLfloat>(2.f * rocket->y / _h - 1);

          rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 2] = 0.f;

          if (gen != _CHROMOSOME_SIZE - 1) {
            rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 3] =
                rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0];

            rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 4] =
                rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1];

            rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 5] = 0.f;
          }
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

    // ...................................................
    //                  OPENGL DISPLAY
    // ...................................................
    // Clear the screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programIDFloor);

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, floorbuffer);
    glVertexAttribPointer(0, // attribute 0. No particular reason for 0, but
                             // must match the layout in the shader.
                          3, // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0,        // stride
                          (void *)0 // array buffer offset
    );

    // Draw the line
    glDrawArrays(GL_LINES, 0, size_level * 2);
    glDisableVertexAttribArray(0);

    for (int pop = 0; pop < _POPULATION_SIZE; pop++) {
      glUseProgram(programIDLine);

      GLuint rocketbuffer;
      glGenBuffers(1, &rocketbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
      glBufferData(
          GL_ARRAY_BUFFER, _SIZE_BUFFER_CHROMOSOME * sizeof(rockets_line[0]),
          rockets_line + pop * _SIZE_BUFFER_CHROMOSOME, GL_STATIC_DRAW);

      glEnableVertexAttribArray(4);
      glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
      glVertexAttribPointer(4, // attribute 4. No particular reason for 4, but
                               // must match the layout in the shader.
                            3, // size
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void *)0 // array buffer offset
      );

      // Draw the rocket lines
      glDrawArrays(GL_LINES, 0, _CHROMOSOME_SIZE * 2);
      glDeleteBuffers(1, &rocketbuffer);
    }

    glDisableVertexAttribArray(4);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  chrono::duration<double> elapsed_seconds{
      chrono::high_resolution_clock::now() - start};
  cout << "Execution time: " << elapsed_seconds.count() << "s" << endl;

  // -----------------------------------------------------
  //                 DISPLAY THE SOLUTION
  // -----------------------------------------------------
  if (solutionFound) {
    cout << endl;
    cout << "Solution found at:" << endl;
    cout << "  Generation: " << generation << endl;
    cout << "  Child: " << idxChromosome << endl;
    cout << "  Gene: " << idxGene << endl << endl;

    Rocket rocket_res{rocket};
    GLfloat GL_rocket_buffer_data[] = {0.f, 0.f, 0.f, 0.f, 0.f,
                                       0.f, 0.f, 0.f, 0.f};
    GLfloat GL_fire_buffer_data[] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

    Chromosome *chromosome{population.getChromosome(idxChromosome)};

    const int number_loop_within_1_sec{2};
    double number_loop{1};
    int number_second{0};
    rocket_res.debug();

    // Check if the ESC key was pressed or the window was closed
    while (!_close && glfwWindowShouldClose(window) == 0) {
      // Clear the screen.
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Use our shader
      glUseProgram(programIDFloor);

      // 1st attribute buffer : vertices
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, floorbuffer);
      glVertexAttribPointer(0, // attribute 0. No particular reason for 0, but
                               // must match the layout in the shader.
                            3, // size
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void *)0 // array buffer offset
      );

      // Draw the line
      glDrawArrays(GL_LINES, 0, size_level * 2);
      glDisableVertexAttribArray(0);

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
          cout << "  Request: " << (int)rocket_res.angle << " "
               << (int)rocket_res.thrust << endl
               << endl;

          rocket_res.debug();
          number_second++;
        }

        number_loop++;
        updateBuffers(rocket_res, number_loop / number_loop_within_1_sec,
                      GL_rocket_buffer_data, GL_fire_buffer_data);
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
      } // endif _pause && alive

      glUseProgram(programIDRocket);

      GLuint rocketbuffer;
      glGenBuffers(1, &rocketbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GL_rocket_buffer_data),
                   GL_rocket_buffer_data, GL_STATIC_DRAW);

      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
      glVertexAttribPointer(2, // attribute 0. No particular reason for 0, but
                               // must match the layout in the shader.
                            3, // size
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void *)0 // array buffer offset
      );
      // Draw the Rocket
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDisableVertexAttribArray(2);

      glDeleteBuffers(1, &rocketbuffer);

      glUseProgram(programIDLine);

      GLuint rocketfirebuffer;
      glGenBuffers(1, &rocketfirebuffer);
      glBindBuffer(GL_ARRAY_BUFFER, rocketfirebuffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GL_fire_buffer_data),
                   GL_fire_buffer_data, GL_STATIC_DRAW);

      glEnableVertexAttribArray(4);
      glBindBuffer(GL_ARRAY_BUFFER, rocketfirebuffer);
      glVertexAttribPointer(4, // attribute 0. No particular reason for 0, but
                               // must match the layout in the shader.
                            3, // size
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void *)0 // array buffer offset
      );
      // Draw the Rocket fire
      glDrawArrays(GL_LINES, 0, 2);
      glDisableVertexAttribArray(4);

      glDeleteBuffers(1, &rocketfirebuffer);

      // Swap buffers
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    // Cleanup VBO
    glDeleteBuffers(1, &floorbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programIDFloor);
    glDeleteProgram(programIDRocket);
    glDeleteProgram(programIDLine);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
  }
  // No solution
  else {
    // Cleanup VBO
    glDeleteBuffers(1, &floorbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programIDFloor);
    glDeleteProgram(programIDRocket);
    glDeleteProgram(programIDLine);
    cout << "No solution found..." << endl;
  }

  return 0;
}
