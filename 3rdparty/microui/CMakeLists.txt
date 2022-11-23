add_library(microui STATIC
    src/microui.c
    src/microui.h
)

target_include_directories(microui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
