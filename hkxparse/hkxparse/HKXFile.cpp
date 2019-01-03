#include <hkxparse/HKXFile.h>
#include <hkxparse/PackfileTypes.h>
#include <hkxparse/HKXPackfileLoader.h>
#include <hkxparse/TagfileTypes.h>
#include <hkxparse/HKXTagfileParser.h>

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

		if (m_mapping.size() >= sizeof(TagfileHeader)) {
			const auto &header = *reinterpret_cast<TagfileHeader *>(m_mapping.data());
			if ((header.magic0 == TagfileMagic0 && header.magic1 == TagfileMagic1) ||
				(header.magic0 == _byteswap_ulong(TagfileMagic0) && header.magic1 == _byteswap_ulong(TagfileMagic1))) {

				parseTagfile();
				return;
			}
		}

		throw std::runtime_error("hkx container not identified");
	}

	void HKXFile::parsePackfile() {
		HKXPackfileLoader loader(m_mapping);
		m_root = loader.loadRoot();
	}

	void HKXFile::parseTagfile() {
		HKXTagfileParser parser(m_mapping);
		m_root = parser.parse();		
	}

}
