cmake_minimum_required(VERSION 3.25)
project(ArgsParserTest)

set(CMAKE_CXX_STANDARD 20)

# including tests, includes test_main.cpp too
file(GLOB google_test_source "./unit_tests/*.cpp")

add_executable(ArgsParserTest gtest_main.cpp test_util.cpp ${google_test_source})
target_link_libraries(ArgsParserTest gtest pthread)
