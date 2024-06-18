cmake_minimum_required(VERSION 3.14)
project(util)

add_executable(hello_test   test.cpp)
install(TARGETS hello_test DESTINATION bin)
