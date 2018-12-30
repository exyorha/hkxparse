#ifndef HKXPARSE_HKX_FILE_H
#define HKXPARSE_HKX_FILE_H

#include <ios>
#include "HKXMapping.h"

namespace hkxparse {
	struct LayoutRules;

	class HKXFile {
	public:
		HKXFile();
		~HKXFile();

		void loadFile(const char *filename);
		void loadFile(const wchar_t *filename);
		void loadFile(std::istream &stream);
		void loadFile(HKXMapping &&mapping);

	private:
		void doLoadFile();
		void parsePackfile();

		void fixup(unsigned char *data, size_t dataSize, const LayoutRules &layoutRules, size_t offset, size_t target);

		HKXMapping m_mapping;
	};
}

#endif
