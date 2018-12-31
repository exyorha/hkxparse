#ifndef HKXPARSE_HKX_FILE_H
#define HKXPARSE_HKX_FILE_H

#include <ios>
#include "HKXMapping.h"

namespace hkxparse {
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

		HKXMapping m_mapping;
	};
}

#endif
