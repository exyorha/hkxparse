#ifndef HKXPARSE_HAVOK_REFLECTION_TYPES_H
#define HKXPARSE_HAVOK_REFLECTION_TYPES_H

#include <stdint.h>

namespace hkxparse {
	struct HavokClass;

	enum class HavokType : uint8_t {
		Void,
		Bool,
		Char,
		Int8,
		UInt8,
		Int16,
		UInt16,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Real,
		Vector4,
		Quaternion,
		Matrix3,
		Rotation,
		QsTransform,
		Matrix4,
		Transform,
		Zero,
		Pointer,
		FunctionPointer,
		Array,
		InPlaceArray,
		Enum,
		Struct,
		SimpleArray,
		HomogeneousArray,
		Variant,
		CString,
		ULong,
		Flags,
		Half,
		StringPtr,
		RelArray
	};

	struct HavokClassEnumItem {
		uint32_t value;
		const char *name;
	};

	struct HavokClassEnum {
		const char *name;
		const HavokClassEnumItem *items;
		size_t numItems;
		uint32_t flags;
	};

	struct HavokClassMember {
		const char *name;
		const HavokClass *typeClass;
		HavokType type;
		HavokType subtype;
		uint16_t arraySize;
		uint16_t flags;
		uint16_t offset;
	};

	struct HavokClass {
		const char *name;
		const HavokClass *parent;
		size_t objectSize;

		const HavokClassEnum *declaredEnums;
		size_t numDeclaredEnums;

		const HavokClassMember *declaredMembers;
		size_t numDeclaredMembers;

		uint32_t flags;
		uint32_t describedVersion;
	};

	struct HavokTypeInfo {
		const char *name;
		uint64_t vtable;
	};
}

#endif
