cmake_minimum_required(VERSION 3.10)

project(game)

set(CMAKE_CXX_STANDARD 20)

option(USE_GLFW3 "Use GLFW3" OFF)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if (USE_GLFW3)
    find_package(glfw3 REQUIRED)
else()
    find_package(SDL2 REQUIRED)
endif()

set(SOURCES
    main.cc
    game.cc
    game.h
    buffer.cc
    buffer.h
    mesh.h
    ioutil.cc
    ioutil.h
    shaderprogram.cc
    shaderprogram.h
)

add_executable(game ${SOURCES})

target_link_libraries(game
    PRIVATE
        GLEW::GLEW
        OpenGL::GL
)

if (USE_GLFW3)
    target_link_libraries(game PRIVATE glfw)
    target_compile_definitions(game PRIVATE USE_GLFW3)
else()
    target_link_libraries(game PRIVATE SDL2::SDL2)
endif()
