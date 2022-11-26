cmake_minimum_required(VERSION 3.10)

project(game)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rdparty)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

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
    pixeltype.h
    pixmap.cc
    pixmap.h
    abstracttexture.h
    texture.cc
    texture.h
    lazytexture.cc
    lazytexture.h
    textureatlaspage.cc
    textureatlaspage.h
    textureatlas.cc
    textureatlas.h
    glyphcache.cc
    glyphcache.h
    painter.cc
    painter.h
    shadermanager.cc
    shadermanager.h
    spritebatcher.cc
    spritebatcher.h
    system.cc
    system.h
    miniui.cc
    miniui.h
    fontcache.cc
    fontcache.h
    font.cc
    font.h
    pixmapcache.cc
    pixmapcache.h
)

add_executable(game ${SOURCES})

target_link_libraries(game
    PRIVATE
        glm
        stb
        GLEW::GLEW
        OpenGL::GL
)

target_link_libraries(game PRIVATE glfw)
