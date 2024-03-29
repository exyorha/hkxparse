#include <hkxparse/HKXPackfileLoader.h>
#include <hkxparse/PackfileTypes.h>
#include <hkxparse/HKXMapping.h>
#include <hkxparse/Deserializer.h>
#include <hkxparse/HavokPackfileLayouts.h>
#include <hkxparse/HavokReflectionTypes.h>

#include <stdexcept>
#include <sstream>

#include <half.h>

namespace hkxparse {

	HKXPackfileLoader::HKXPackfileLoader(HKXMapping &mapping) : m_mapping(mapping) {
		const auto &header = *reinterpret_cast<PackfileHeader *>(m_mapping.data());

		if (!header.layoutRules.littleEndian) {
			throw std::runtime_error("big endian files are not supported yet");
		}

		if (header.fileVersion != 8) {
			throw std::runtime_error("unsupported packfile version");
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
				while (!stream.atEnd()) {
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
				while (!stream.atEnd()) {
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

				uint32_t offset;
				uint32_t section;
				uint32_t target;

				while (!stream.atEnd()) {
					stream >> offset;

					if (offset == 0xFFFFFFFF)
						break;

					stream >> section >> target;

					if (section >= static_cast<uint32_t>(header.numSections)) {
						throw std::runtime_error("section index is out of range in global fixup");
					}

					auto className = reinterpret_cast<char *>(m_mapping.data() + sectionHeaders[section].absoluteDataStart + target);

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

					if (classMayHaveVtable(*classIt)) {
						fixup(data, dataSize, header.layoutRules, offset, sectionHeaders[section].absoluteDataStart + target);\
					}
				}
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
	}	

	HKXPackfileLoader::~HKXPackfileLoader() {

	}
	
	bool HKXPackfileLoader::classMayHaveVtable(const HavokClass *classReflection) const {
		auto begin = m_layout->typeInfos;
		auto end = m_layout->typeInfos + m_layout->typeInfoCount;
		auto classIt = std::lower_bound(begin, end, classReflection->name, [](const HavokTypeInfo *hClass, const char *hClassName) {
			return strcmp(hClass->name, hClassName) < 0;
		});

		if (classIt == end || strcmp((*classIt)->name, classReflection->name) != 0)
			return true;

		printf("vtable for %s is %08llX\n", classReflection->name, (*classIt)->vtable);

		return (*classIt)->vtable != 0;
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
		if (!pointer)
			return HKXStructRef();

		auto it = m_structures.find(pointer);
		if (it == m_structures.end()) {
			auto ptr = std::make_shared<HKXStruct>();
			m_structures.emplace(pointer, ptr);

			printf("pointer: %llu\n", pointer);

			Deserializer stream(layoutRules, m_mapping.data() + pointer, m_mapping.size() - static_cast<size_t>(pointer));

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
	
	void HKXPackfileLoader::parseStructure(const HavokClass *classReflection, Deserializer &stream, HKXStruct &target, bool nested) {
		if (!nested) {
			if (classMayHaveVtable(classReflection)) {
				printf("checking for override of %s\n", classReflection->name);
				uint64_t className;

				stream.mark();
				stream.readPointer(className);
				stream.seekFromMark(0);

				if (className != 0) {

					auto classNameStr = reinterpret_cast<char *>(m_mapping.data() + className);

					bool classFound = false;


					auto begin = m_layout->classes;
					auto end = m_layout->classes + m_layout->classCount;
					auto classIt = std::lower_bound(begin, end, classNameStr, [](const HavokClass *hClass, const char *hClassName) {
						return strcmp(hClass->name, hClassName) < 0;
					});

					if (classIt == end || strcmp((*classIt)->name, classNameStr) != 0) {
						std::stringstream error;
						error << "No definition for class " << classNameStr;
						throw std::runtime_error(error.str());
					}

					for (auto classInChain = *classIt; classInChain; classInChain = classInChain->parent) {
						if (classInChain == classReflection) {
							classFound = true;
						}
					}

					if (!classFound) {
						std::stringstream error;
						error << "VTable mismatch: vtable points to " << classNameStr << ", but it is not derived from " << classReflection->name;
						throw std::runtime_error(error.str());
					}

					if (*classIt != classReflection) {
						printf("renamed %s to %s\n", classReflection->name, classNameStr);

						classReflection = *classIt;
					}
				}
			}
		}

		printf("deserializing %s, nested %d\n", classReflection->name, nested);

		if (classReflection->parent) {
			parseStructure(classReflection->parent, stream, target, true);
			printf("back to %s\n", classReflection->name);
		}

		target.classNames.emplace_back(classReflection->name);

		stream.mark();

		for (size_t memberIndex = 0; memberIndex < classReflection->numDeclaredMembers; memberIndex++) {
			auto &member = classReflection->declaredMembers[memberIndex];

			printf("member: %s, type: %u, subtype: %u, array size: %u, flags: %u, offset: %u\n", member.name, member.type, member.subtype, member.arraySize, member.flags, member.offset);

			if (!(member.flags & 1024)) {
				auto &it = target.fields.emplace(member.name, std::monostate());
				deserializeField(stream, member, it.first->second);
			}
		}

		if (nested) {
			stream.seekFromMark(0);
		}
		else {
			stream.seekFromMark(classReflection->objectSize);
		}
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

			if (member.subtype == HavokType::Struct || (member.subtype == HavokType::Pointer && member.typeClass)) {
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

			uint64_t ptr;
			uint32_t len;
			stream.readPointer(ptr);
			stream >> len;
			
			printf("array: ptr %llu, length %u\n", ptr, len);

			Deserializer arrayStream(stream.layoutRules(), m_mapping.data() + ptr, static_cast<size_t>(m_mapping.size() - ptr));

			if (member.subtype == HavokType::Int8 || member.subtype == HavokType::UInt8) {
				std::vector<unsigned char> data(len);
				arrayStream.readBytes(data.data(), data.size());

				value = std::move(data);
			}
			else {
				value = HKXArray();
				auto &ary = std::get<HKXArray>(value);

				ary.values.resize(len);

				for (size_t index = 0; index < len; index++) {
					deserializeField(arrayStream, member, member.subtype, ary.values[index]);
				}
			}

			break;
		}

		case HavokType::InPlaceArray:
			__debugbreak();
			break;

		case HavokType::Enum:
			deserializeField(stream, member, member.subtype, value);
			break;

		case HavokType::Struct:
		{
			value = HKXStruct();

			auto mark = stream.getMark();

			parseStructure(member.typeClass, stream, std::get<HKXStruct>(value));

			stream.mark(mark);

			break;
		}

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
		{
			uint16_t val;
			stream >> val;

			union {
				float f;
				uint32_t i;
			} u;

			u.i = half_to_float(val);
			value = u.f;
			break;
		}

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