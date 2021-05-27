# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (MarsLander_Genetic)

find_package(OpenGL REQUIRED)

# Compile external dependencies 
add_subdirectory (external)


include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	1_MarsLander_Genetic/
	./
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# 0_TestOpenGL
add_executable(cube3D
	0_TestOpenGL/cube.cpp
	common/shader.cpp
	common/shader.hpp
	common/quaternion_utils.cpp
	common/quaternion_utils.hpp
	0_TestOpenGL/shaders/TransformVertexShader.vertexshader
	0_TestOpenGL/shaders/ColorFragmentShader.fragmentshader
)
target_link_libraries(cube3D
	${ALL_LIBS}
)

# 1_MarsLander_Genetic
add_executable(MarsLander_Genetic
	1_MarsLander_Genetic/main.cpp
	common/shader.cpp
	common/shader.hpp
	1_MarsLander_Genetic/Rocket.cpp
	1_MarsLander_Genetic/Rocket.hpp
	1_MarsLander_Genetic/Genetic.cpp
	1_MarsLander_Genetic/Genetic.hpp
	1_MarsLander_Genetic/Utils.cpp
	1_MarsLander_Genetic/Utils.hpp
	1_MarsLander_Genetic/Levels.cpp
	1_MarsLander_Genetic/Levels.hpp
	1_MarsLander_Genetic/Visualize.cpp
	1_MarsLander_Genetic/Visualize.hpp
	1_MarsLander_Genetic/shaders/FloorFragmentShader.fragmentshader
	1_MarsLander_Genetic/shaders/FloorVertexShader.vertexshader
	1_MarsLander_Genetic/shaders/RocketFireFragmentShader.fragmentshader
	1_MarsLander_Genetic/shaders/RocketFireVertexShader.vertexshader
	1_MarsLander_Genetic/shaders/RocketFragmentShader.fragmentshader
	1_MarsLander_Genetic/shaders/RocketVertexShader.vertexshader

)
target_link_libraries(MarsLander_Genetic
	${ALL_LIBS}
)

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )
