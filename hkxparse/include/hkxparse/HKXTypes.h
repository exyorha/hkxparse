#ifndef HKXPARSE_HKX_TYPES_H
#define HKXPARSE_HKX_TYPES_H

#include <memory>
#include <variant>
#include <string>
#include <unordered_map>

namespace hkxparse {
	class Deserializer;

	struct HKXStruct;

	struct HKXVector4 {
		float x;
		float y;
		float z;
		float w;
	};

	struct HKXQuaternion {
		HKXVector4 vec;
	};

	struct HKXMatrix3 {
		HKXVector4 v[3];
	};

	struct HKXQsTransform {
		HKXVector4 translation;
		HKXQuaternion rotation;
		HKXVector4 scale;
	};

	struct HKXMatrix4 {
		HKXVector4 v[4];
	};

	struct HKXArray;

	using HKXStructRef = std::shared_ptr<HKXStruct>;
	using HKXVariant = std::variant< // Variant
		std::monostate,
		uint64_t, // Bool, Char, Int8, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64, Enum, Flags, ULong
		float, // Real, Half
		HKXVector4, // Vector4
		HKXQuaternion, // Quaternion
		HKXMatrix3, // Matrix3, Rotation
		HKXQsTransform, // QsTransform
		HKXMatrix4, // Matrix4, Transform
		HKXStructRef, // Pointer (some types of)
		HKXArray, // Array, InPlaceArray, SimpleArray, HomogeneousArray, RelArray
		std::string, // CString, StringPtr
		HKXStruct // Struct
	>;

	struct HKXStruct {
		std::vector<const char *> classNames;
		std::unordered_map<const char *, HKXVariant> fields;
	};

	struct HKXArray {
		std::vector<HKXVariant> values;
	};

	Deserializer &operator >>(Deserializer &stream, HKXVector4 &val);
	Deserializer &operator >>(Deserializer &stream, HKXQuaternion &val);
	Deserializer &operator >>(Deserializer &stream, HKXMatrix3 &val);
	Deserializer &operator >>(Deserializer &stream, HKXQsTransform &val);
	Deserializer &operator >>(Deserializer &stream, HKXMatrix4 &val);

}

#endif
