/*
** File Name: io.hpp
** Author:    Aditya Ramesh
** Date:      07/17/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZF9FCDF67_0EAB_4D8C_9952_DFE380A00205
#define ZF9FCDF67_0EAB_4D8C_9952_DFE380A00205

#include <cassert>
#include <type_traits>
#include <neo/io/archive/definitions.hpp>

namespace neo {
namespace archive {

/*
** Verifies that a given component of the element type described in the header
** matches the corresponding component of the input type.
*/

template <uint8_t Index, class T>
struct verify_component;

template <uint8_t Index, class Scalar>
struct verify_component
{
	static_assert(std::is_arithmetic<Scalar>::value, "Type must be arithmetic.");

	static boost::optional<std::tuple<const uint8_t*, size_t>>
	apply(
		const uint8_t* buf,
		std::size_t rem_buf_size,
		input_state& is,
		error_state& es
	)
	{
		if (rem_buf_size < 2) {
			es.push_record(
				severity::critical,
				context{offset_type{0}, Index},
				"Unexpected end of header."
			);
			return boost::none;
		}
		if (scalar_code<Scalar>::value != buf[0]) {
			es.push_record(
				severity::critical,
				context{offset_type{0}, Index},
				"Mismatching scalar types."
			);
			return boost::none;
		}
		if (buf[1] != 0) {
			es.push_record(
				severity::critical,
				context{offset_type{0}, Index},
				"Scalar component should have dimension zero."
			);
			return boost::none;
		}
		return std::make_tuple(buf + 2, rem_buf_size - 2);
	}
};

template <uint8_t Index, class Scalar, std::size_t Size>
struct verify_component<Index, vector<Scalar, Size>>
{
	static boost::optional<std::tuple<const uint8_t*, size_t>>
	apply(
		const uint8_t* buf,
		std::size_t rem_buf_size,
		input_state& is,
		error_state& es
	)
	{
		if (rem_buf_size < 6) {
			es.push_record(
				severity::critical,
				context{offset_type{0}, Index},
				"Unexpected end of header."
			);
			return boost::none;
		}
		if (scalar_code<Scalar>::value != buf[0]) {
			es.push_record(
				severity::critical,
				context{offset_type{0}, Index},
				"Mismatching scalar types."
			);
			return boost::none;
		}
		if (buf[1] != 1) {
			es.push_record(
				severity::critical,
				context{offset_type{0}, Index},
				"Vector component should have dimension one."
			);
			return boost::none;
		}

		auto size = *(uint32_t*)(buf + 2);
		if (is.flip_integers()) {
			s = cc::bswap(s);
		}
		if (s != Size) {
			es.push_record(
				severity::critical,
				context{offset_type{0}, Index},
				"Mismatching vector sizes."
			);
			return boost::none;
		}
		return std::make_tuple(buf + 6, rem_buf_size - 6);
	}
};

template <
	class Scalar,
	std::size_t Rows,
	std::size_t Cols,
	storage_order Order
>
struct verify_component<matrix<Scalar, Rows, Cols, Order>>
{
	static std::tuple<const uint8_t*, std::size_t>
	apply(const uint8_t* p, const std::size_t rem, const bool flip, bool& trans)
	{
		if (rem < 11) {
			throw std::runtime_error{"Unexpected end of file."};
		}
		if (scalar_code<Scalar>::value != p[0]) {
			throw std::runtime_error{"Mismatching scalar type."};
		}
		if (p[1] != 2) {
			throw std::runtime_error{"Expected dimension two."};
		}

		auto o = static_cast<storage_order>(p[2]);
		trans = o != Order;

		auto r = *(uint32_t*)(p + 3);
		auto c = *(uint32_t*)(p + 7);

		if (flip) {
			r = cc::bswap(r);
			c = cc::bswap(c);
		}

		if (r != Rows) {
			throw std::runtime_error{"Mismatching row count."};
		}
		if (c != Cols) {
			throw std::runtime_error{"Mismatching column count."};
		}
		return std::make_tuple(p + 11, rem - 11);
	}
};

/*
** Verifies that each component of the element type described in the header
** matches the corresponding component of the input type.
*/

template <
	std::size_t Current, std::size_t Max,
	std::size_t PassedMatrices, std::size_t Matrices,
	class... Ts
>
struct verify_header_impl;

/*
** This is the specialization in the case that the current type `T` is not a
** matrix. In this case, we do not have to pass a reference to the corresponding
** element of the `trans` array to the helper function.
*/
template <
	std::size_t Current, std::size_t Max,
	std::size_t PassedMatrices, std::size_t Matrices,
	class T, class... Ts
