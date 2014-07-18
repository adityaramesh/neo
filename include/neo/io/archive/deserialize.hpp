/*
** File Name: deserialize.hpp
** Author:    Aditya Ramesh
** Date:      07/17/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z806F248F_0CBC_4050_AACB_86268BAB6909
#define Z806F248F_0CBC_4050_AACB_86268BAB6909

#include <cassert>
#include <tuple>
#include <type_traits>
#include <neo/core/operation_status.hpp>
#include <neo/io/archive/definitions.hpp>

namespace neo {
namespace archive {
namespace detail {

/*
** Performs the actual work to process a component to ensure that it has the
** correct byte order and storage order, in the case of a matrix.
*/
template <uint8_t Index, uint8_t MatrixIndex, class Scalar>
struct process_component
{
	static_assert(std::is_arithmetic<Scalar>::value,
		"Scalar type must be arithmetic.");

	static constexpr auto is_int = std::is_integral<Scalar>::value;
	static constexpr auto is_float = std::is_floating_point<Scalar>::value;

	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t* buf, io_state<SerializedType>& is, error_state&)
	{
		if (
			(is_int && is.flip_integers() && sizeof(Scalar) > 1) ||
			(is_float && is.flip_floats() && sizeof(Scalar) > 1)
		) {
			std::get<Index>(is.element()) = cc::bswap(*(Scalar*)buf);
		}
		else {
			std::get<Index>(is.element()) = *(Scalar*)buf;
		}
	}
};

template <uint8_t Index, uint8_t MatrixIndex, class Scalar, size_t Size>
struct process_component<Index, MatrixIndex, vector<Scalar, Size>>
{
	using input_type = vector<Scalar, Size>;
	using output_type = mapped_eigen_type<input_type>;
	static constexpr auto is_int = std::is_integral<Scalar>::value;
	static constexpr auto is_float = std::is_floating_point<Scalar>::value;

	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t* buf, io_state<SerializedType>& is, error_state&)
	{
		if (
			(is_int && is.flip_integers() && sizeof(Scalar) > 1) ||
			(is_float && is.flip_floats() && sizeof(Scalar) > 1)
		) {
			auto p = (Scalar*)buf;
			for (auto i = size_t{0}; i != Size; ++i) {
				*p = cc::bswap(*p);
			}
		}
		::new (&std::get<Index>(is.element())) output_type{(Scalar*)buf};
	}
};

template <
	uint8_t Index,
	uint8_t MatrixIndex,
	class Scalar,
	size_t Rows,
	size_t Cols,
	storage_order Order
>
struct process_component<Index, MatrixIndex, matrix<Scalar, Rows, Cols, Order>>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using output_type = mapped_eigen_type<input_type>;
	using transposed_type = Eigen::Map<Eigen::Matrix<
		Scalar, Cols, Rows,
		eigen_storage_order<Order>::value
	>>;

	static constexpr auto is_int = std::is_integral<Scalar>::value;
	static constexpr auto is_float = std::is_floating_point<Scalar>::value;

	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t* buf, io_state<SerializedType>& is, error_state&)
	{
		if (
			(is_int && is.flip_integers() && sizeof(Scalar) > 1) ||
			(is_float && is.flip_floats() && sizeof(Scalar) > 1)
		) {
			auto p = (Scalar*)buf;
			for (auto i = size_t{0}; i != Rows * Cols; ++i) {
				*p = cc::bswap(*p);
			}
		}

		// TODO: The code to perform the transpose could be optimized so
		// that no temporary buffer needs to be allocated.
		if (is.transpose_matrix(MatrixIndex)) {
			auto t = transposed_type{(Scalar*)buf};
			t.transpose().eval();
			::new (&std::get<Index>(is.element())) output_type{t.data()};
		}
		else {
			::new (&std::get<Index>(is.element())) output_type{(Scalar*)buf};
		}

	}
};

/*
** Processes a component of the element type to ensure that it has the correct
** byte order (and storage order, in the case of a matrix).
*/
template <
	size_t Current, size_t Max,
	size_t PassedMatrices, size_t Matrices,
	class... Ts
>
struct process_element_impl;

/*
** This is the specialization in the case that the current type `T` is not a
** matrix. In this case, we do not have to pass a reference to the corresponding
** element of the `trans` array to the helper function.
*/
template <
	size_t Current, size_t Max,
	size_t PassedMatrices, size_t Matrices,
	class T, class... Ts
>
struct process_element_impl<Current, Max, PassedMatrices, Matrices, T, Ts...>
{
	using input_type = std::tuple<T, Ts...>;
	using output_type = mapped_eigen_type<input_type>;
	using helper = process_component<Current, PassedMatrices, T>;
	using next = process_element_impl<
		Current + 1, Max, PassedMatrices, Matrices, Ts...
	>;

	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t* buf, io_state<SerializedType>& is, error_state& es)
	{
		helper::apply(buf, is, es);
		next::apply(buf + element_size<T>::value, is, es);
	}
};

/*
** This is the specialization for the case that the current type `T` is a
** matrix. In this case, we do have to pass a reference to the corresponding
** element of the `trans` array to the helper function.
*/
template <
	size_t Current, size_t Max,
	size_t PassedMatrices, size_t Matrices,
	class Scalar, size_t Rows, size_t Cols, storage_order Order,
	class... Ts
>
struct process_element_impl<
	Current, Max, PassedMatrices, Matrices,
	matrix<Scalar, Rows, Cols, Order>, Ts...
>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using output_type = mapped_eigen_type<std::tuple<input_type, Ts...>>;
	using helper = process_component<Current, PassedMatrices, input_type>;
	using next = process_element_impl<
		Current + 1, Max, PassedMatrices + 1, Matrices, Ts...
	>;

	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t* buf, io_state<SerializedType>& is, error_state& es)
	{
		helper::apply(buf, is, es);
		next::apply(buf + element_size<input_type>::value, is, es);
	}
};

template <size_t Max, size_t Matrices>
struct process_element_impl<Max, Max, Matrices, Matrices>
{
	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t*, io_state<SerializedType>&, error_state&) {}
};

/*
** Processes an element so that the corresponding data in the buffer have the
** correct byte order, storage order, and so on.
*/
template <class InputType, size_t Matrices>
struct process_element
{
	using output_type = mapped_eigen_type<InputType>;
	using helper = process_component<0, 0, InputType>;

	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t* buf, io_state<SerializedType>& is, error_state& es)
	{ helper::apply(buf, is, es); }
};

template <class... Ts, size_t Matrices>
struct process_element<std::tuple<Ts...>, Matrices>
{
	using input_type = std::tuple<Ts...>;
	using output_type = mapped_eigen_type<input_type>;
	using helper = process_element_impl<0, sizeof...(Ts), 0, Matrices, Ts...>;

	template <class SerializedType>
	static CC_ALWAYS_INLINE void
	apply(uint8_t* buf, io_state<SerializedType>& is, error_state& es)
	{ helper::apply(buf, is, es); }
};

}

template <class SerializedType>
operation_status
deserialize(
	uint8_t* buf, size_t n,
	io_state<SerializedType>& is,
	buffer_state& bs, error_state& es
) noexcept
{
	(void)n;
	cc::println("$ $", is.element_size(), n);
	assert(n >= is.element_size());
	bs.consumed(is.element_size());

	static constexpr auto matrices =
	count_matrices<SerializedType>::value;

	using helper =
	detail::process_element<SerializedType, matrices>;

	helper::apply(buf, is, es);
	return operation_status::success;
}

}}

#endif
