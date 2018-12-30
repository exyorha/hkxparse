#ifndef HKXPARSE_LAYOUT_RULES_H
#define HKXPARSE_LAYOUT_RULES_H

#include <stdint.h>

namespace hkxparse {
	struct LayoutRules {
		uint8_t bytesInPointer;
		uint8_t littleEndian;
		uint8_t reusePaddingOptimization;
		uint8_t emptyBaseClassOptimization;
	};
}

#endif
