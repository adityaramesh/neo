/*
** File Name:	storage_order.hpp
** Author:	Aditya Ramesh
** Date:	05/23/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z6191EFDA_D65C_4622_9870_C1A827D996D8
#define Z6191EFDA_D65C_4622_9870_C1A827D996D8

#include <cstdint>
#include <neo/utility/enum_bitmask.hpp>

namespace neo {
namespace archive {

enum class storage_order : uint8_t
{
	row_major,
	column_major,
};

}}

#endif
