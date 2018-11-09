project(smallsh)

set(CMAKE_C_FLAGS "-std=c99")
include_directories(.)

add_executable(
        builtin_commands.c
        builtin_commands.h
        main.c
        signal_catchers.c
        signal_catchers.h
        smallsh.c
        smallsh.h
)
