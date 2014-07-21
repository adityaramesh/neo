/*
** File Name: serialize.hpp
** Author:    Aditya Ramesh
** Date:      07/19/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZFC98EEE7_779B_4116_B23D_512BD6A688FC
#define ZFC98EEE7_779B_4116_B23D_512BD6A688FC

#include <cassert>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <neo/core/operation_status.hpp>
#include <neo/io/archive/definitions.hpp>

namespace neo {
namespace archive {
namespace detail {

/*
** Helper class that writes a single component of an element.
*/
template <class InputType, class OutputType>
struct write_component;

template <class Scalar1, class Scalar2>
struct write_component
{
	static_assert(std::is_arithmetic<Scalar1>::value, "Input type must be scalar.");
	static_assert(std::is_arithmetic<Scalar2>::value, "Output type must be scalar.");

	static CC_ALWAYS_INLINE uint8_t*
	apply(const Scalar1& s, uint8_t* p)
	{
		*(Scalar2*)p = static_cast<Scalar2>(s);
		return p + sizeof(Scalar2);
	}
};

template <class InputType, class Scalar, size_t Size>
struct write_component<InputType, vector<Scalar, Size>>
{
	static_assert(InputType::IsVectorAtCompileTime, "Input type must be a vector.");

	using index = typename InputType::Index;
	using output_type = vector<Scalar, Size>;

	/*
	** This allows us to use `std::copy` instead of a less efficient for
	** loop in the case that the object being written out already exists in
	** memory.
	*/
	static constexpr auto can_use_copy =
	is_plain_object<InputType>::value;

	static CC_ALWAYS_INLINE uint8_t*
	apply(const InputType& v, uint8_t* p)
	{
		auto q = (Scalar*)p;
		if (can_use_copy) {
			std::copy(v.data(), v.data() + Size, q);
		}
		else {
			for (auto i = index{0}; i != v.rows() * v.cols(); ++i) {
				q[i] = v(i);
			}
		}
		return p + sizeof(Scalar) * v.rows() * v.cols();
	}
};

template <
	class InputType,
	class Scalar,
	size_t Rows,
	size_t Cols,
	storage_order Order
>
struct write_component<InputType, matrix<Scalar, Rows, Cols, Order>>
{
	using output_type = matrix<Scalar, Rows, Cols, Order>;
	using index = typename InputType::Index;

	/*
	** This allows us to use `std::copy` instead of a less efficient for
	** loop in the case that the object being written out already exists in
	** memory.
	*/
	static constexpr auto can_use_copy =
	is_plain_object<InputType>::value;

	static CC_ALWAYS_INLINE uint8_t*
	apply(const InputType& m, uint8_t* p)
	{
		auto q = (Scalar*)p;

		if (
			can_use_copy &&
			((Order == storage_order::row_major && m.IsRowMajor) ||
			(Order == storage_order::column_major && !m.IsRowMajor))
		)
		{
			std::copy(m.data(), m.data() + Rows * Cols, q);
		}
		else if (m.IsRowMajor) {
			if (Order == storage_order::row_major) {
				for (auto i = index{0}; i != m.rows(); ++i) {
				for (auto j = index{0}; j != m.cols(); ++j) {
					q[m.cols() * i + j] = m(i, j);
				}}
			}
			else {
				/*
				** We need to transpose the matrix while
				** indexing into a buffer that has the opposite
				** storage order. This is why the indices on the
				** RHS of the assignment are swapped.
				*/
				for (auto i = index{0}; i != m.rows(); ++i) {
				for (auto j = index{0}; j != m.cols(); ++j) {
					q[m.rows() * i + j] = m(j, i);
				}}
			}
		}
		else {
			/*
			** We switch the loop order in this case so that the
			** traversal is more cache-friendly. The result is the
			** same.
			*/
			if (Order == storage_order::column_major) {
				for (auto j = index{0}; j != m.cols(); ++j) {
				for (auto i = index{0}; i != m.rows(); ++i) {
					q[m.cols() * i + j] = m(i, j);
				}}
			}
			else {
				for (auto j = index{0}; j != m.cols(); ++j) {
				for (auto i = index{0}; i != m.rows(); ++i) {
					q[m.rows() * i + j] = m(j, i);
				}}
			}
		}
		return p + sizeof(Scalar) * m.rows() * m.cols();
	}
};

/*
** Helper class that writes an entire element.
*/
template <
	size_t Current,
	size_t Max,
	class InputType,
	class OutputType
>
struct write_element_helper
{
	using input_component =
	typename std::tuple_element<Current, InputType>::type;

	using output_component =
	typename std::tuple_element<Current, OutputType>::type;

	using helper =
	write_component<input_component, output_component>;

	using next =
	write_element_helper<Current + 1, Max, InputType, OutputType>;

	static CC_ALWAYS_INLINE void
	apply(const InputType& v, uint8_t* p)
	{
		auto q = helper::apply(std::get<Current>(v), p);
		return next::apply(v, q);
	}
};

template <size_t Max, class InputType, class OutputType>
struct write_element_helper<Max, Max, InputType, OutputType>
{
	static CC_ALWAYS_INLINE void
	apply(const InputType&, const uint8_t*) {}
};

/*
** Helper class that dispatches the appropriate method to write either an
** element consisting of a single matrix, or one that is a tuple of matrices.
*/
template <class InputType, class OutputType>
struct write_element;

template <class T, class OutputType>
struct write_element
{
	static CC_ALWAYS_INLINE void
	apply(const T& v, uint8_t* p)
	{
		write_component<T, OutputType>::apply(v, p);
	}
};

template <class... Ts, class OutputType>
struct write_element<std::tuple<Ts...>, OutputType>
{
	using input_type = std::tuple<Ts...>;

	static CC_ALWAYS_INLINE void
	apply(const input_type& v, uint8_t* p)
	{
		write_element_helper<
			0, sizeof...(Ts), input_type, OutputType
		>::apply(v, p);
	}
};

}

template <class T, class SerializedType>
operation_status
serialize(
	const T& t, uint8_t* buf, size_t n,
	io_state<SerializedType>& is,
	buffer_state& bs, error_state&
) noexcept
{
	(void)n;
	assert(n >= is.element_size());
	bs.consumed(is.element_size());
	detail::write_element<T, SerializedType>::apply(t, buf);
	return operation_status::success;
}

}}

#endif
