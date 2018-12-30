#ifndef HKXPARSE_DESERIALIZER_H
#define HKXPARSE_DESERIALIZER_H

#include "LayoutRules.h"

namespace hkxparse {
	class Deserializer {
	public:
		Deserializer(const LayoutRules &layoutRules, const unsigned char *data, size_t dataSize);
		~Deserializer();

		Deserializer(const Deserializer &other) = delete;
		Deserializer &operator =(const Deserializer &other) = delete;

		Deserializer &operator >>(uint32_t &value);
		
		void readBytes(unsigned char *target, size_t size);

	private:
		LayoutRules m_layoutRules;
		const unsigned char *m_ptr;
		const unsigned char *m_end;
	};
}

#endif
