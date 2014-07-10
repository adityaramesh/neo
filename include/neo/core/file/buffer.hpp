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
#include <neo/core/io_type.hpp>
#include <neo/core/buffer_constraints.hpp>
#include <neo/core/file/open_mode.hpp>

namespace neo {
namespace file {

class buffer
{
	enum source
	{
		memalign,
		allocator,
		mmap_read,
		mmap_write,
		mmap_read_write
	};

	uint8_t* m_buf{};
	size_t m_size{};
	off_t m_off{};
	source m_src;
public:
	/*
	** Creates a mapped buffer with access permissions determined by the
	** value of the given enum.
	*/
	explicit buffer(io_type t, uint8_t* ptr, size_t size, off_t off = 0) noexcept
	: m_buf{ptr}, m_size{size}, m_off{off}
	{
		switch (t) {
		case io_type::input: m_src = mmap_read; break;
		case io_type::output: m_src = mmap_write; break;
		case io_type::input | io_type::output: m_src = mmap_read_write; break;
		}
	}

	/*
	** Allocates a buffer with the smallest size satisfying the given
	** constraints.
	*/
	explicit buffer(const buffer_constraints& bc)
	{ resize_helper(bc); }

	~buffer()
	{
		switch (m_src) {
		case memalign:  std::free(m_buf); break;
		case allocator: delete[] m_buf;   break;
		}
	}

	decltype(m_buf) data() const { return m_buf + m_off; }
	bool readable() const { return m_src != mmap_write; }
	bool writable() const { return m_src != mmap_read; }

	bool mapped() const
	{
		return m_src == mmap_read  ||
		       m_src == mmap_write ||
		       m_src == mmap_read_write;
	}

	size_t size() const
	{
		assert(m_off <= m_size);
		return m_size - m_off;
	}

	buffer& resize(const buffer_constraints& bc)
	{
		assert(!mapped());
		this->~buffer();
		resize_helper(bc);
		return *this;
	}
private:
	template <open_mode>
	friend class seekable_file;

	decltype(m_buf) base_pointer() const { return m_buf; }

	buffer& offset(off_t off)
	{
		assert(mapped());
		m_off = off;
		return *this;
	}

	void resize_helper(const buffer_constraints& bc)
	{
		m_size = min_size(bc).get();
		if (bc.align_to()) {
			m_src = memalign;
			auto r = ::posix_memalign((void**)&m_buf, bc.align_to().get(), m_size);
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
