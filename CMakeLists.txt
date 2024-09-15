cmake_minimum_required(VERSION 3.29)
project(3DVoxelEngineV1)

set(CMAKE_CXX_STANDARD 26)

# Add the executable target first
add_executable(3DVoxelEngineV1
        main.cpp
        # Add other source files ehre
        glad/src/glad.c
)

# Include directories
target_include_directories(3DVoxelEngineV1 PRIVATE
    glad/include
)

add_definitions(-DSHADER_DIR="${CMAKE_SOURCE_DIR}/shaders")

# Find the glfw3 package
find_package(glfw3 CONFIG REQUIRED)

# Link glfw3 to your executable target
target_link_libraries(3DVoxelEngineV1 PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(3DVoxelEngineV1 PRIVATE glm::glm)