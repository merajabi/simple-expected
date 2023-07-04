cmake_minimum_required(VERSION 3.21)

project(LibSimpleExpected)

#find_package(SQLite3 CONFIG REQUIRED)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#add_executable(LibSimpleExpected)
add_library(LibSimpleExpected OBJECT)

target_sources(LibSimpleExpected
	PRIVATE
		src/SimpleExpected.h
	)

#target_link_libraries(LibSimpleExpected
#	PRIVATE
#		SQLite::SQLite3
#	)

target_include_directories(LibSimpleExpected
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
	)

# Enable testing functionality. Andoni: Do not use "enable_testing()"
include(CTest)
add_subdirectory(tests)

