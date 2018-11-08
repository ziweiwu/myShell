cmake_minimum_required(VERSION 2.8)
project(smallsh)

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(.)

add_executable(src
        main.c build_in_commands.c build_in_commands.h)
