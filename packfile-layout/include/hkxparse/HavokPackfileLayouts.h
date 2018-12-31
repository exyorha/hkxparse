#ifndef HKXPARSE_HAVOK_PACKFILE_LAYOUTS_H
#define HKXPARSE_HAVOK_PACKFILE_LAYOUTS_H

namespace hkxparse {
	struct HavokClass;

	struct HavokPackfileLayout {
		const char *name;
		unsigned char layoutRules[4];
		const HavokClass *const *classes;
		unsigned int classCount;
	};

	const HavokPackfileLayout *findLayout(const char *name, const unsigned char *layoutRules);
}

#endif
