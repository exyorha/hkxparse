add_library(hkxparse STATIC
	include/hkxparse/Deserializer.h
	include/hkxparse/HKXFile.h
	include/hkxparse/HKXMapping.h
	include/hkxparse/LayoutRules.h
	include/hkxparse/PackfileTypes.h
	hkxparse/Deserializer.cpp
	hkxparse/HKXFile.cpp
	hkxparse/HKXMapping.cpp
)

target_include_directories(hkxparse PUBLIC include)
