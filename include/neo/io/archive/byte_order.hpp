/*
** File Name:	byte_order.hpp
** Author:	Aditya Ramesh
** Date:	05/23/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z01B1173D_9745_4F1D_B91C_E915545BE08C
#define Z01B1173D_9745_4F1D_B91C_E915545BE08C

#include <ccbase/platform.hpp>
#include <neo/utility/bitmask_enum.hpp>

namespace neo {
namespace archive {

enum class byte_order : unsigned
{
	integer_little = 0x0,
	integer_big    = 0x1,
	float_little   = 0x0,
	float_big      = 0x2,
	integer_mask   = 0x01,
	float_mask     = 0x10,
};

DEFINE_ENUM_BITWISE_OPERATORS(byte_order)

#if PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_LITTLE
	static constexpr auto platform_integer_byte_order = byte_order::integer_little;
	static constexpr auto platform_float_byte_order   = byte_order::integer_little;
	static constexpr auto platform_byte_order         = byte_order::integer_little | byte_order::float_little;
#elif PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_BIG
	static constexpr auto platform_integer_byte_order = byte_order::integer_big;
	static constexpr auto platform_float_byte_order   = byte_order::integer_big;
	static constexpr auto platform_byte_order         = byte_order::integer_big | byte_order::float_big;
#else
	#error "Unsupported platform integer byte order."
#endif

}}

#endif
