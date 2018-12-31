#include <hkxparse/HKXPackfileLoader.h>
#include <hkxparse/PackfileTypes.h>
#include <hkxparse/HKXMapping.h>
#include <hkxparse/Deserializer.h>
#include <hkxparse/HavokPackfileLayouts.h>
#include <hkxparse/HavokReflectionTypes.h>

#include <stdexcept>
#include <sstream>

namespace hkxparse {

	HKXPackfileLoader::HKXPackfileLoader(HKXMapping &mapping) : m_mapping(mapping) {
		const auto &header = *reinterpret_cast<PackfileHeader *>(m_mapping.data());

		if (!header.layoutRules.littleEndian) {
			throw std::runtime_error("big endian files are not supported yet");
		}

		if (header.fileVersion != 8) {
			throw std::runtime_error("unsupported packfile version");
		}

		auto sectionHeaders = reinterpret_cast<const PackfileSectionHeader *>(&header + 1);

		for (int32_t sectionIndex = 0; sectionIndex < header.numSections; sectionIndex++) {
			const auto &section = sectionHeaders[sectionIndex];

			auto data = m_mapping.data() + section.absoluteDataStart;
			auto dataSize = section.localFixupsOffset;

			auto localFixups = data + section.localFixupsOffset;
			auto localFixupsSize = section.globalFixupsOffset - section.localFixupsOffset;

			auto globalFixups = data + section.globalFixupsOffset;
			auto globalFixupsSize = section.virtualFixupsOffset - section.globalFixupsOffset;

			auto virtualFixups = data + section.virtualFixupsOffset;
			auto virtualFixupsSize = section.exportsOffset - section.virtualFixupsOffset;

			auto exports = data + section.exportsOffset;
			auto exportsSize = section.importsOffset - section.exportsOffset;

			auto imports = data + section.importsOffset;
			auto importsSize = section.endOffset - section.importsOffset;

			if (localFixupsSize != 0) {
				printf("%s: %zu bytes of local fixups\n", section.sectionTag, localFixupsSize);

				Deserializer stream(header.layoutRules, localFixups, localFixupsSize);

				uint32_t offset;
				uint32_t target;
				while (true) {
					stream >> offset;

					if (offset == 0xFFFFFFFF)
						break;

					stream >> target;

					fixup(data, dataSize, header.layoutRules, offset, section.absoluteDataStart + target);
				}
			}

			if (globalFixupsSize != 0) {
				printf("%s: %zu bytes of global fixups\n", section.sectionTag, globalFixupsSize);

				Deserializer stream(header.layoutRules, globalFixups, globalFixupsSize);

				uint32_t offset;
				uint32_t section;
				uint32_t target;
				while (true) {
					stream >> offset;

					if (offset == 0xFFFFFFFF)
						break;

					stream >> section >> target;

					if (section >= static_cast<uint32_t>(header.numSections)) {
						throw std::runtime_error("section index is out of range in global fixup");
					}

					fixup(data, dataSize, header.layoutRules, offset, sectionHeaders[section].absoluteDataStart + target);
				}
			}

			if (virtualFixupsSize != 0) {
				printf("%s: %zu bytes of virtual fixups\n", section.sectionTag, virtualFixupsSize);

				Deserializer stream(header.layoutRules, virtualFixups, virtualFixupsSize);
			}

			if (exportsSize != 0) {
				printf("%s: %zu bytes of exports\n", section.sectionTag, exportsSize);

				Deserializer stream(header.layoutRules, exports, exportsSize);
			}

			if (importsSize != 0) {
				printf("%s: %zu bytes of imports\n", section.sectionTag, importsSize);

				Deserializer stream(header.layoutRules, imports, importsSize);
			}
		}

		m_layout = findLayout(header.contentsVersion, reinterpret_cast<const unsigned char *>(&header.layoutRules));
		if (!m_layout) {
			std::stringstream error;
			error << "No packfile layout for version " << header.contentsVersion << ", layout rules " <<
				static_cast<unsigned int>(header.layoutRules.bytesInPointer) << "-" <<
				static_cast<unsigned int>(header.layoutRules.littleEndian) << "-" <<
				static_cast<unsigned int>(header.layoutRules.reusePaddingOptimization) << "-" <<
				static_cast<unsigned int>(header.layoutRules.emptyBaseClassOptimization);
			throw std::runtime_error(error.str());
		}
	}	

