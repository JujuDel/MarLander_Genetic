// Include standard headers
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>

// Include GLEW.
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <levels.hpp>
#include <Rocket.hpp>
#include <Utils.hpp>
#include <Genetic.hpp>

#define VISUALIZE_RESULT
//#define VISUALIZE_GENETIC

#ifdef VISUALIZE_GENETIC
static const size_t _SIZE_BUFFER_CHROMOSOME{ 3 * 2 * (_CHROMOSOME_SIZE - 1) };
#endif

bool _pause{ false };
bool _close{ false };

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        _close = true;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (_pause)
        {
            std::cout << "PLAY" << std::endl << std::endl;
        }
        else
        {
            std::cout << "PAUSE" << std::endl;
        }
        _pause = !_pause;
    }
}

int main()
{
    const int size_level{ size_level1 };
    const int* level{ level1 };
    GeneticPopulation population(rocket1, level, size_level);

#if defined(VISUALIZE_RESULT) || defined(VISUALIZE_GENETIC)
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(875, 375, "Mars Lander", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        int k{ getchar() };
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetKeyCallback(window, keyCallback);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programIDFloor = LoadShaders("../1_MarsLander_Genetic/shaders/FloorVertexShader.vertexshader", "../1_MarsLander_Genetic/shaders/FloorFragmentShader.fragmentshader");
    GLuint programIDLine = LoadShaders("../1_MarsLander_Genetic/shaders/RocketFireVertexShader.vertexshader", "../1_MarsLander_Genetic/shaders/RocketFireFragmentShader.fragmentshader");
    GLuint programIDRocket = LoadShaders("../1_MarsLander_Genetic/shaders/RocketVertexShader.vertexshader", "../1_MarsLander_Genetic/shaders/RocketFragmentShader.fragmentshader");

    // Convert from [0, _w] x [0, _h] to [-1, 1] x [-1, 1]
    GLfloat floor_buffer_data[3 * (2 * (size_level - 1))];
    for (int i = 0; i < size_level; ++i)
    {
        if (i == 0)
        {
            floor_buffer_data[0] = 2 * level[0] / _w - 1;
            floor_buffer_data[1] = 2 * level[1] / _h - 1;
            floor_buffer_data[2] = 0.f;
        }
        else if (i == size_level - 1)
        {
            const int idx{ 3 * (2 * size_level - 3) };
            floor_buffer_data[idx + 0] = 2 * level[2 * i + 0] / _w - 1;
            floor_buffer_data[idx + 1] = 2 * level[2 * i + 1] / _h - 1;
            floor_buffer_data[idx + 2] = 0.f;
        }
        else
        {
            const int idx{ 3 * (2 * i - 1) };
            floor_buffer_data[idx + 0] = 2 * level[2 * i + 0] / _w - 1;
            floor_buffer_data[idx + 1] = 2 * level[2 * i + 1] / _h - 1;
            floor_buffer_data[idx + 2] = 0.f;
            floor_buffer_data[idx + 3] = 2 * level[2 * i + 0] / _w - 1;
            floor_buffer_data[idx + 4] = 2 * level[2 * i + 1] / _h - 1;
            floor_buffer_data[idx + 5] = 0.f;
        }
    }

    GLuint floorbuffer;
    glGenBuffers(1, &floorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, floorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_buffer_data), floor_buffer_data, GL_STATIC_DRAW);
#endif
#ifdef VISUALIZE_GENETIC
    GLfloat rockets_line[_POPULATION_SIZE * _SIZE_BUFFER_CHROMOSOME];
    for (int chrom = 0; chrom < _POPULATION_SIZE; ++chrom)
    {
        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + 0] = 2.f * static_cast<GLfloat>(population.rocket_save.x) / _w - 1;
        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + 1] = 2.f * static_cast<GLfloat>(population.rocket_save.y) / _h - 1;
    }
