/*
** File Name: io.hpp
** Author:    Aditya Ramesh
** Date:      07/12/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z5A38F0F5_4A8B_46DE_8297_588006B2CCBF
#define Z5A38F0F5_4A8B_46DE_8297_588006B2CCBF

#include <neo/core/file/buffer.hpp>
#include <neo/core/file/strategy.hpp>
#include <neo/core/file/system.hpp>

namespace neo {
namespace file {

template <io_mode IOMode>
cc::expected<void>
read(
	offset_type off,
	size_t n,
	buffer& b,
	const handle& h,
	const strategy<IOMode>& s
) noexcept
{
	assert(n > 0);
	assert(!!(IOMode & io_mode::input));

	if (!s.write_method()) {
		assert(off + n <= s.current_file_size());
	}
	else if (s.maximum_file_size()) {
		assert(off + n <= s.maximum_file_size());
	}

	switch (s.read_method().get()) {
	case io_method::mmap:
		b.offset(off);
		return true;
	case io_method::paging:
	case io_method::direct:
		auto r = full_read(h.descriptor(), b.data(), n, off);
		if (!r) { return r.exception(); }
		return true;
	default:
		return std::runtime_error{"Unsupported read method."};
	}
}

template <io_mode IOMode>
cc::expected<void>
write(
	offset_type off,
	size_t n,
	const buffer& b,
	const handle& h,
	const strategy<IOMode>& s
) noexcept
{
	assert(n > 0);
	assert(!!(IOMode & io_mode::output));

	if (s.maximum_file_size()) {
		assert(off + n <= s.maximum_file_size());
	}

	switch (s.write_method().get()) {
	case io_method::mmap:
	case io_method::paging:
	case io_method::direct;
	default:
		return std::runtime_error{"Unsupported write method."};
	}
}

}}

#endif
