#include <hkxparse/HKXTagfileParser.h>
#include <hkxparse/HKXMapping.h>
#include <hkxparse/TagfileTypes.h>

#include <sstream>
#include <stdexcept>
#include <array>

namespace hkxparse {
	HKXTagfileParser::HKXTagfileParser(HKXMapping &mapping) : m_mapping(mapping), m_nextAllocatedObject(1) {
		m_rules.bytesInPointer = 0;
		
		const auto &header = *reinterpret_cast<TagfileHeader *>(m_mapping.data());
		if (header.magic0 == TagfileMagic0 && header.magic1 == TagfileMagic1) {
			m_rules.littleEndian = 1;
		}
		else if ((header.magic0 == _byteswap_ulong(TagfileMagic0) && header.magic1 == _byteswap_ulong(TagfileMagic1))) {
			m_rules.littleEndian = 0;
		}
		else {
			throw std::runtime_error("bad tagfile magic");
		}

		m_rules.reusePaddingOptimization = 0;
		m_rules.emptyBaseClassOptimization = 0;

		m_stream = Deserializer(m_rules, m_mapping.data() + sizeof(TagfileHeader), m_mapping.size() - sizeof(TagfileHeader));

		TagfileTypeInfo voidType;
		voidType.name = "BuiltinVoidType";
		voidType.parentTypeIndex = 0;
		voidType.unk3 = 0;

		TagfileMemberInfo voidTypeMember;
		voidTypeMember.name = "void";
		voidTypeMember.type = TagTypeVoid;
		voidType.members.emplace_back(std::move(voidTypeMember));
		m_types.emplace_back(std::move(voidType));
	}
	
	HKXTagfileParser::~HKXTagfileParser() {

	}

	HKXStructRef HKXTagfileParser::parse() {
		while (true) {
			auto type = m_stream.readVarInt();

			switch (type) {
			case TagFileInfo:
			{
				auto version = m_stream.readVarInt();

				if (version == 3 || version == 4) {
					m_stringPool.clear();
					m_stringPool.emplace_back();
					m_stringPool.emplace_back();
				}

				switch (version) {
				case 3:
					break;

				case 4:
					m_havokVersion = readString();
					break;

				default:
				{
					std::stringstream error;
					error << "Unsupported version " << version;
					throw std::runtime_error(error.str());
				}
				}

				break;
			}

			case TagMetadata:
			{
				const auto &result = m_types.emplace_back(readTypeInfo());
				m_typeLookup.emplace(result.name, m_types.size() - 1);
				break;
			}

			case TagObjectRemember:
			{
				auto it = m_objects.find(m_nextAllocatedObject);
				if (it == m_objects.end()) {
					printf("!!!!!!!!!! Creating new object %d\n", m_nextAllocatedObject);
					auto obj = std::make_shared<HKXStruct>();
					m_objects.emplace(m_nextAllocatedObject, obj);
					m_nextAllocatedObject++;

					parseStruct(*obj, 0);
				}
				else {
					printf("!!!!!!!!!! Reusing existing object %d\n", m_nextAllocatedObject);
					m_nextAllocatedObject++;

					parseStruct(*it->second, 0);
				}

				break;
			}

			case TagFileEnd:
				goto breakOuter;

			default:
			{
				std::stringstream error;
				error << "Unsupported tag type " << type;
				throw std::runtime_error(error.str());
			}
			}
		}
	breakOuter:

		for (const auto &pair : m_objects) {
			if (pair.second->classNames.empty())
				throw std::logic_error("unresolved forward references still exist after parsing");
		}

		auto root = m_objects.find(1);
		if (root == m_objects.end()) {
			return {};
		}
		else {
			return root->second;
		}
	}

	size_t HKXTagfileParser::countMembers(int32_t classIndex) {
		size_t memberCount = 0;
		for (int32_t typeIndex = classIndex; typeIndex != 0; typeIndex = m_types[typeIndex].parentTypeIndex) {
			memberCount += m_types[typeIndex].members.size();
		}

		return memberCount;
	}

	void HKXTagfileParser::parseStruct(HKXStruct &st, int32_t classIndex) {
		// TagObjectRemember

		if (classIndex == 0) {
			classIndex = m_stream.readVarInt();
		}

		const auto &typeInfo = m_types[classIndex];
		
		auto memberCount = countMembers(classIndex);

		printf("Reading %s, total members: %zu\n", typeInfo.name.c_str(), memberCount);

		MemberBitmap memberBitmap;

		if (memberCount > memberBitmap.size() * 8)
			throw std::logic_error("too many members");

		m_stream.readBytes(memberBitmap.data(), (memberCount + 7) / 8);

		printf("Bitmap: ");
		for (size_t index = 0; index < (memberCount + 7) / 8; index++) {
			printf("%02X ", memberBitmap[index]);
		}
		printf("\n");

		size_t firstIndex = 0;

		parseStructMembers(st, memberBitmap, firstIndex, typeInfo);
	}

