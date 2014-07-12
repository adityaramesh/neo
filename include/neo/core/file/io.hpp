/*
** File Name: io.hpp
** Author:    Aditya Ramesh
** Date:      07/12/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z5A38F0F5_4A8B_46DE_8297_588006B2CCBF
#define Z5A38F0F5_4A8B_46DE_8297_588006B2CCBF

#include <algorithm>
#include <neo/core/file/buffer.hpp>
#include <neo/core/file/strategy.hpp>
#include <neo/core/file/system.hpp>

namespace neo {
namespace file {

template <
	io_mode IOMode,
	typename std::enable_if<!!(IOMode & io_mode::input), int>::type
>
cc::expected<void>
read(
	const handle<IOMode>& h,
	offset_type off,
	size_t n,
	buffer<IOMode>& b
) noexcept
{
	assert(n > 0);

	if (!s.write_method()) {
		assert(off + n <= s.current_file_size());
	}
	else if (s.maximum_file_size()) {
		assert(off + n <= s.maximum_file_size());
	}

	switch (s.read_method().get()) {
	case io_method::mmap:
		if (b.mapped() && b.map() == h.map()) {
			b.offset(off);
		}
		else {
			assert(b.writable());
			std::copy_n(h.map() + off, n, b.data());
		}
		return true;
	case io_method::paging:
	case io_method::direct:
		assert(b.writable());
		auto r = full_read(h.descriptor(), b.data(), n, off);
		if (!r) { return r.exception(); }
		return true;
	}
}

template <
	io_mode IOMode,
	typename std::enable_if<!!(IOMode & io_mode::output), int>::type
>
cc::expected<void>
write(
	const handle<IOMode>& h,
	offset_type off,
	size_t n,
	const buffer<IOMode>& b
) noexcept
{
	assert(n > 0);

	if (s.maximum_file_size()) {
		assert(off + n <= s.maximum_file_size());
	}

	switch (s.write_method().get()) {
	case io_method::mmap:
		if (b.mapped() && b.map() == h.map()) {
			return;
		}
		else {
			assert(b.readable());
			std::copy_n(b.data(), n, h.map() + off);
		}
		return true;
	case io_method::paging:
	case io_method::direct;
		assert(b.readable());
		auto r = full_write(h.descriptor(), b.data(), n, off);
		if (!r) { return r.exception(); }
		return true;
	}
}

}}

#endif
