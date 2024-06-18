cmake_minimum_required(VERSION 3.14)
project(hello)

add_library(hello SHARED hello.cpp)
install(TARGETS hello DESTINATION lib)

add_executable(hello_test   test.cpp)
target_link_libraries(hello_test hello)
install(TARGETS hello_test DESTINATION bin)
