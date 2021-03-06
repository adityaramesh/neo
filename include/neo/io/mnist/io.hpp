/*
** File Name: io.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z650E10DF_38E3_45E4_ADF4_A98E0504E45E
#define Z650E10DF_38E3_45E4_ADF4_A98E0504E45E

#include <cassert>
#include <neo/core/operation_status.hpp>
#include <neo/io/mnist/definitions.hpp>
#include <ccbase/platform.hpp>

namespace neo {
namespace mnist {

static constexpr auto hdr_size = 8;

operation_status
read_header(
	uint8_t* buf, size_t n,
	image_io_state& is, buffer_state& bs, error_state& es
) noexcept
{
	(void)n;
	assert(n >= is.header_size());

	auto hdr = reinterpret_cast<uint32_t*>(buf);
	bs.consumed(is.header_size());

	#if PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_LITTLE
		if (!(hdr[0] == 0x01080000 || hdr[0] == 0x03080000)) {
			es.push_record(
				severity::critical,
				context{offset_type{0}},
				"Invalid image file magic."
			);
			return operation_status::failure |
				operation_status::fatal_error;
		}
		is.element_count(cc::bswap(hdr[1]));
	#elif PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_BIG
		if (!(hdr[0] == 0x00000801 || hdr[0] == 0x00000803)) {
			es.push_record(
				severity::critical,
				context{offset_type{0}},
				"Invalid image file magic."
			);
			return operation_status::failure |
				operation_status::fatal_error;
		}
		is.element_count(hdr[1]);
	#endif

	return operation_status::success;
}

operation_status
read_header(
	uint8_t* buf, size_t n,
	label_io_state& ls, buffer_state& bs, error_state& es
) noexcept
{
	(void)n;
	assert(n >= ls.header_size());

	auto hdr = reinterpret_cast<uint32_t*>(buf);
	bs.consumed(ls.header_size());

	#if PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_LITTLE
		if (!(hdr[0] == 0x01080000 || hdr[0] == 0x03080000)) {
			es.push_record(
				severity::critical,
				context{offset_type{0}},
				"Invalid label file magic."
			);
			return operation_status::failure |
				operation_status::fatal_error;
		}
		ls.element_count(cc::bswap(hdr[1]));
	#elif PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_BIG
		if (!(hdr[0] == 0x00000801 || hdr[0] == 0x00000803)) {
			es.push_record(
				severity::critical,
				context{offset_type{0}},
				"Invalid label file magic."
			);
			return operation_status::failure |
				operation_status::fatal_error;
		}
		ls.element_count(hdr[1]);
	#endif

	return operation_status::success;
}

operation_status
scan(
	uint8_t* buf, size_t n,
	image_io_state& is, buffer_state& bs, error_state&
) noexcept
{
	(void)n;
	using value_type = image_io_state::value_type;
	assert(n >= is.element_size());

	::new (&is.element()) value_type{buf};
	bs.consumed(is.element_size());
	return operation_status::success;
}

operation_status
scan(
	uint8_t* buf, size_t n,
	label_io_state& ls, buffer_state& bs, error_state&
) noexcept
{
	(void)n;
	assert(n >= ls.element_size());

	ls.element(*buf);
	bs.consumed(ls.element_size());
	return operation_status::success;
}

}}

#endif
