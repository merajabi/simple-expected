cmake_minimum_required(VERSION 3.15)
project(SimpleExpected VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_TESTING "Build tests" OFF)

# Set C++ standard (modify if needed)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create interface library for header-only target
add_library(SimpleExpected INTERFACE)
target_include_directories(SimpleExpected INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTING)
    # Enable testing
    enable_testing()

    # Add tests subdirectory
    add_subdirectory(tests)
endif()