>
struct verify_header_impl<Current, Max, PassedMatrices, Matrices, T, Ts...>
{
	using next = verify_header_impl<
		Current + 1, Max, PassedMatrices, Matrices, Ts...
	>;

	static void
	apply(
		const uint8_t* p,
		const std::size_t hdr_size,
		const bool flip,
		std::array<bool, Matrices>& trans
	)
	{
		auto t = verify_component<T>::apply(p, hdr_size, flip);
		next::apply(std::get<0>(t), std::get<1>(t), flip, trans);
	}
};

/*
** This is the specialization for the case that the current type `T` is a
** matrix. In this case, we do have to pass a reference to the corresponding
** element of the `trans` array to the helper function.
*/
template <
	std::size_t Current, std::size_t Max,
	std::size_t PassedMatrices, std::size_t Matrices,
	class Scalar, std::size_t Rows, std::size_t Cols, storage_order Order,
	class... Ts
>
struct verify_header_impl<
	Current, Max, PassedMatrices, Matrices,
	matrix<Scalar, Rows, Cols, Order>, Ts...
>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using next = verify_header_impl<
		Current + 1, Max, PassedMatrices + 1, Matrices, Ts...
	>;

	static void
	apply(
		const uint8_t* p,
		const std::size_t hdr_size,
		const bool flip,
		std::array<bool, Matrices>& trans
	)
	{
		auto t = verify_component<input_type>::apply(
			p, hdr_size, flip, trans[PassedMatrices]);
		next::apply(std::get<0>(t), std::get<1>(t), flip, trans);
	}
};

template <std::size_t Max, std::size_t Matrices>
struct verify_header_impl<Max, Max, Matrices, Matrices>
{
	static void
	apply(
		const uint8_t*,
		const std::size_t,
		const bool,
		std::array<bool, Matrices>&
	) {}
};

/*
** Verifies that the element type described in the header of the file
** corresponds to the given input type.
*/

template <class InputType, std::size_t Matrices>
struct verify_header
{
	static void
	apply(
		const uint8_t* buf,
		std::size_t buf_size,
		input_state& is,
		error_state& es
	{
		verify_component<0, InputType>(buf, buf_size, is, es);
	}
};

template <
	class Scalar,
	std::size_t Rows,
	std::size_t Cols,
	storage_order Order,
	std::size_t Matrices
>
struct verify_header<matrix<Scalar, Rows, Cols, Order>, Matrices>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using helper = verify_component<0, input_type>;

	static void
	apply(
		const uint8_t* buf,
		std::size_t buf_size,
		input_state& is,
		error_state& es
	)
	{
		helper::apply(buf, buf_size, is, es);
	}
};

template <class... Ts, std::size_t Matrices>
struct verify_header<std::tuple<Ts...>, Matrices>
{
	using helper = verify_header_impl<0, sizeof...(Ts), 0, Matrices, Ts...>;

	static void
	apply(
		const uint8_t* buf,
		std::size_t buf_size,
		input_state& is,
		error_state& es
	)
	{
		helper::apply(buf, buf_size, is, es);
	}
};

template <class SerializedType>
operation_status
read_header(
	uint8_t* buf, size_t n,
	io_state<SerializedType>& is,
	buffer_state& bs, error_state& es
) noexcept
{
	assert(n >= is.header_size());
	bs.consumed(is.header_size());

	if (buf[0] > version) {
		es.push_record(
			severity::critical,
			context{offset_type{0}, 0},
			"Unsupported archive version."
		);
		return operation_status::failure |
			operation_status::fatal_error;
	}

	auto o = static_cast<byte_order>(buf[1]);
	auto io = o & byte_order::integer_mask;
	auto fo = o & byte_order::float_mask;
	is.flip_integers(io != platform_integer_byte_order);
	is.flip_floats(fo != platform_float_byte_order);

	if (buf[2] != element_extents<SerializedType>::value) {
		es.push_record(
			severity::critical,
			context{offset_type{0}, 0},
			"Mismatching element extents."
		);
		return operation_status::failure |
			operation_status::fatal_error;
	}

	/*
	** Refer to `notes/archive_format.md` for more information regarding
	** these values.
	*/
	static constexpr auto component_info_offset = 3;
	static constexpr auto elem_count_size = 8;

	// TODO define the type alias for verify_header
	verify_header::apply(
		buf + component_info_offset, 
		is.header_size() - component_info_offset,
		is, es
	);

	is.element_count(*(uint64_t*)(buf + is.header_size() -
		elem_count_size));
	return operation_status::success;
}

}}

#endif
