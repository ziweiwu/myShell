project(smallsh C)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(.)

add_executable(smallsh
        builtin_commands.c
        builtin_commands.h
        main.c
        smallsh.c
        smallsh.h)
