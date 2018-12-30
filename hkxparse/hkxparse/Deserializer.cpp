#include <hkxparse/Deserializer.h>

#include <stdexcept>

namespace hkxparse {
	Deserializer::Deserializer(const LayoutRules &layoutRules, const unsigned char *data, size_t dataSize) : m_layoutRules(layoutRules), m_ptr(data), m_end(data + dataSize) {

	}

	Deserializer::~Deserializer() {

	}

	void Deserializer::readBytes(unsigned char *target, size_t size) {
		if (m_ptr + size > m_end) {
			throw std::runtime_error("out of bounds read");
		}

		memcpy(target, m_ptr, size);
		m_ptr += size;
	}

	Deserializer &Deserializer::operator >>(uint32_t &value) {
		union {
			uint32_t val;
			unsigned char bytes[4];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		if (m_layoutRules.littleEndian) {
			value = u.val;
		}
		else {
			value = _byteswap_ulong(u.val);
		}

		return *this;
	}
}