#endif

    bool solutionFound{ false };
    int generation{ 0 };
    int idxChromosome{ 0 };
    int idxGene{ 0 };

    std::chrono::steady_clock::time_point start{ std::chrono::steady_clock::now() };

    while (!solutionFound)
    {
        std::cout << "Generation " << generation++ << std::endl;
        population.initRockets();

        // For every possible moves, i.e., for every genes
        for (int gen = 0; !solutionFound && gen < _CHROMOSOME_SIZE; ++gen)
        {
            // For every Rocket and their associated chromosome
            for (int chrom = 0; chrom < _POPULATION_SIZE; ++chrom)
            {
                Rocket* rocket{ population.getRocket(chrom) };
                if (rocket->isAlive)
                {
                    Gene* gene{ population.getChromosome(chrom)->getGene(gen) };

                    rocket->updateAngleAndPower(gene->angle, gene->power);
                    rocket->updateFuel();
                    rocket->updateAccSpeedPos();

                    const Line_d prev_curr{ {rocket->pX, rocket->pY}, {rocket->x, rocket->y} };
                    for (int k = 1; k < size_level; ++k)
                    {
                        const Line_d floor{ {level[2 * (k - 1)], level[2 * (k - 1) + 1]}, {level[2 * k], level[2 * k + 1]} };
                        if (rocket->x < 0 || rocket->x > _w || rocket->y < 0 || rocket->y > _h)
                        {
                            rocket->isAlive = false;
                            rocket->floor_id_crash = -1;
                        }
                        else if (isIntersect(prev_curr, floor))
                        {
                            rocket->isAlive = false;
                            rocket->floor_id_crash = k;

                            // Landing successful!
                            if (k == population.landing_zone_id && rocket->isParamSuccess())
                            {
                                std::cout << "Landing on " << k << " SUCCESS!" << std::endl;
                                solutionFound = true;
                                idxChromosome = chrom;
                                idxGene = gen;
                            }
                            break;
                        }
                    }
#ifdef VISUALIZE_GENETIC
                    const int idx{ 3 * (2 * gen + 1) };
                    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0] = static_cast<GLfloat>(2.f * rocket->x / _w - 1);
                    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1] = static_cast<GLfloat>(2.f * rocket->y / _h - 1);
                    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 2] = 0.f;
                    if (gen != _CHROMOSOME_SIZE - 1)
                    {
                        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 3] = rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0];
                        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 4] = rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1];
                        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 5] = 0.f;
                    }
                }
                else
                {
                    if (gen < 2)
                        continue;
                    const int idx{ 3 * (2 * gen + 1) };
                    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0] = static_cast<GLfloat>(2.f * rocket->x / _w - 1);
                    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1] = static_cast<GLfloat>(2.f * rocket->y / _h - 1);
                    rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 2] = 0.f;
                    if (gen != _CHROMOSOME_SIZE - 1)
                    {
                        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 3] = rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 0];
                        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 4] = rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 1];
                        rockets_line[chrom * _SIZE_BUFFER_CHROMOSOME + idx + 5] = 0.f;
                    }
#endif
                }
            }
        }
        if (!solutionFound)
            population.mutate();
        else
            break;

