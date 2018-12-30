#ifndef HKXPARSE_HKX_MAPPING_H
#define HKXPARSE_HKX_MAPPING_H

namespace hkxparse {
	class HKXMapping {
	public:
		HKXMapping() noexcept;
		HKXMapping(size_t size);
		~HKXMapping();

		HKXMapping(const HKXMapping &other) = delete;
		HKXMapping &operator =(const HKXMapping &other) = delete;

		HKXMapping(HKXMapping &&other) noexcept;
		HKXMapping &operator =(HKXMapping &&other) noexcept;

		inline operator bool() const {
			return m_mapping;
		}
		
		inline unsigned char *data() const { return m_mapping; }
		inline size_t size() const { return m_size; }

		void swap(HKXMapping &other) noexcept;

	private:
		size_t m_size;
		unsigned char *m_mapping;
	};
}

#endif
