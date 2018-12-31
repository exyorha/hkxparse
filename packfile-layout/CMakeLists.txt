set(HKXPARSE_PACKFILE_LAYOUT_FILES
	hk_2010.2.0-r1_1-0-1-4 # Skyrim 
)

add_library(hkxparse-packfile-layout STATIC
  include/hkxparse/HavokPackfileLayouts.h
  include/hkxparse/HavokReflectionTypes.h
  HavokPackfileLayouts.cpp
)
target_include_directories(hkxparse-packfile-layout PUBLIC include)
target_include_directories(hkxparse-packfile-layout PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)

foreach(layout IN LISTS HKXPARSE_PACKFILE_LAYOUT_FILES)
  target_sources(hkxparse-packfile-layout PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/${layout}.cpp
	${CMAKE_CURRENT_BINARY_DIR}/include/${layout}.h
  )

  add_custom_command(
  	  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${layout}.cpp
	         ${CMAKE_CURRENT_BINARY_DIR}/include/${layout}.h
	  COMMAND ${RUBY_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/make_layout.rb
	          ${CMAKE_CURRENT_SOURCE_DIR}/${layout}.json
			  ${CMAKE_CURRENT_BINARY_DIR}/include/${layout}.h
			  ${CMAKE_CURRENT_BINARY_DIR}/${layout}.cpp
	  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${layout}.json
	  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/make_layout.rb
	  VERBATIM
  )
endforeach()

