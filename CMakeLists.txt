cmake_minimum_required(VERSION 3.23)
project(nes)

set(CMAKE_CXX_STANDARD 20)

add_executable(nes src/main.cpp src/cpu.cpp src/nes.h src/mem.cpp)
