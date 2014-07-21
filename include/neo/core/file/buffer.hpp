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
#include <neo/core/io_mode.hpp>
#include <neo/core/buffer_constraints.hpp>
#include <neo/core/file/open_mode.hpp>
#include <neo/core/file/system.hpp>

namespace neo {
namespace file {

template <io_mode IOMode>
class buffer
{
	enum source
	{
		memalign,
		allocator,
		mmap_read_only,
		mmap_write_only,
		mmap_read_write
	};

	uint8_t* m_buf{};
	size_t m_size{};
	off_t m_off{};
	source m_src;
public:
	/*
	** Creates a buffer whose contents are backed by a mapped file.
	*/
	explicit buffer(uint8_t* map, size_t size)
	: m_buf{map}, m_size{size}
	{
		switch (IOMode) {
		case io_mode::input:
			m_src = mmap_read_only;
			break;
		case io_mode::output:
			m_src = mmap_write_only;
			break;
		case io_mode::input | io_mode::output:
			m_src = mmap_read_write;
			break;
		}
	}

	/*
	** Allocates a buffer with the smallest size satisfying the given
	** constraints.
	*/
	explicit buffer(const buffer_constraints& bc)
	{ resize_helper(bc); }

	buffer(const buffer&) = delete;

	buffer(buffer&& rhs) noexcept :
	m_buf{rhs.m_buf}, m_size{rhs.m_size}, m_off{rhs.m_off}, m_src{rhs.m_src}
	{ rhs.m_buf = nullptr; }

	buffer& operator=(const buffer&) = delete;

	buffer& operator=(buffer&& rhs) noexcept
	{
		this->~buffer();
		m_buf = rhs.m_buf;
		m_size = rhs.m_size;
		m_off = rhs.m_off;
		m_src = rhs.m_src;
		return *this;
	}

	~buffer()
	{
		switch (m_src) {
		case memalign:  std::free(m_buf); break;
		case allocator: delete[] m_buf;   break;
		}
	}

	decltype(m_buf) data() const { return m_buf + m_off; }
	bool readable() const { return m_src != mmap_write_only; }
	bool writable() const { return m_src != mmap_read_only; }

	const uint8_t* map() const
	{
		assert(mapped());
		return m_buf;
	}
	
	uint8_t* map()
	{
		assert(mapped());
		return m_buf;
	}

	bool mapped() const
	{
		return m_src == mmap_read_only ||
		       m_src == mmap_write_only ||
		       m_src == mmap_read_write;
	}

	size_t size() const
	{
		assert(m_off <= (off_t)m_size);
		return m_size - m_off;
	}

	buffer& resize(const buffer_constraints& bc)
	{
		assert(!mapped());
		this->~buffer();
		resize_helper(bc);
		return *this;
	}

	buffer& offset(off_t off)
	{
		assert(mapped());
		m_off = off;
		return *this;
	}
private:
	void resize_helper(const buffer_constraints& bc)
	{
		auto s = min_size(bc);
		m_size = s ? *s : ::getpagesize();

		if (bc.align_to()) {
			m_src = memalign;
			auto r = ::posix_memalign((void**)&m_buf, *bc.align_to(), m_size);
			if (r != 0) {
				throw std::system_error{r, std::system_category()};
			}
		}
		else {
			m_src = allocator;
			m_buf = new uint8_t[m_size];
		}
	}
};

}}

#endif
