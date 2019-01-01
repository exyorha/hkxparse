#ifndef HKXPARSE_DESERIALIZER_H
#define HKXPARSE_DESERIALIZER_H

#include "LayoutRules.h"

#include <stdint.h>

namespace hkxparse {
	class Deserializer {
	public:
		Deserializer(const LayoutRules &layoutRules, const unsigned char *data, size_t dataSize);
		~Deserializer();

		Deserializer(const Deserializer &other) = delete;
		Deserializer &operator =(const Deserializer &other) = delete;
				
		void readBytes(unsigned char *target, size_t size);
		void readBool(bool &val);
		void readPointer(uint64_t &val);

		//Deserializer &operator >>(bool &val);
		Deserializer &operator >>(char &val);
		Deserializer &operator >>(int8_t &val);
		Deserializer &operator >>(uint8_t &val);
		Deserializer &operator >>(int16_t &val);
		Deserializer &operator >>(uint16_t &val);
		Deserializer &operator >>(int32_t &val);
		Deserializer &operator >>(uint32_t &val);
		Deserializer &operator >>(int64_t &val);
		Deserializer &operator >>(uint64_t &val);
		Deserializer &operator >>(float &val);

		inline const LayoutRules &layoutRules() const { return m_layoutRules; }

		void mark();
		inline void mark(const unsigned char *ptr) { m_mark = ptr; }
		inline const unsigned char *getMark() const { return m_mark; }

		void seekFromMark(size_t offset);

	private:
		LayoutRules m_layoutRules;
		const unsigned char *m_ptr;
		const unsigned char *m_end;
		const unsigned char *m_mark;
	};
}

#endif
