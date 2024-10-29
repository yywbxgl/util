project(acc_robotaxi)
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_BUILD_TYPE "Debug")


add_subdirectory(execution_manager)
add_subdirectory(state_manager)


add_subdirectory(monitor)
# add_subdirectory(monitor_dv)
# add_subdirectory(googletest)


