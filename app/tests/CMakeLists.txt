
find_package(GTest CONFIG REQUIRED)

add_executable(SimpleExpectedTests)

target_sources(SimpleExpectedTests
	PRIVATE
		src/SimpleExpectedTests.cpp
	)

target_link_libraries(SimpleExpectedTests
	PRIVATE
		LibSimpleExpected

		GTest::gtest
		GTest::gtest_main
	)

target_include_directories(SimpleExpectedTests
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
	)
	
add_test(NAME SimpleExpectedTests COMMAND SimpleExpectedTests)
