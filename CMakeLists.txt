cmake_minimum_required(VERSION 3.13)  # CMake version check
project(OSH)               # Create project "simple_example"

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES main.c main.c)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(OSH ${SOURCE_FILES})