cmake_minimum_required(VERSION 3.3)
project(task3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp rational.cpp rational.h)
add_executable(task3 ${SOURCE_FILES})