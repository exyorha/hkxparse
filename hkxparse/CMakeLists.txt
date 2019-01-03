add_library(hkxparse STATIC
	include/hkxparse/Deserializer.h
	include/hkxparse/HKXFile.h
	include/hkxparse/HKXMapping.h
	include/hkxparse/HKXPackfileLoader.h
	include/hkxparse/HKXTagfileParser.h
	include/hkxparse/HKXTypes.h
	include/hkxparse/LayoutRules.h
	include/hkxparse/PackfileTypes.h
	include/hkxparse/PrettyPrinter.h
	include/hkxparse/TagfileTypes.h
	hkxparse/Deserializer.cpp
	hkxparse/HKXFile.cpp
	hkxparse/HKXMapping.cpp
	hkxparse/HKXPackfileLoader.cpp
	hkxparse/HKXTagfileParser.cpp
	hkxparse/HKXTypes.cpp
	hkxparse/PrettyPrinter.cpp
)

target_include_directories(hkxparse PUBLIC include)
target_link_libraries(hkxparse PRIVATE hkxparse-packfile-layout halffloat)

