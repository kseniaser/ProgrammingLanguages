cmake_minimum_required(VERSION 3.4)

project(rational)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(rational SHARED rational.cpp)

install (FILES rational.h DESTINATION include)
install (TARGETS rational DESTINATION lib)
