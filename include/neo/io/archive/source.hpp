/*
** File Name:	source.hpp
** Author:	Aditya Ramesh
** Date:	05/25/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z08BC3843_5747_45EA_A586_D2A51BCF9FED
#define Z08BC3843_5747_45EA_A586_D2A51BCF9FED

#include <array>
#include <cstdint>
#include <ccbase/platform.hpp>
#include <neo/core/block_source_facade.hpp>
#include <neo/utility/archive/common.hpp>

#if !(PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_LITTLE || \
      PLATFORM_INTEGER_BYTE_ORDER == PLATFORM_BYTE_ORDER_BIG)
	#error "Unsupported platform byte order."
#endif

namespace neo {
namespace archive {
namespace detail {

/*
** Verifies that a given component of the element type described in the header
** matches the corresponding component of the input type.
*/

template <class T>
struct verify_component;

template <class Scalar>
struct verify_component
{
	static_assert(std::is_arithmetic<Scalar>::value, "Type must be arithmetic.");

	static std::tuple<const uint8_t*, std::size_t>
	apply(const uint8_t* p, const std::size_t rem, const bool)
	{
		if (rem < 2) {
			throw std::runtime_error{"Unexpected end of file."};
		}
		if (scalar_code<Scalar>::value != p[0]) {
			throw std::runtime_error{"Mismatching scalar type."};
		}
		if (p[1] != 0) {
			throw std::runtime_error{"Expected dimension zero."};
		}
		return std::make_tuple(p + 2, rem - 2);
	}
};

template <class Scalar, std::size_t Size>
struct verify_component<vector<Scalar, Size>>
{
	static std::tuple<const uint8_t*, std::size_t>
	apply(const uint8_t* p, const std::size_t rem, const bool flip)
	{
		if (rem < 6) {
			throw std::runtime_error{"Unexpected end of file."};
		}
		if (scalar_code<Scalar>::value != p[0]) {
			throw std::runtime_error{"Mismatching scalar type."};
		}
		if (p[1] != 1) {
			throw std::runtime_error{"Expected dimension one."};
		}

		auto s = *(uint32_t*)(p + 2);
		if (flip) { s = cc::bswap(s); }
		if (s != Size) {
			throw std::runtime_error{"Mismatching vector size."};
		}
		return std::make_tuple(p + 6, rem - 6);
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
		const uint8_t* p,
		const std::size_t hdr_size,
		const bool flip,
		const std::array<bool, Matrices>&
	)
	{
		verify_component<InputType>(p, hdr_size, flip);
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
	using helper = verify_component<input_type>;

	static void
	apply(
		const uint8_t* p,
		const std::size_t hdr_size,
		const bool flip,
		std::array<bool, Matrices>& trans
	)
	{
		helper::apply(p, hdr_size, flip, trans[0]);
	}
};

template <class... Ts, std::size_t Matrices>
struct verify_header<std::tuple<Ts...>, Matrices>
{
	using helper = verify_header_impl<0, sizeof...(Ts), 0, Matrices, Ts...>;

	static void
	apply(
		const uint8_t* p,
		const std::size_t hdr_size,
		const bool flip,
		std::array<bool, Matrices>& trans
	)
	{
		helper::apply(p, hdr_size, flip, trans);
	}
};

/*
** Performs the actual work to process a component to ensure that it has the
** correct byte order and storage order, in the case of a matrix.
*/

template <class Scalar>
struct process_component
{
	static_assert(std::is_arithmetic<Scalar>::value, "Type must be arithmetic.");
	static constexpr auto is_int = std::is_integral<Scalar>::value;
	static constexpr auto is_float = std::is_floating_point<Scalar>::value;

	static CC_ALWAYS_INLINE Scalar
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats
	)
	{
		if (
			(is_int && flip_ints && sizeof(Scalar) > 1) ||
			(is_float && flip_floats && sizeof(Scalar) > 1)
		) {
			return cc::bswap(*(Scalar*)p);
		}
		else {
			return *(Scalar*)p;
		}
	}
};