#ifdef VISUALIZE_GENETIC
        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programIDFloor);

        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, floorbuffer);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // Draw the line
        glDrawArrays(GL_LINES, 0, size_level * 2);

        for (int pop = 0; pop < _POPULATION_SIZE; ++pop)
        {
            glUseProgram(programIDLine);

            GLuint rocketbuffer;
            glGenBuffers(1, &rocketbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
            glBufferData(GL_ARRAY_BUFFER, _SIZE_BUFFER_CHROMOSOME * sizeof(rockets_line[0]), rockets_line + pop * _SIZE_BUFFER_CHROMOSOME, GL_STATIC_DRAW);

            glEnableVertexAttribArray(4);
            glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
            glVertexAttribPointer(
                4,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );

            // Draw the Rocket fire
            glDrawArrays(GL_LINES, 0, _CHROMOSOME_SIZE * 2);

            glDeleteBuffers(1, &rocketbuffer);
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(4);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
#endif
    }

    std::chrono::duration<double> elapsed_seconds{ std::chrono::steady_clock::now() - start };
    std::cout << "Execution time: " << elapsed_seconds.count() << "s\n";
    std::cout << "Generation: " << generation << std::endl;
    std::cout << "Child: " << idxChromosome << std::endl;
    std::cout << "Gene: " << idxGene << std::endl << std::endl;

    Chromosome* chromosome{ population.getChromosome(idxChromosome) };
    chromosome->chromosome[idxGene - 2].angle = 0;
    chromosome->chromosome[idxGene - 1].angle = 0;

#ifdef VISUALIZE_RESULT
    if (solutionFound)
    {
        Rocket rocket{ population.rocket_save };
        GLfloat GL_rocket_buffer_data[] = {
            0.f, 0.f, 0.f,
            0.f, 0.f, 0.f,
            0.f, 0.f, 0.f
        };
        GLfloat GL_fire_buffer_data[] = {
            0.f, 0.f, 0.f,
            0.f, 0.f, 0.f
        };

        const int number_loop_within_1_sec{ 10 };
        double number_loop{ 1 };
        int number_second{ 0 };
        do
        {
            // Clear the screen.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use our shader
            glUseProgram(programIDFloor);

            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, floorbuffer);
            glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );

            // Draw the line
            glDrawArrays(GL_LINES, 0, size_level * 2);
            glDisableVertexAttribArray(0);

            if (!_pause && rocket.isAlive)
            {
                if (number_loop >= number_loop_within_1_sec)
                {
                    rocket.debug(number_second);
                    number_loop = 0;

                    rocket.updateAngleAndPower(chromosome->getGene(number_second)->angle, chromosome->getGene(number_second)->power);

                    rocket.updateFuel();
                    rocket.updateAccSpeedPos();

                    number_second++;
                }

                number_loop++;
                updateBuffers(rocket, number_loop / number_loop_within_1_sec, GL_rocket_buffer_data, GL_fire_buffer_data);
                if (checkCollision(&GL_rocket_buffer_data[0], 9, &floor_buffer_data[0], 3 * (2 * (size_level - 1))))
                {
                    rocket.isAlive = false;
                }

            } // endif _pause && collision

            glUseProgram(programIDRocket);

            GLuint rocketbuffer;
            glGenBuffers(1, &rocketbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GL_rocket_buffer_data), GL_rocket_buffer_data, GL_STATIC_DRAW);

            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, rocketbuffer);
            glVertexAttribPointer(
                2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );
            // Draw the Rocket
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glDisableVertexAttribArray(2);

            glUseProgram(programIDLine);

            GLuint rocketfirebuffer;
            glGenBuffers(1, &rocketfirebuffer);
            glBindBuffer(GL_ARRAY_BUFFER, rocketfirebuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GL_fire_buffer_data), GL_fire_buffer_data, GL_STATIC_DRAW);

            glEnableVertexAttribArray(4);
            glBindBuffer(GL_ARRAY_BUFFER, rocketfirebuffer);
            glVertexAttribPointer(
                4,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );
            // Draw the Rocket fire
            glDrawArrays(GL_LINES, 0, 2);
            glDisableVertexAttribArray(4);

            // Swap buffers
            glfwSwapBuffers(window);
            glfwPollEvents();
        } // Check if the ESC key was pressed or the window was closed
        while (!_close && glfwWindowShouldClose(window) == 0);
    }

    // Cleanup VBO
    glDeleteBuffers(1, &floorbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programIDFloor);
    glDeleteProgram(programIDRocket);
    glDeleteProgram(programIDLine);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
#endif

    return 0;
}
