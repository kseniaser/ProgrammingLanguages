cmake_minimum_required(VERSION 3.3)

set(HEADERS rational.h)
set(SOURCES rational.cpp)
add_library(rational ${SOURCES} ${HEADERS})
install (TARGETS rational DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install (FILES rational.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install (FILES rational.cpp DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