template <class Scalar, std::size_t Size>
struct process_component<vector<Scalar, Size>>
{
	using input_type = vector<Scalar, Size>;
	using output_type = mapped_eigen_type<input_type>;
	static constexpr auto is_int = std::is_integral<Scalar>::value;
	static constexpr auto is_float = std::is_floating_point<Scalar>::value;

	static CC_ALWAYS_INLINE output_type
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats
	)
	{
		if (
			(is_int && flip_ints && sizeof(Scalar) > 1) ||
			(is_float && flip_floats && sizeof(Scalar) > 1)
		) {
			auto q = (Scalar*)p;
			for (auto i = std::size_t{0}; i != Size; ++i) {
				*q = cc::bswap(*q);
			}
		}
		return {(Scalar*)p};
	}
};

template <
	class Scalar,
	std::size_t Rows,
	std::size_t Cols,
	storage_order Order
>
struct process_component<matrix<Scalar, Rows, Cols, Order>>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using output_type = mapped_eigen_type<input_type>;
	using transposed_type = Eigen::Map<Eigen::Matrix<
		Scalar, Cols, Rows,
		eigen_storage_order<Order>::value
	>>;

	static constexpr auto is_int = std::is_integral<Scalar>::value;
	static constexpr auto is_float = std::is_floating_point<Scalar>::value;

	static CC_ALWAYS_INLINE output_type
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats,
		const bool trans
	)
	{
		if (
			(is_int && flip_ints && sizeof(Scalar) > 1) ||
			(is_float && flip_floats && sizeof(Scalar) > 1)
		) {
			auto q = (Scalar*)p;
			for (auto i = std::size_t{0}; i != Rows * Cols; ++i) {
				*q = cc::bswap(*q);
			}
		}

		// TODO: The code to perform the transpose could be optimized so
		// that no temporary buffer needs to be allocated.
		auto t1 = transposed_type{(Scalar*)p};
		auto t2 = output_type{(Scalar*)p};
		if (trans) { t2 = t1.transpose().eval(); }
		return t2;
	}
};

/*
** Processes a component of the element type to ensure that it has the correct
** byte order and storage order, in the case of a matrix.
*/

template <
	std::size_t Current, std::size_t Max,
	std::size_t PassedMatrices, std::size_t Matrices,
	class... Ts
>
struct process_element_impl;

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
struct process_element_impl<Current, Max, PassedMatrices, Matrices, T, Ts...>
{
	using input_type = std::tuple<T, Ts...>;
	using output_type = mapped_eigen_type<input_type>;
	using next = process_element_impl<
		Current + 1, Max, PassedMatrices, Matrices, Ts...
	>;

	static CC_ALWAYS_INLINE output_type
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats,
		const std::array<bool, Matrices>& trans
	)
	{
		return std::tuple_cat(
			std::make_tuple(process_component<T>::apply(
				p, flip_ints, flip_floats
			)),
			next::apply(
				p + element_size<T>::value,
				flip_ints, flip_floats, trans
			)
		);
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
struct process_element_impl<
	Current, Max, PassedMatrices, Matrices,
	matrix<Scalar, Rows, Cols, Order>, Ts...
>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using output_type = mapped_eigen_type<std::tuple<input_type, Ts...>>;
	using next = process_element_impl<
		Current + 1, Max, PassedMatrices + 1, Matrices, Ts...
	>;

	static CC_ALWAYS_INLINE output_type
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats,
		const std::array<bool, Matrices>& trans
	)
	{
		return std::tuple_cat(
			std::make_tuple(process_component<input_type>::apply(
				p, flip_ints, flip_floats, trans[PassedMatrices]
			)),
			next::apply(
				p + element_size<input_type>::value,
				flip_ints, flip_floats, trans
			)
		);
	}
};

template <std::size_t Max, std::size_t Matrices>
struct process_element_impl<Max, Max, Matrices, Matrices>
{
	static CC_ALWAYS_INLINE std::tuple<>
	apply(
		const uint8_t*,
		const bool,
		const bool,
		const std::array<bool, Matrices>&
	)
	{
		return std::tuple<>{};
	}
};

/*
** Processes an element so that the corresponding data in the buffer have the
** correct byte order, storage order, and so on.
*/

