#include <hkxparse/HKXFile.h>
#include <hkxparse/PackfileTypes.h>
#include <hkxparse/HKXPackfileLoader.h>
#include <hkxparse/PrettyPrinter.h>

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
		HKXPackfileLoader loader(m_mapping);
		auto root = loader.loadRoot();

		{
			std::ofstream stream;
			stream.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
			stream.open("C:\\projects\\hkxparse\\dump.txt", std::ios::out | std::ios::trunc);

			PrettyPrinter printer(stream);
			printer.print(root);
		}

		__debugbreak();
	}

}
