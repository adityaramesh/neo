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

/*
** Helper class that writes the header of the archive.
*/
template <class... Ts>
struct write_header;

template <class Scalar>
struct write_header<Scalar>
{
	static uint8_t* apply(uint8_t* buf)
	{
		buf[0] = scalar_code<Scalar>::value;
		buf[1] = 0;
		return buf + 2;
	}
};

template <class Scalar, size_t Size>
struct write_header<vector<Scalar, Size>>
{
	static uint8_t* apply(uint8_t* buf)
	{
		buf[0] = scalar_code<Scalar>::value;
		buf[1] = 1;
		*(uint32_t*)(buf + 2) = Size;
		return buf + 6;
	}
};

template <
	class Scalar,
	size_t Rows,
	size_t Cols,
	storage_order Order
>
struct write_header<matrix<Scalar, Rows, Cols, Order>>
{
	static uint8_t* apply(uint8_t* buf)
	{
		buf[0] = scalar_code<Scalar>::value;
		buf[1] = 2;
		buf[2] = static_cast<uint8_t>(Order);
		*(uint32_t*)(buf + 3) = Rows;
		*(uint32_t*)(buf + 7) = Cols;
		return buf + 11;
	}
};

template <class T, class... Ts>
struct write_header<T, Ts...>
{
	static void apply(uint8_t* buf)
	{
		auto p = write_header<T>::apply(buf);
		write_header<Ts...>::apply(p);
	}
};

template <class... Ts>
struct write_header<std::tuple<Ts...>>
{
	static void apply(uint8_t* buf)
	{
		buf[0] = version;
		buf[1] = static_cast<uint8_t>(platform_byte_order);
		buf[2] = sizeof...(Ts);
		write_header<Ts...>::apply(buf + 3);
	}
};

}

template <class SerializedType>
operation_status
write_header(
	uint8_t* buf, size_t n,
	io_state<SerializedType>& is,
	buffer_state& bs, error_state&
) noexcept
{
	(void)n;
	assert(n >= is.element_size());
	detail::write_header<SerializedType>::apply(buf);

	static constexpr auto elem_count_size = 8;

	if (is.element_count()) {
		*reinterpret_cast<uint64_t*>(buf + is.header_size() - elem_count_size) =
			is.element_count();
		bs.consumed(is.header_size());
	}
	else {
		bs.consumed(is.header_size() - elem_count_size);
	}
	return operation_status::success;
}

}}

#endif
