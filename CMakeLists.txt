cmake_minimum_required(VERSION 3.28)
project(table C)

set(CMAKE_C_STANDARD 11)

add_executable(table main.c
        minilibs/utils.h)
