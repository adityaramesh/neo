/*
** File Name: io_method.hpp
** Author:    Aditya Ramesh
** Date:      07/01/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z1B530BF8_4263_4E8F_AE5B_796D4FD438B8
#define Z1B530BF8_4263_4E8F_AE5B_796D4FD438B8

#include <cassert>
#include <ostream>
#include <ccbase/format.hpp>
#include <neo/utility/enum_bitmask.hpp>

namespace neo {

enum class io_method : unsigned
{
	// Valid states are either `paging` or `direct` OR'd with either
	// `buffer` or `mmap`.
	buffer,
	mmap,
	paging,
	direct,
};

DEFINE_ENUM_BITWISE_OPERATORS(io_method)

std::ostream& operator<<(std::ostream& os, io_method m)
{
	if (!!(m & io_method::buffer)) {
		if (!!(m & io_method::paging)) {
			cc::write(os, "paged buffered IO");
		}
		else if (!!(m & io_method::direct)) {
			cc::write(os, "direct buffered IO");
		}
		else {
			assert(false && "Invalid io_method value.");
		}
	}
	else if (!!(m & io_method::mmap)) {
		if (!!(m & io_method::paging)) {
			cc::write(os, "paged mmap IO");
		}
		else if (!!(m & io_method::direct)) {
			cc::write(os, "direct mmap IO");
		}
		else {
			assert(false && "Invalid io_method value.");
		}
	}
	else {
		assert(false && "Invalid io_method value.");
	}
	return os;
}

}

#endif
