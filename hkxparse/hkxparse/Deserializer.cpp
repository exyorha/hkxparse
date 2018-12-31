#include <hkxparse/Deserializer.h>

#include <stdexcept>

namespace hkxparse {
	Deserializer::Deserializer(const LayoutRules &layoutRules, const unsigned char *data, size_t dataSize) : m_layoutRules(layoutRules), m_ptr(data), m_end(data + dataSize), m_mark(data) {

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


	void Deserializer::readBool(bool &val) {
		__debugbreak();
	}

	void Deserializer::readPointer(uint64_t &val) {
		if (m_layoutRules.bytesInPointer == 4) {
			uint32_t proxy;
			*this >> proxy;
			val = proxy;
		}
		else if (m_layoutRules.bytesInPointer == 8) {
			*this >> val;
		}
		else {
			throw std::runtime_error("unsupported pointer length");
		}
	}

	Deserializer &Deserializer::operator >>(char &value) {
		union {
			char val;
			unsigned char bytes[1];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		value = u.val;

		return *this;
	}
	Deserializer &Deserializer::operator >>(int8_t &value) {
		union {
			int8_t val;
			unsigned char bytes[1];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		value = u.val;

		return *this;
	}

	Deserializer &Deserializer::operator >>(uint8_t &value) {
		union {
			uint8_t val;
			unsigned char bytes[1];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		value = u.val;

		return *this;
	}

	Deserializer &Deserializer::operator >>(int16_t &value) {
		union {
			int16_t val;
			unsigned char bytes[2];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		if (m_layoutRules.littleEndian) {
			value = u.val;
		}
		else {
			value = _byteswap_ushort(u.val);
		}

		return *this;
	}

	Deserializer &Deserializer::operator >>(uint16_t &value) {
		union {
			uint16_t val;
			unsigned char bytes[2];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		if (m_layoutRules.littleEndian) {
			value = u.val;
		}
		else {
			value = _byteswap_ushort(u.val);
		}

		return *this;
	}

	Deserializer &Deserializer::operator >>(int32_t &value) {
		union {
			int32_t val;
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


	Deserializer &Deserializer::operator >>(int64_t &value) {
		union {
			int64_t val;
			unsigned char bytes[4];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		if (m_layoutRules.littleEndian) {
			value = u.val;
		}
		else {
			value = _byteswap_uint64(u.val);
		}

		return *this;
	}

	Deserializer &Deserializer::operator >>(uint64_t &value) {
		union {
			uint64_t val;
			unsigned char bytes[4];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		if (m_layoutRules.littleEndian) {
			value = u.val;
		}
		else {
			value = _byteswap_uint64(u.val);
		}

		return *this;
	}


	Deserializer &Deserializer::operator >>(float &val) {
		union {
			uint32_t val;
			float f;
			unsigned char bytes[4];
		} u;

		readBytes(u.bytes, sizeof(u.bytes));

		if (!m_layoutRules.littleEndian) {
			u.val = _byteswap_ulong(u.val);
		}

		val = u.f;

		return *this;
	}

	void Deserializer::mark() {
		m_mark = m_ptr;
	}

	void Deserializer::seekFromMark(size_t offset) {
		auto target = m_mark + offset;
		if (target > m_end) {
			throw std::runtime_error("seek is out of range");
		}

		m_ptr = target;
	}
}
