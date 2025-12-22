cmake_minimum_required(VERSION 3.16)
project(text-extraction-c-frontend VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer a local SFML copy in the repo if present
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/SFML-3.0.2")

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::System)

