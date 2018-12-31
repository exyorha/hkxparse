add_library(hkxparse STATIC
	include/hkxparse/Deserializer.h
	include/hkxparse/HKXFile.h
	include/hkxparse/HKXMapping.h
	include/hkxparse/HKXPackfileLoader.h
	include/hkxparse/HKXTypes.h
	include/hkxparse/LayoutRules.h
	include/hkxparse/PackfileTypes.h
	hkxparse/Deserializer.cpp
	hkxparse/HKXFile.cpp
	hkxparse/HKXMapping.cpp
	hkxparse/HKXPackfileLoader.cpp
	hkxparse/HKXTypes.cpp
)

target_include_directories(hkxparse PUBLIC include)
target_link_libraries(hkxparse PRIVATE hkxparse-packfile-layout)

