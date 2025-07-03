# Find GoogleTest package
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Create test executable
add_executable(SimpleExpectedTests
    src/SimpleExpectedTests.cpp
    src/MoreExpectedTests.cpp
)

# Link dependencies
target_link_libraries(SimpleExpectedTests PRIVATE
    SimpleExpected
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SimpleExpectedTests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wformat=2
        -Wnull-dereference
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wold-style-cast
        -Wcast-align
#        -Werror
    )
endif()

# Add GoogleTest integration
include(GoogleTest)
gtest_discover_tests(SimpleExpectedTests
    EXTRA_ARGS --gtest_color=yes
)

