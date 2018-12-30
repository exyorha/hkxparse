#ifndef HKXPARSE_PACKFILE_TYPES_H
#define HKXPARSE_PACKFILE_TYPES_H

#include <stdint.h>
#include <hkxparse/LayoutRules.h>

namespace hkxparse {
	enum : int32_t {
		PackfileMagic0 = 0x57E0E057,
		PackfileMagic1 = 0x10C0C010
	};

	struct PackfileHeader {
		int32_t magic0;
		int32_t magic1;
		int32_t userTag;
		int32_t fileVersion;
		LayoutRules layoutRules;
		int32_t numSections;
		int32_t contentsSectionIndex;
		int32_t contentsSectionOffset;
		int32_t contentsClassNameSectionIndex;
		int32_t contentsClassNameSectionOffset;
		char contentsVersion[16];
		int32_t flags;
		int32_t pad;
	};

	struct PackfileSectionHeader {
		char sectionTag[20];
		int32_t absoluteDataStart;
		int32_t localFixupsOffset;
		int32_t globalFixupsOffset;
		int32_t virtualFixupsOffset;
		int32_t exportsOffset;
		int32_t importsOffset;
		int32_t endOffset;
	};
}

#endif
