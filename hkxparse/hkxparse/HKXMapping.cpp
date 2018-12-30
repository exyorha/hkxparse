#include <hkxparse/HKXMapping.h>

#include <algorithm>

namespace hkxparse {
	HKXMapping::HKXMapping() noexcept : m_size(0), m_mapping(nullptr) {

	}

	HKXMapping::HKXMapping(size_t size) : m_size(size), m_mapping(new unsigned char[size]) {

	}

	HKXMapping::~HKXMapping() {
		delete[] m_mapping;
	}

	HKXMapping::HKXMapping(HKXMapping &&other) noexcept : m_size(0), m_mapping(nullptr) {
		swap(other);
	}

	HKXMapping &HKXMapping::operator =(HKXMapping &&other) noexcept {
		swap(other);

		return *this;
	}

	void HKXMapping::swap(HKXMapping &other) noexcept {
		std::swap(m_size, other.m_size);
		std::swap(m_mapping, other.m_mapping);
	}
}