	HKXPackfileLoader::~HKXPackfileLoader() {

	}

	HKXStructRef HKXPackfileLoader::loadRoot() {
		const auto &header = *reinterpret_cast<PackfileHeader *>(m_mapping.data());
		auto sectionHeaders = reinterpret_cast<const PackfileSectionHeader *>(&header + 1);

		auto className = reinterpret_cast<char *>(m_mapping.data()) + sectionHeaders[header.contentsClassNameSectionIndex].absoluteDataStart + header.contentsClassNameSectionOffset;

		size_t dataOffset = sectionHeaders[header.contentsSectionIndex].absoluteDataStart + header.contentsSectionOffset;

		return parseStructureAtPointer(header.layoutRules, dataOffset, className);
	}

	template<typename ClassArgType>
	HKXStructRef HKXPackfileLoader::parseStructureAtPointer(const LayoutRules &layoutRules, uint64_t pointer, ClassArgType classArg) {
		auto it = m_structures.find(pointer);
		if (it == m_structures.end()) {
			auto ptr = std::make_shared<HKXStruct>();
			m_structures.emplace(pointer, ptr);

			Deserializer stream(layoutRules, m_mapping.data() + pointer, m_mapping.size() - pointer);

			parseStructure(classArg, stream, *ptr);

			return ptr;
		}
		else {
			return it->second;
		}
	}

	void HKXPackfileLoader::fixup(unsigned char *data, size_t dataSize, const LayoutRules &layoutRules, size_t offset, size_t target) {
		if (layoutRules.bytesInPointer == 4) {
			*reinterpret_cast<uint32_t *>(data + offset) = static_cast<uint32_t>(target);
		}
		else if (layoutRules.bytesInPointer == 8) {
			*reinterpret_cast<uint64_t *>(data + offset) = static_cast<uint64_t>(target);
		}
		else {
			throw std::runtime_error("unsupported pointer size");
		}
	}
	
	void HKXPackfileLoader::parseStructure(const char *className, Deserializer &stream, HKXStruct &target) {
		auto begin = m_layout->classes;
		auto end = m_layout->classes + m_layout->classCount;
		auto classIt = std::lower_bound(begin, end, className, [](const HavokClass *hClass, const char *hClassName) {
			return strcmp(hClass->name, hClassName) < 0;
		});

		if (classIt == end || strcmp((*classIt)->name, className) != 0) {
			std::stringstream error;
			error << "No definition for class " << className;
			throw std::runtime_error(error.str());
		}

		return parseStructure(*classIt, stream, target);
	}		
	
	void HKXPackfileLoader::parseStructure(const HavokClass *classReflection, Deserializer &stream, HKXStruct &target) {
		if (classReflection->parent) {
			parseStructure(classReflection->parent, stream, target);
		}

		target.classNames.emplace_back(classReflection->name);

		stream.mark();

		for (size_t memberIndex = 0; memberIndex < classReflection->numDeclaredMembers; memberIndex++) {
			auto &member = classReflection->declaredMembers[memberIndex];

			printf("member: %s, type: %u, subtype: %u, array size: %u, flags: %u, offset: %u\n", member.name, member.type, member.subtype, member.arraySize, member.flags, member.offset);

			auto &it = target.fields.emplace(member.name, std::monostate());
			deserializeField(stream, member, it.first->second);
		}

		stream.seekFromMark(classReflection->objectSize);
	}

	void HKXPackfileLoader::deserializeField(Deserializer &stream, const HavokClassMember &member, HKXVariant &value) {
		stream.seekFromMark(member.offset);

		deserializeField(stream, member, member.type, value);
	}

