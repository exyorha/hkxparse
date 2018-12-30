add_library(jsoncpp STATIC
	${CMAKE_CURRENT_BINARY_DIR}/jsoncpp.cpp
	${CMAKE_CURRENT_BINARY_DIR}/include/json/json.h
	${CMAKE_CURRENT_BINARY_DIR}/include/json/json-forwards.h
)

target_include_directories(jsoncpp PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp.cpp
	       ${CMAKE_CURRENT_BINARY_DIR}/include/json/json.h
	       ${CMAKE_CURRENT_BINARY_DIR}/include/json/json-forwards.h
	COMMAND ${PYTHON_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/amalgamate.py
			-s ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp.cpp
			-i ${CMAKE_CURRENT_BINARY_DIR}/include/json/json.h
			-t ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp
	VERBATIM
)
