cmake_minimum_required(VERSION 3.15)
project(test_package LANGUAGES CXX)

find_package(SimpleExpected REQUIRED CONFIG)

# Create test executable
add_executable(example src/example.cpp)
target_link_libraries(example PRIVATE SimpleExpected::SimpleExpected)

# Add test
enable_testing()
add_test(NAME example COMMAND example)
