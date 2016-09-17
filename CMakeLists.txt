cmake_minimum_required (VERSION 2.6)
project (ThreadedMandelbrot)
add_definitions(-std=c++11)
add_executable (ThreadedMandel Timer.h Main.cpp)
