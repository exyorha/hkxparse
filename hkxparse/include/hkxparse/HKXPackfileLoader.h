#ifndef HKXPARSE_HKX_PACKFILE_LOADER_H
#define HKXPARSE_HKX_PACKFILE_LOADER_H

#include <hkxparse/HKXTypes.h>
#include <hkxparse/HavokReflectionTypes.h>

namespace hkxparse {
	class HKXMapping;
	struct LayoutRules;
	struct HavokPackfileLayout;
	class Deserializer;

	class HKXPackfileLoader {
	public:
		explicit HKXPackfileLoader(HKXMapping &mapping);
		~HKXPackfileLoader();

		HKXPackfileLoader(const HKXPackfileLoader &other) = delete;
		HKXPackfileLoader &operator =(const HKXPackfileLoader &other) = delete;

		HKXStructRef loadRoot();

	private:
		void fixup(unsigned char *data, size_t dataSize, const LayoutRules &layoutRules, size_t offset, size_t target);
		void parseStructure(const char *className, Deserializer &stream, HKXStruct &target);
		void parseStructure(const HavokClass *classReflection, Deserializer &stream, HKXStruct &target);
		void deserializeField(Deserializer &stream, const HavokClassMember &member, HKXVariant &value); 
		void deserializeField(Deserializer &stream, const HavokClassMember &member, HavokType type, HKXVariant &value);

		template<typename ClassArgType>
		HKXStructRef parseStructureAtPointer(const LayoutRules &layoutRules, uint64_t pointer, ClassArgType classArg);

		HKXMapping &m_mapping;
		const HavokPackfileLayout *m_layout;
		std::unordered_map<uint64_t, HKXStructRef> m_structures;
	};
}

#endif