	void HKXTagfileParser::parseStructMembers(HKXStruct &st, const MemberBitmap &bitmap, size_t &firstIndex, const TagfileTypeInfo &typeInfo) {
		if (typeInfo.parentTypeIndex != 0) {
			parseStructMembers(st, bitmap, firstIndex, m_types[typeInfo.parentTypeIndex]);
		}

		printf("trying %s, first member: %zu, total members: %zu\n", typeInfo.name.c_str(), firstIndex, typeInfo.members.size());

		st.classNames.emplace_back(typeInfo.name);

		for (const auto &field : typeInfo.members) {
			size_t fieldIndex = firstIndex;

			printf("index %zu: %s\n", fieldIndex, field.name.c_str());
			if (bitmap[fieldIndex / 8] & (1 << (fieldIndex % 8))) {
				printf("Field %s is present\n", field.name.c_str());

				parseField(st, field);
			}

			firstIndex++;
		}

		printf("finish with firstIndex %zu\n", firstIndex);

		//firstIndex += typeInfo.members.size();
	}

	const std::string &HKXTagfileParser::readString() {
		auto length = m_stream.readVarInt();

		if (length <= 0) {
			return m_stringPool[-length];
		}
		else {
			std::string newString;
			newString.resize(length);
			m_stream.readBytes(reinterpret_cast<unsigned char *>(newString.data()), newString.size());
			
			return m_stringPool.emplace_back(std::move(newString));
		}
	}

	TagfileTypeInfo HKXTagfileParser::readTypeInfo() {
		TagfileTypeInfo info;

		info.name = readString();
		info.unk3 = m_stream.readVarInt();
		info.parentTypeIndex = m_stream.readVarInt();
		
		auto memberCount = m_stream.readVarInt();
		info.members.resize(memberCount);

		for (auto &member : info.members) {
			member.name = readString();
			member.type = m_stream.readVarInt();

			if (member.type & TagTupleFlag) {
				member.tupleSize = m_stream.readVarInt();
			}

			if ((member.type & TagBasicTypeMask) == TagTypeObject || (member.type & TagBasicTypeMask) == TagTypeStruct) {
				member.className = readString();
			}
		}

		return info;
	}

	void HKXTagfileParser::parseField(HKXStruct &st, const TagfileMemberInfo &member) {
		if (member.type & ~(TagArrayFlag | TagTupleFlag | TagBasicTypeMask)) {
			std::stringstream error;
			error << "Unsupported flags in field type: " << member.type;
			throw std::runtime_error(error.str());
		}

		if (member.type == (TagTupleFlag | TagTypeByte)) {
			// Special case: byte tuple

			std::vector<unsigned char> bytes(member.tupleSize);
			m_stream.readBytes(bytes.data(), bytes.size());
			st.fields.emplace(member.name, std::move(bytes));
		}
		else if (member.type == (TagArrayFlag | TagTypeByte)) {
			// Special case: byte array

			std::vector<unsigned char> bytes(m_stream.readVarInt());
			m_stream.readBytes(bytes.data(), bytes.size());
			st.fields.emplace(member.name, std::move(bytes));
		} else if (member.type & (TagTupleFlag | TagArrayFlag)) {
			if ((member.type & (TagArrayFlag | TagTupleFlag)) == (TagArrayFlag | TagTupleFlag)) {
				throw std::logic_error("member is both an array and a tuple");
			}

			auto result = st.fields.emplace(member.name, HKXArray());
			auto &ary = std::get<HKXArray>(result.first->second);

			if (member.type & TagTupleFlag) {
				ary.values.resize(member.tupleSize);
			}
			else {
				ary.values.resize(m_stream.readVarInt());
			}
			
			parseArray(member, ary);

		}
		else {
			auto result = st.fields.emplace(member.name, std::monostate());			
			parseFieldValue(member.type & TagBasicTypeMask, member.className, result.first->second, -1);
		}
	}

	void HKXTagfileParser::parseArray(const TagfileMemberInfo &member, HKXArray &ary) {
		auto prefix = parseArrayPrefix(member.type);

		if ((member.type & TagBasicTypeMask) == TagTypeStruct) {
			parseStructArray(member, ary);
		}
		else {
			for (auto &value : ary.values) {
				parseFieldValue(member.type & TagBasicTypeMask, member.className, value, prefix);
			}
		}
	}

	int32_t HKXTagfileParser::parseArrayPrefix(unsigned int type) {
		if ((type & TagBasicTypeMask) == TagTypeInt) {
			printf("int prefix\n");
			auto arrayItemWidth = m_stream.readVarInt();
			return arrayItemWidth;
		} else if ((type & TagBasicTypeMask) == TagTypeVec4) {
			printf("vec4 prefix\n");
			auto numberOfMembers = m_stream.readVarInt();
			return numberOfMembers;
		}
		else {
			return -1;
		}
	}

