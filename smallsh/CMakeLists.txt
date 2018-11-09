project(smallsh C)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

include_directories(.)

add_executable(smallsh
        builtin_commands.c
        builtin_commands.h
        main.c
        signal_catchers.c
        signal_catchers.h
        smallsh.c
        smallsh.h)