	void HKXPackfileLoader::deserializeField(Deserializer &stream, const HavokClassMember &member, HavokType type, HKXVariant &value) {
		switch (type) {
		case HavokType::Void:
		case HavokType::Zero:
			value = std::monostate();
			break;

		case HavokType::Bool:
		{
			bool val;
			stream.readBool(val);
			value = static_cast<uint64_t>(val);
			break;
		}

		case HavokType::Char:
		{
			char val;
			stream >> val;
			value = static_cast<uint64_t>(static_cast<int64_t>(val));
			break;			
		}

		case HavokType::Int8:
		{
			int8_t val;
			stream >> val;
			value = static_cast<uint64_t>(static_cast<int64_t>(val));
			break;
		}

		case HavokType::UInt8:
		{
			uint8_t val;
			stream >> val;
			value = static_cast<uint64_t>(val);
			break;
		}

		case HavokType::Int16:
		{
			int16_t val;
			stream >> val;
			value = static_cast<uint64_t>(static_cast<int64_t>(val));
			break;
		}

		case HavokType::UInt16:
		{
			uint16_t val;
			stream >> val;
			value = static_cast<uint64_t>(val);
			break;
		}

		case HavokType::Int32:
		{
			int32_t val;
			stream >> val;
			value = static_cast<uint64_t>(static_cast<int64_t>(val));
			break;
		}

		case HavokType::UInt32:
		{
			uint32_t val;
			stream >> val;
			value = static_cast<uint64_t>(val);
			break;
		}

		case HavokType::Int64:
		{
			int64_t val;
			stream >> val;
			value = static_cast<uint64_t>(static_cast<int64_t>(val));
			break;
		}

		case HavokType::UInt64:
		{
			uint64_t val;
			stream >> val;
			value = static_cast<uint64_t>(val);
			break;
		}

		case HavokType::Real:
		{
			float val;
			stream >> val;
			value = val;
			break;
		}

		case HavokType::Vector4:
		{
			HKXVector4 val;
			stream >> val;
			value = val;
			break;
		}

		case HavokType::Quaternion:
		{
			HKXQuaternion val;
			stream >> val;
			value = val;
			break;
		}

		case HavokType::Matrix3:
		case HavokType::Rotation:
		{
			HKXMatrix3 val;
			stream >> val;
			value = val;
			break;
		}

		case HavokType::QsTransform:
		{
			HKXQsTransform val;
			stream >> val;
			value = val;
			break;
		}

		case HavokType::Matrix4:
		case HavokType::Transform:
		{
			HKXMatrix4 val;
			stream >> val;
			value = val;
			break;
		}

		case HavokType::Pointer:
		{
			uint64_t ptr;
			stream.readPointer(ptr);

			if (member.subtype == HavokType::Struct) {
				value = parseStructureAtPointer(stream.layoutRules(), ptr, member.typeClass);
			}
			else {
				__debugbreak();
			}
			break;
		}

		case HavokType::FunctionPointer:
			__debugbreak();
			break;

		case HavokType::Array:
		{
			value = HKXArray();
			auto &ary = std::get<HKXArray>(value);

			uint64_t ptr;
			uint32_t len;
			stream.readPointer(ptr);
			stream >> len;

			Deserializer arrayStream(stream.layoutRules(), m_mapping.data() + ptr, static_cast<size_t>(m_mapping.size() - ptr));
			ary.values.resize(len);

			for (size_t index = 0; index < len; index++) {
				deserializeField(arrayStream, member, member.subtype, ary.values[index]);
			}

			break;
		}

		case HavokType::InPlaceArray:
			__debugbreak();
			break;

		case HavokType::Enum:
			__debugbreak();
			break;

		case HavokType::Struct:
			value = HKXStruct();

			parseStructure(member.typeClass, stream, std::get<HKXStruct>(value));

			break;

		case HavokType::SimpleArray:
			__debugbreak();
			break;

		case HavokType::HomogeneousArray:
			__debugbreak();
			break;

		case HavokType::Variant:
			__debugbreak();
			break;

		case HavokType::CString:
			__debugbreak();
			break;

		case HavokType::ULong:
		{
			uint64_t val;
			stream.readPointer(val);
			value = val;
			break;
		}

		case HavokType::Flags:
			__debugbreak();
			break;

		case HavokType::Half:
			__debugbreak();
			break;

		case HavokType::StringPtr:
		{
			uint64_t val;

			stream.readPointer(val);

			if (val == 0) {
				value = std::string();
			}
			else {
				value = reinterpret_cast<char *>(m_mapping.data()) + val;
			}

			break;
		}

		case HavokType::RelArray:
			__debugbreak();
			break;
		}
	}
}