cmake_minimum_required(VERSION 3.25)
project(SimpleArgsParser)

set(CMAKE_CXX_STANDARD 23)
include_directories(include)

# including tests
file(GLOB google_test_source "test/*.cpp")

add_executable(SimpleArgsParser main.cpp ${google_test_source})
target_link_libraries(SimpleArgsParser gtest pthread)
