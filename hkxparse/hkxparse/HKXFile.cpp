#include <hkxparse/HKXFile.h>
#include <hkxparse/PackfileTypes.h>
#include <hkxparse/Deserializer.h>

#include <fstream>

namespace hkxparse {
	HKXFile::HKXFile() {

	}

	HKXFile::~HKXFile() {

	}

	void HKXFile::loadFile(const char *filename) {
		std::ifstream stream;
		stream.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
		stream.open(filename, std::ios::in | std::ios::binary);
		loadFile(stream);
	}

	void HKXFile::loadFile(const wchar_t *filename) {
		std::ifstream stream;
		stream.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
		stream.open(filename, std::ios::in | std::ios::binary);
		loadFile(stream);
	}

	void HKXFile::loadFile(std::istream &stream) {
		stream.seekg(0, std::ios::end);

		auto size = static_cast<size_t>(stream.tellg());

		auto mapping = HKXMapping(size);

		stream.seekg(0);

		stream.read(reinterpret_cast<char *>(mapping.data()), size);

		loadFile(std::move(mapping));
	}

	void HKXFile::loadFile(HKXMapping &&mapping) {
		m_mapping = std::move(mapping);
		doLoadFile();
	}

	void HKXFile::doLoadFile() {
		if (m_mapping.size() >= sizeof(PackfileHeader)) {
			const auto &header = *reinterpret_cast<PackfileHeader *>(m_mapping.data());
			if (header.magic0 == PackfileMagic0 && header.magic1 == PackfileMagic1) {
				parsePackfile();
				return;
			}
		}

		throw std::runtime_error("hkx container not identified");
	}

	void HKXFile::parsePackfile() {
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
	}

	void HKXFile::fixup(unsigned char *data, size_t dataSize, const LayoutRules &layoutRules, size_t offset, size_t target) {
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
}
