#define _USE_MATH_DEFINES

#include <iostream>
#include <math.h>

#include "Rocket.hpp"
#include "Visualize.hpp"
#include "common/shader.hpp"
#include "levels.hpp"
#include "utils.hpp"

extern const float _w; //!< Map width.
extern const float _h; //!< Map height.

extern bool _pause{false}; //!< Play/pause status of the program.
extern bool _close{false}; //!< Close or not the main window.

constexpr int _base{100};   //!< Size of the base of the rocket.
constexpr int _height{150}; //!< Size of the height of the rocket.
constexpr int _fire{25};    //!< Size of the maximum thrust power vizu.

/************************************************************/
Visualization_OpenGL::Visualization_OpenGL(const Rocket &f_rocket)
    : VertexArrayID{0}, programIDFloor{0}, programIDLine{0}, programIDRocket{0},
      floorbuffer{0}, m_window{nullptr} {
  for (int chrom = 0; chrom < _POPULATION_SIZE; ++chrom) {
    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + 0] =
        2.f * static_cast<GLfloat>(f_rocket.x) / _w - 1;
    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + 1] =
        2.f * static_cast<GLfloat>(f_rocket.y) / _h - 1;
  }

  for (int i = 0; i < 9; ++i)
    GL_rocket_buffer_data[i] = 0.;
  for (int i = 0; i < 6; ++i)
    GL_fire_buffer_data[i] = 0.;
}

/************************************************************/
int Visualization_OpenGL::initOpenGL() {
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
  m_window = glfwCreateWindow(875, 375, "Mars Lander", NULL, NULL);
  if (m_window == NULL) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, "
                    "they are not 3.3 compatible.\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(m_window);

  // Initialize GLEW
  glewExperimental = true; // Needed in core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    int k{getchar()};
    glfwTerminate();
    return -1;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetKeyCallback(m_window, keyCallback);

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

/************************************************************/
GLFWwindow *Visualization_OpenGL::getWindow() { return m_window; }

/************************************************************/
void Visualization_OpenGL::updateRocketLine(const Rocket *f_rocket,
                                            const int f_gen,
                                            const int f_chrom) {
  const int idx{3 * (2 * f_gen + 1)};

  rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0] =
      static_cast<GLfloat>(2.f * f_rocket->x / _w - 1);

  rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1] =
      static_cast<GLfloat>(2.f * f_rocket->y / _h - 1);

  rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 2] = 0.f;

  if (f_gen != _CHROMOSOME_SIZE - 1) {
    rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 3] =
        rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0];

    rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 4] =
        rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1];

    rockets_line[f_chrom * _SIZE_BUFFER_CHROMOSOME + idx + 5] = 0.f;
  }
}

/************************************************************/
void Visualization_OpenGL::updateSingleRocket(const Rocket &f_rocket,
                                              const double f_elapsed) {
  assert(0 <= f_elapsed <= 1);

  /* Formulaes:
   *    pos.x = acc.x * t^2 / 2 + vx0 * t + x0
   *    pos.y = acc.y * t^2 / 2 + vy0 * t + y0
   */

  const double x{f_rocket.ax * std::pow(f_elapsed, 2) / 2. +
                 f_rocket.vx * f_elapsed + f_rocket.x};
  const double y{f_rocket.ay * std::pow(f_elapsed, 2) / 2. +
                 f_rocket.vy * f_elapsed + f_rocket.y};

  Coord_d F{0, -1. * _fire * f_rocket.thrust};
  Coord_d P1{0, _height};
  Coord_d P2{_base / 2.f, 0};
  Coord_d P3{-_base / 2.f, 0};

  const double angle_rad{f_rocket.angle * M_PI / 180.};
  const double c{cos(angle_rad)};
  const double s{sin(angle_rad)};

  applyRotation(F, c, s);
  applyRotation(P1, c, s);
  applyRotation(P2, c, s);
  applyRotation(P3, c, s);

  GL_rocket_buffer_data[0] = static_cast<GLfloat>(P1.x + x);
  GL_rocket_buffer_data[1] = static_cast<GLfloat>(P1.y + y);
  GL_rocket_buffer_data[3] = static_cast<GLfloat>(P2.x + x);
  GL_rocket_buffer_data[4] = static_cast<GLfloat>(P2.y + y);
  GL_rocket_buffer_data[6] = static_cast<GLfloat>(P3.x + x);
  GL_rocket_buffer_data[7] = static_cast<GLfloat>(P3.y + y);
  GL_fire_buffer_data[0] = static_cast<GLfloat>(x);
  GL_fire_buffer_data[1] = static_cast<GLfloat>(y);
  GL_fire_buffer_data[3] = static_cast<GLfloat>(F.x + x);
  GL_fire_buffer_data[4] = static_cast<GLfloat>(F.y + y);

  // From [0, _w] x [0, _h] to [-1, 1] x [-1, 1]
  GL_rocket_buffer_data[0] = 2.f * GL_rocket_buffer_data[0] / _w - 1.f;
  GL_rocket_buffer_data[1] = 2.f * GL_rocket_buffer_data[1] / _h - 1.f;
  GL_rocket_buffer_data[3] = 2.f * GL_rocket_buffer_data[3] / _w - 1.f;
  GL_rocket_buffer_data[4] = 2.f * GL_rocket_buffer_data[4] / _h - 1.f;
  GL_rocket_buffer_data[6] = 2.f * GL_rocket_buffer_data[6] / _w - 1.f;
  GL_rocket_buffer_data[7] = 2.f * GL_rocket_buffer_data[7] / _h - 1.f;
  GL_fire_buffer_data[0] = 2.f * GL_fire_buffer_data[0] / _w - 1.f;
  GL_fire_buffer_data[1] = 2.f * GL_fire_buffer_data[1] / _h - 1.f;
  GL_fire_buffer_data[3] = 2.f * GL_fire_buffer_data[3] / _w - 1.f;
  GL_fire_buffer_data[4] = 2.f * GL_fire_buffer_data[4] / _h - 1.f;
}

/************************************************************/
void Visualization_OpenGL::drawPopulation() {
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
    glBufferData(GL_ARRAY_BUFFER,
                 _SIZE_BUFFER_CHROMOSOME * sizeof(rockets_line[0]),
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
  glfwSwapBuffers(m_window);
  glfwPollEvents();
}

/************************************************************/
void Visualization_OpenGL::drawSingleRocket() {
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
  glfwSwapBuffers(m_window);
  glfwPollEvents();
}

/************************************************************/
void Visualization_OpenGL::end() {
  // Cleanup VBO
  glDeleteBuffers(1, &floorbuffer);
  glDeleteVertexArrays(1, &VertexArrayID);
  glDeleteProgram(programIDFloor);
  glDeleteProgram(programIDRocket);
  glDeleteProgram(programIDLine);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
}

/************************************************************/
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  if (key == GLFW_KEY_ESCAPE) {
    _close = true;
  }
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    if (_pause) {
      std::cout << "PLAY" << std::endl << std::endl;
    } else {
      std::cout << "PAUSE" << std::endl;
    }
    _pause = !_pause;
  }
}
