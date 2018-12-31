cmake_minimum_required(VERSION 3.12)
project(hkxparse)

find_program(RUBY_EXECUTABLE ruby)
if(NOT RUBY_EXECUTABLE)
	message(FATAL_ERROR "Ruby is required")
endif()

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(packfile-layout)
add_subdirectory(hkxparse)

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
	add_subdirectory(hkxparse-test)
endif()

