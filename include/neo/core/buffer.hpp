/*
** File Name: buffer.hpp
** Author:    Aditya Ramesh
** Date:      06/30/2014
** Contact:   _@adityaramesh.com
**
** A simple buffer class used for IO. The buffer is associated with a size and
** capacity, and accommodates for arbitrary alignment requirements. It is also
** resizable.
*/

#ifndef Z6BA46DF5_A643_4FA2_9445_FBBADE3471B9
#define Z6BA46DF5_A643_4FA2_9445_FBBADE3471B9

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <neo/core/buffer_constraints.hpp>

namespace neo {

class buffer
{
	enum buffer_source
	{
		memalign,
		allocator,
		external
	};

	uint8_t* m_buf{};
	size_t m_size{};
	buffer_source m_src{external};
public:
	explicit buffer() noexcept {}

	explicit buffer(const buffer_constraints& bc)
	{ resize_helper(bc); }

	~buffer()
	{
		switch (m_src) {
		case memalign:  std::free(m_buf); break;
		case allocator: delete[] m_buf;   break;
		}
	}

	decltype(m_buf) data() const { return m_buf; }
	decltype(m_size) size() const { return m_size; }

	buffer& data(uint8_t* p)
	{
		m_src = external;
		m_buf = p;
		return *this;
	}

	buffer& size(size_t n)
	{
		assert(m_src == external);
		m_size = n;
		return *this;
	}

	buffer& resize(const buffer_constraints& bc)
	{
		this->~buffer();
		resize_helper(bc);
		return *this;
	}
private:
	void resize_helper(const buffer_constraints& bc)
	{
		m_size = min_size(bc).get();
		if (bc.align_to()) {
			m_src = memalign;
			m_buf = ::posix_memalign((void**)&m_buf, bc.align_to().get(), m_cap);
		}
		else {
			m_src = allocator;
			m_buf = new size_t[m_cap];
		}
	}
};

}

#endif
