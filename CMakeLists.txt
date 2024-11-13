cmake_minimum_required(VERSION 3.16)

project(deimos VERSION 1.0.0 LANGUAGES CXX C)

# Set CMake language requirements
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build components
add_subdirectory("${PROJECT_SOURCE_DIR}/src/extern")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/app")