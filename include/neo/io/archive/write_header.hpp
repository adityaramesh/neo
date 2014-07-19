/*
** File Name: write_header.hpp
** Author:    Aditya Ramesh
** Date:      07/18/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z1214CF91_7CA1_4E99_A64E_9BDBF9939CEF
#define Z1214CF91_7CA1_4E99_A64E_9BDBF9939CEF

#include <cassert>
#include <type_traits>
#include <neo/core/operation_status.hpp>
#include <neo/io/archive/definitions.hpp>

namespace neo {
namespace archive {
namespace detail {


}

template <class SerializedType>
operation_status
write_header(
	uint8_t* buf, size_t n,
	io_state<SerializedType>& is,
	buffer_state& bs, error_state& es
) noexcept
{

}

}}

#endif
