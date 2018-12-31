#include <hkxparse/HavokPackfileLayouts.h>
#include <hk_2010.2.0-r1_1-0-1-4.h>

#include <string.h>

namespace hkxparse {
	static const struct HavokPackfileLayout packfileLayouts[] = {
		{ "hk_2010.2.0-r1", { 4, 1, 0, 1 }, hk_2010_2_0_r1_1_0_1_4Classes, sizeof(hk_2010_2_0_r1_1_0_1_4Classes) / sizeof(hk_2010_2_0_r1_1_0_1_4Classes[0]) },
	};

	const HavokPackfileLayout *findLayout(const char *name, const unsigned char *layoutRules) {
		for (size_t index = 0; index < sizeof(packfileLayouts) / sizeof(packfileLayouts[0]); index++) {
			const auto &layout = packfileLayouts[index];

			if (strcmp(layout.name, name) == 0 && memcmp(layout.layoutRules, layoutRules, sizeof(layoutRules)) == 0) {
				return &layout;
			}
		}

		return nullptr;
	}
}
