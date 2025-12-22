cmake_minimum_required(VERSION 3.16)
project(text-extraction-frontend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/ViewModels/MainViewModel.cpp
    src/UseCases/ExtractTextUseCase.cpp
    src/UI/Widget.cpp
    src/UI/Sidebar.cpp
)

add_library(tef_core INTERFACE)
target_include_directories(tef_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(SFML 3 COMPONENTS Graphics Window System QUIET)
if(SFML_FOUND)
    add_compile_definitions(USE_SFML)
    add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::System)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
else()
    message(STATUS "SFML not found; building console fallback")
    add_executable(${PROJECT_NAME} ${SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
