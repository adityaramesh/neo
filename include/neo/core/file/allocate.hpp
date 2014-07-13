/*
** File Name: allocate.hpp
** Author:    Aditya Ramesh
** Date:      07/10/2014
** Contact:   _@adityaramesh.com
**
** This file declares functions that allocate buffers used to buffer input,
** output, or both.
*/

#ifndef Z7F9C1B33_ED86_4319_93E4_5809D7EDCBE7
#define Z7F9C1B33_ED86_4319_93E4_5809D7EDCBE7

#include <limits>
#include <type_traits>
#include <neo/core/file/buffer.hpp>
#include <neo/core/file/handle.hpp>
#include <neo/core/file/strategy.hpp>

namespace neo {
namespace file {

template <
	io_mode IOMode,
	typename std::enable_if<!!(IOMode & io_mode::input), int>::type = 0
>
buffer<IOMode>
allocate_ibuffer(
	const handle<IOMode>& h,
	const strategy<IOMode>& s, 
	const buffer_constraints& bc
)
{
	assert(s.read_method());
	assert(bc.satisfies(s.required_constraints(io_mode::input)));

	if (s.read_method() == io_method::mmap) {
		return buffer<IOMode>{h.map(), (size_t)s.current_file_size().get()};
	}
	else {
		return buffer<IOMode>{bc};
	}
}

template <
	io_mode IOMode,
	typename std::enable_if<!!(IOMode & io_mode::input), int>::type = 0
>
buffer<IOMode>
allocate_ibuffer(const handle<IOMode>& h, const strategy<IOMode>& s)
{
	return allocate_ibuffer(h, s, s.preferred_constraints(io_mode::input));
}

template <
	io_mode IOMode,
	typename std::enable_if<!!(IOMode & io_mode::output), int>::type = 0
>
buffer<IOMode>
allocate_obuffer(
	const handle<IOMode>& h,
	const strategy<IOMode>& s,
	const buffer_constraints& bc
)
{
	assert(s.write_method());
	assert(bc.satisfies(s.required_constraints(io_mode::output)));

	if (s.write_method() == io_method::mmap) {
		return buffer<IOMode>{h.map(), (size_t)s.maximum_file_size().get()};
	}
	else {
		return buffer<IOMode>{bc};
	}
}

template <
	io_mode IOMode,
	typename std::enable_if<!!(IOMode & io_mode::output), int>::type = 0
>
buffer<IOMode>
allocate_obuffer(const handle<IOMode>& h, const strategy<IOMode>& s)
{
	return allocate_obuffer(h, s, s.preferred_constraints(io_mode::output));
}

buffer<io_mode::input | io_mode::output>
allocate_iobuffer(
	const handle<io_mode::input | io_mode::output>& h,
	const strategy<io_mode::input | io_mode::output>& s,
	const buffer_constraints& bc
)
{
	using buffer_type = buffer<io_mode::input | io_mode::output>;
	assert(s.supports_dual_use_buffers());
	assert(bc.satisfies(s.required_constraints(io_mode::input | io_mode::output)));

	if (s.read_method() == io_method::mmap &&
		s.write_method() == io_method::mmap)
	{
		return buffer_type{h.map(), (size_t)s.maximum_file_size().get()};
	}
	else {
		return buffer_type{bc};
	}
}

buffer<io_mode::input | io_mode::output>
allocate_iobuffer(
	const handle<io_mode::input | io_mode::output>& h,
	const strategy<io_mode::input | io_mode::output>& s
)
{
	return allocate_iobuffer(h, s, s.preferred_constraints(
		io_mode::input | io_mode::output));
}

}}

#endif
