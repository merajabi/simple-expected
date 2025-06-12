# Find GoogleTest package
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Create test executable
add_executable(SimpleExpectedTests
    src/SimpleExpectedTests.cpp
)

# Link dependencies
target_link_libraries(SimpleExpectedTests PRIVATE
    SimpleExpected
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Add GoogleTest integration
include(GoogleTest)
gtest_discover_tests(SimpleExpectedTests
    EXTRA_ARGS --gtest_color=yes
)

