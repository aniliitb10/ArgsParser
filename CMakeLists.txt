cmake_minimum_required(VERSION 3.25)
project(ArgsParser)

set(CMAKE_CXX_STANDARD 20)
include_directories(include)

add_subdirectory(test)

add_executable(ArgsParser main.cpp)
target_link_libraries(ArgsParser)
