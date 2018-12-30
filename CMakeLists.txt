cmake_minimum_required(VERSION 3.12)
project(hkxparse)

find_package(PythonInterp REQUIRED)

add_subdirectory(jsoncpp)
add_subdirectory(hkxparse)
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
	add_subdirectory(hkxparse-test)
endif()

