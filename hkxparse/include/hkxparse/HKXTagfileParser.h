#ifndef HKXPARSE_HKX_TAGFILE_PARSER_H
#define HKXPARSE_HKX_TAGFILE_PARSER_H

#include <hkxparse/HKXTypes.h>
#include <hkxparse/LayoutRules.h>
#include <hkxparse/Deserializer.h>
#include <hkxparse/TagfileTypes.h>
#include <array>

namespace hkxparse {
	class HKXMapping;

	class HKXTagfileParser {
	public:
		explicit HKXTagfileParser(HKXMapping &mapping);
		~HKXTagfileParser();

		HKXTagfileParser(const HKXTagfileParser &other) = delete;
		HKXTagfileParser &operator =(const HKXTagfileParser &other) = delete;

		HKXStructRef parse();

	private:
		using MemberBitmap = std::array<uint8_t, 16>;

		const std::string &readString();
		TagfileTypeInfo readTypeInfo();

		void parseStruct(HKXStruct &st, int32_t classIndex);
		void parseStructMembers(HKXStruct &st, const MemberBitmap &bitmap, size_t &firstIndex, const TagfileTypeInfo &typeInfo);
		void parseField(HKXStruct &st, const TagfileMemberInfo &member);
		void parseFieldValue(unsigned int type, const std::string &className, HKXVariant &value, int32_t arrayPrefix);
		size_t countMembers(int32_t classIndex);
		void parseStructArray(const TagfileMemberInfo &member, HKXArray &ary);
		int32_t parseArrayPrefix(unsigned int type);
		const TagfileMemberInfo *structMemberByIndex(const TagfileTypeInfo &typeInfo, size_t index, size_t *firstIndex = nullptr);
		void parseArray(const TagfileMemberInfo &member, HKXArray &ary);

		HKXMapping &m_mapping;
		LayoutRules m_rules;
		Deserializer m_stream;
		std::vector<std::string> m_stringPool;
		std::string m_havokVersion;
		std::vector<TagfileTypeInfo> m_types;
		std::unordered_map<std::string, int32_t> m_typeLookup;
		int32_t m_nextAllocatedObject;
		std::unordered_map<int32_t, std::shared_ptr<HKXStruct>> m_objects;
	};
}

#endif
