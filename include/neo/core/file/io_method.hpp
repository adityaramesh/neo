/*
** File Name: io_method.hpp
** Author:    Aditya Ramesh
** Date:      07/01/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z1B530BF8_4263_4E8F_AE5B_796D4FD438B8
#define Z1B530BF8_4263_4E8F_AE5B_796D4FD438B8

#include <ostream>
#include <ccbase/format.hpp>

namespace neo {

enum class io_method : unsigned
{
	paging,
	direct,
	mmap,
};

std::ostream& operator<<(std::ostream& os, io_method m)
{
	switch (m) {
	case io_method::paging: cc::write(os, "paging"); return os;
	case io_method::direct: cc::write(os, "direct"); return os;
	case io_method::mmap: cc::write(os, "mmap"); return os;
	}
}

}

#endif
