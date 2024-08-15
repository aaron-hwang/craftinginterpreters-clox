cmake_minimum_required(VERSION 3.28)
project(clox C)

set(CMAKE_C_STANDARD 11)

add_executable(clox main/main.c
        main/common.h
        main/chunk.h
        main/chunk.c
        main/memory.h
        main/memory.c
        main/debug.h
        main/debug.c
        main/value.c
        main/value.h
        main/vm.c
        main/vm.h)
