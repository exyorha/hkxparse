#ifndef HKXPARSE_TAGFILE_TYPES_H
#define HKXPARSE_TAGFILE_TYPES_H

#include <stdint.h>

namespace hkxparse {
	enum : uint32_t {
		TagfileMagic0 = 0xCAB00D1E,
		TagfileMagic1 = 0xD011FACE
	};

	enum : int32_t {
		TagNone = 0,
		TagFileInfo = 1,
		TagMetadata = 2,
		TagObject = 3,
		TagObjectRemember = 4,
		TagObjectBackref = 5,
		TagObjectNull = 6,
		TagFileEnd = 7
	};

	enum : int32_t {
		TagTypeVoid = 0,
		TagTypeByte,
		TagTypeInt,
		TagTypeReal,
		TagTypeVec4,
		TagTypeVec8,
		TagTypeVec12,
		TagTypeVec16,
		TagTypeObject,
		TagTypeStruct,
		TagTypeCString,

		TagBasicTypeMask = 15,

		TagArrayFlag = 16,
		TagTupleFlag = 32
	};

	struct TagfileHeader {
		uint32_t magic0;
		uint32_t magic1;
	};

	struct TagfileMemberInfo {
		std::string name;
		int32_t type;
		int32_t tupleSize; // Tuples only
		std::string className; // Object and Struct only
	};

	struct TagfileTypeInfo {
		std::string name;
		int32_t unk3;
		int32_t parentTypeIndex;
		std::vector<TagfileMemberInfo> members;
	};
}

#endif
