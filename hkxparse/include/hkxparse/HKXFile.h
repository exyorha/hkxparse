#ifndef HKXPARSE_HKX_FILE_H
#define HKXPARSE_HKX_FILE_H

#include <ios>
#include "HKXMapping.h"
#include "HKXTypes.h"

namespace hkxparse {
	class HKXFile {
	public:
		HKXFile();
		~HKXFile();

		void loadFile(const char *filename);
		void loadFile(const wchar_t *filename);
		void loadFile(std::istream &stream);
		void loadFile(HKXMapping &&mapping);

		inline const HKXStructRef &root() const { return m_root; }

	private:
		void doLoadFile();
		void parsePackfile();
		void parseTagfile();

		HKXMapping m_mapping;
		HKXStructRef m_root;
	};
}

#endif
