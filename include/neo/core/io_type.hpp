/*
** File Name: io_type.hpp
** Author:    Aditya Ramesh
** Date:      07/09/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z68EEB476_1C46_4C67_B314_9D2B39ABE8D9
#define Z68EEB476_1C46_4C67_B314_9D2B39ABE8D9

#include <neo/core/bitmask_enum.hpp>

namespace neo {

enum class io_type : unsigned
{
	input  = 1,
	output = 2,
};

DEFINE_ENUM_BITWISE_OPERATORS(io_type)

}

#endif
