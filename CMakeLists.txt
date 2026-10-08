cmake_minimum_required(VERSION 3.20)

project(lab_02_variant_11 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Werror -pedantic)

add_executable(child1 src/child1.c)
add_executable(child2 src/child2.c)
add_executable(parent src/parent.c)