template <class InputType, std::size_t Matrices>
struct process_element
{
	using output_type = mapped_eigen_type<InputType>;
	using helper = process_component<InputType>;

	static CC_ALWAYS_INLINE output_type
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats,
		const std::array<bool, Matrices>&
	)
	{
		return helper::apply(p, flip_ints, flip_floats);
	}
};

template <
	class Scalar,
	std::size_t Rows,
	std::size_t Cols,
	storage_order Order,
	std::size_t Matrices
>
struct process_element<matrix<Scalar, Rows, Cols, Order>, Matrices>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using output_type = mapped_eigen_type<input_type>;
	using helper = process_component<input_type>;

	static CC_ALWAYS_INLINE output_type
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats,
		const std::array<bool, Matrices>& trans
	)
	{
		return helper::apply(p, flip_ints, flip_floats, trans[0]);
	}
};

template <class... Ts, std::size_t Matrices>
struct process_element<std::tuple<Ts...>, Matrices>
{
	using input_type = std::tuple<Ts...>;
	using output_type = mapped_eigen_type<input_type>;
	using helper = process_element_impl<0, sizeof...(Ts), 0, Matrices, Ts...>;

	static CC_ALWAYS_INLINE output_type
	apply(
		const uint8_t* p,
		const bool flip_ints,
		const bool flip_floats,
		const std::array<bool, Matrices>& trans
	)
	{
		return helper::apply(p, flip_ints, flip_floats, trans);
	}
};

}

template <class InputType>
class source
{
	friend class neo::block_source_core_access;

	using reference = mapped_eigen_type<InputType>;
	using offset_type = int64_t;

	static constexpr auto header_size =
	neo::archive::header_size<InputType>::value;

	static constexpr auto elem_size =
	element_size<InputType>::value;

	static constexpr auto buffer_size =
	header_size < elem_size ? elem_size : header_size;

	static constexpr auto matrices =
	count_matrices<InputType>::value;

	using verify_header =
	detail::verify_header<InputType, matrices>;

	using process_element =
	detail::process_element<InputType, matrices>;

	/*
	** Used to indicate whether the `i`th matrix in each element needs to be
	** transposed after being read in order to accommodate for the storage
	** order of the input type.
	*/
	mutable std::array<bool, matrices> trans;
	// Do we need to flip the byte order of integers?
	mutable bool flip_ints;
	// Do we need to flip the byte order of floats?
	mutable bool flip_floats;

	/*
	** This method parses the header to ensure that the serialized type
	** corresponds to the given input type. It then returns some information
	** that is used by the facade class.
	*/
	template <class BufferType>
	std::tuple<offset_type, std::size_t, offset_type>
	parse_header(BufferType& b) const
	{
		auto t = b.read(0, buffer_size).get();
		if (std::get<1>(t) < 3) {
			throw std::runtime_error{"Unexpected end of file."};
		}

		auto p = std::get<0>(t);
		if (p[0] > archive_version) {
			throw std::runtime_error{"Unsupported archive version."};
		}

		auto o = static_cast<byte_order>(p[1]);
		auto io = integer_byte_order(o);
		auto fo = float_byte_order(o);
		flip_ints = io != platform_integer_byte_order;
		flip_floats = fo != platform_float_byte_order;

		if (p[2] != element_extents<InputType>::value) {
			throw std::runtime_error{"Mismatching element extents."};
		}
		verify_header::apply(p + 3, header_size - 3, flip_ints, trans);
		auto c = static_cast<offset_type>(*(uint64_t*)(p + header_size - 8));

		return std::make_tuple(
			offset_type{header_size},
			std::size_t{elem_size}, c
		);
	}

	template <class Pointer>
	reference make_reference(const Pointer p) const
	{
		return process_element::apply(p, flip_ints, flip_floats, trans);
	}
};

template <class InputType, context_type ContextType, bool UseDirectIO = true>
using forward_source =
forward_block_source_facade<source<InputType>, ContextType, UseDirectIO>;

template <class InputType, context_type ContextType, bool UseDirectIO = true>
using random_source =
random_block_source_facade<source<InputType>, ContextType, UseDirectIO>;

}}

#endif
