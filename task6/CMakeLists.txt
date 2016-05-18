cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_library(lazy_string lazy_string.cpp)
install(FILES lazy_string.h DESTINATION include)
install(TARGETS lazy_string DESTINATION lib)
