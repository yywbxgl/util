cmake_minimum_required(VERSION 3.14)
project(sun_hello)

add_library(sun_hello SHARED hello.cpp)
install(TARGETS sun_hello DESTINATION lib)
target_include_directories(sun_hello PUBLIC ./)

add_executable(sun_hello_test   test.cpp)
target_link_libraries(sun_hello_test sun_hello)
install(TARGETS sun_hello_test DESTINATION bin)
