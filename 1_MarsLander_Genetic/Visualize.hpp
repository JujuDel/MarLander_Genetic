#ifndef VISUALIZE_HPP
#define VISUALIZE_HPP

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#include "Genetic.hpp"

//!< Size if of the buffer containing the genes of a chromosome.
constexpr size_t _SIZE_BUFFER_CHROMOSOME{3 * 2 * _CHROMOSOME_SIZE};

//! brief  Visualization class
class Visualization_OpenGL {
public:
  //! @brief  C'tor.
  //!
  //! @param[in] f_rocket  The rocket to initialize with.
  Visualization_OpenGL(const Rocket &f_rocket, const int *const level,
                       const int size_level);

  //! @brief  Initialize the environment:
  //!     - Initialise GLFW
  //!     - Open a window and create its OpenGL context
  //!     - Initialize GLEW
  //!     - Handles keypress event
  //!     - Create and compile the GLSL program from the shaders
  //!     - Prepare the data buffers
  //!
  //! @param[out] VertexArrayID
  //! @param[out] programIDFloor
  //! @param[out] programIDLine
  //! @param[out] programIDRocket
  //! @param[out] floorbuffer
  //!
  //! @return 0 on success, -1 on failure.
  int initOpenGL();

  //! @brief  Get a pointer to the opaque windows object.
  //!
  //! @return A pointer to the opaque windows object.
  GLFWwindow *getWindow();

  //! @brief  Update the OpenGL line of one rocket among the whole population.
  //!
  //! @param[in] f_rocket  A current rocket state.
  //! @param[in] f_gen     Its gene index within its chromosome.
  //! @param[in] f_chrom   Its chromosome index within the population.
  void updateRocketLine(const Rocket *f_rocket, const int f_gen,
                        const int f_chrom);

  //! @brief  Update the OpenGL buffers for the single rocket.
  //!
  //! @param[in] f_rocket   The rocket.
  //! @param[in] f_elapsed  Elapsed time since the previous update.
  void updateSingleRocket(const Rocket &f_rocket, const double f_elapsed);

  //! @brief  Draw the whole population.
  void drawPopulation();

  //! @brief  Draw the single rocket.
  void drawSingleRocket();

  //! @brief  Cleanup VBO, close OpenGL window and terminate GLFW.
  void end();

private:
  GLFWwindow *m_window; //!< The main window.

  GLuint VertexArrayID;   //!< ID of the vertex array.
  GLuint programIDFloor;  //!< Red floor program
  GLuint programIDLine;   //!< Blue lines program.
  GLuint programIDRocket; //!< Green rocket program.
  GLuint floorbuffer;     //!< Buffer containing the floor data.

  GLfloat rockets_line[_POPULATION_SIZE *
                       _SIZE_BUFFER_CHROMOSOME]; //!< The population buffer.

  GLfloat GL_rocket_buffer_data[9]; //!< Single rocket triangle buffer.
  GLfloat GL_fire_buffer_data[6];   //!< Single rocket thrust power buffer.

  const int *const m_level; //!< Poiter to the current floor data buffer.
  const int m_size_level;   //!< Size of the current floor data buffer.
};

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
                 int mods);

#endif