	void HKXTagfileParser::parseFieldValue(unsigned int type, const std::string &className, HKXVariant &value, int32_t arrayPrefix) {
		printf("type: %u, className: %s, array prefix: %d\n", type, className.c_str(), arrayPrefix);

		switch (type) {
		case TagTypeByte:
			value = static_cast<uint64_t>(m_stream.readByte());
			break;

		case TagTypeInt:
			value = static_cast<uint64_t>(static_cast<int64_t>(m_stream.readVarInt()));
			break;

		case TagTypeReal:
		{
			float val;
			m_stream >> val;
			value = val;
			break;
		}

		case TagTypeVec4:
		{
			if (arrayPrefix < 0) {
				arrayPrefix = 4;
			}
			else if (arrayPrefix < 1 || arrayPrefix > 4) {
				throw std::logic_error("unsupported vec4 length");
			}

			value = HKXVector4();
			auto &base = std::get<HKXVector4>(value);
			base.x = 0.0f;
			base.y = 0.0f;
			base.z = 0.0f;
			base.w = 0.0f;

			auto *ptr = &base.x;
			for (int32_t index = 0; index < arrayPrefix; index++) {
				m_stream >> *ptr;

				ptr++;
			}

			break;
		}

		case TagTypeVec12:
			value = HKXMatrix3();
			m_stream >> std::get<HKXMatrix3>(value);
			break;

		case TagTypeVec16:
			value = HKXMatrix4();
			m_stream >> std::get<HKXMatrix4>(value);
			break;

		case TagTypeObject:
		{
			auto objectIndex = m_stream.readVarInt();
			if (objectIndex == 0) {
				printf("nullref\n");

				value = HKXStructRef();
			}
			else {
				auto it = m_objects.find(objectIndex);
				if (it != m_objects.end()) {
					printf("backref to %d\n", objectIndex);
					value = it->second;
				}
				else {
					printf("fwdref to %d\n", objectIndex);
					auto obj = std::make_shared<HKXStruct>();
					m_objects.emplace(objectIndex, obj);

					value = obj;
				}
			}

			break;
		}

		case TagTypeStruct:
		{
			int32_t classIndex = 0;
			if (!className.empty()) {
				auto it = m_typeLookup.find(className);
				if (it == m_typeLookup.end()) {
					std::stringstream stream;
					stream << "Class " << className << " not found";
					throw std::logic_error(stream.str());
				}

				classIndex = it->second;
			}
			value = HKXStruct();
			parseStruct(std::get<HKXStruct>(value), classIndex);
			break;
		}

		case TagTypeCString:
			value = readString();
			break;

		default:
		{
			std::stringstream error;
			error << "Unsupported field type: " << type;
			throw std::runtime_error(error.str());
		}
		}
	}

	const TagfileMemberInfo *HKXTagfileParser::structMemberByIndex(const TagfileTypeInfo &typeInfo, size_t index, size_t *firstIndex) {
		size_t firstIndexStorage = 0;
		if (!firstIndex) {
			firstIndex = &firstIndexStorage;
		}

		if (typeInfo.parentTypeIndex != 0) {
			auto result = structMemberByIndex(m_types[typeInfo.parentTypeIndex], index, firstIndex);
			if (result)
				return result;
		}

		if (index - *firstIndex < typeInfo.members.size()) {
			return &typeInfo.members[index - *firstIndex];
		}
		else {
			*firstIndex += typeInfo.members.size();

			return nullptr;
		}
	}

	void HKXTagfileParser::parseStructArray(const TagfileMemberInfo &member, HKXArray &ary) {
		int32_t classIndex = 0;

		if (member.type == (TagArrayFlag | TagTypeStruct) && member.className.empty()) {
			classIndex = m_stream.readVarInt();
		}
		else if (!member.className.empty()) {
			auto it = m_typeLookup.find(member.className);
			if (it == m_typeLookup.end()) {
				throw std::logic_error("unable to find class");
			}
			classIndex = it->second;
		}

		if (classIndex == 0) {
			throw std::logic_error("class index unknown in struct array");
		}

		const auto &typeInfo = m_types[classIndex];

		auto memberCount = countMembers(classIndex);

		MemberBitmap memberBitmap;

		if (memberCount > memberBitmap.size() * 8)
			throw std::logic_error("too many members");

		m_stream.readBytes(memberBitmap.data(), (memberCount + 7) / 8);

		printf("Bitmap: ");
		for (size_t index = 0; index < (memberCount + 7) / 8; index++) {
			printf("%02X ", memberBitmap[index]);
		}
		printf("\n");

		for (auto &member : ary.values) {
			member = HKXStruct();
			auto &st = std::get<HKXStruct>(member);

			for (auto typeIndex = classIndex; typeIndex != 0; typeIndex = m_types[typeIndex].parentTypeIndex) {
				st.classNames.emplace_back(m_types[typeIndex].name);
			}
		}

		for (int32_t index = 0; index < static_cast<int32_t>(memberCount); index++) {
			if (memberBitmap[index / 8] & (1 << (index % 8))) {
				auto memberType = structMemberByIndex(typeInfo, index);

				HKXArray view;
				view.values.resize(ary.values.size());

				printf("parsing array for %s\n", memberType->name.c_str());

				parseArray(*memberType, view);

				for (size_t index = 0, size = ary.values.size(); index < size; index++) {
					std::get<HKXStruct>(ary.values[index]).fields.emplace(memberType->name, std::move(view.values[index]));
				}
			}
		}

		printf("FINISHED WITH STRUCT ARRAY\n");
	}
}
