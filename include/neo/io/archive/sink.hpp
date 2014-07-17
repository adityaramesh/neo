/*
** File Name:	sink.hpp
** Author:	Aditya Ramesh
** Date:	05/21/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z32CB466E_2059_4F45_A857_B8A5B2A52C3B
#define Z32CB466E_2059_4F45_A857_B8A5B2A52C3B

#include <cstdint>
#include <neo/core/buffer_size.hpp>
#include <neo/core/forward_sink.hpp>
#include <neo/core/forward_write_buffer.hpp>
#include <neo/utility/archive/common.hpp>

namespace neo {
namespace archive {
namespace detail {

/*
** Helper class that writes the header of the file.
*/

template <class... Ts>
struct write_header;

template <class Scalar>
struct write_header<Scalar>
{
	static uint8_t* apply(uint8_t* p)
	{
		p[0] = scalar_code<Scalar>::value;
		p[1] = 0;
		return p + 2;
	}
};

template <class Scalar, std::size_t Size>
struct write_header<vector<Scalar, Size>>
{
	static uint8_t* apply(uint8_t* p)
	{
		p[0] = scalar_code<Scalar>::value;
		p[1] = 1;
		*(uint32_t*)(p + 2) = Size;
		return p + 6;
	}
};

template <
	class Scalar,
	std::size_t Rows,
	std::size_t Cols,
	storage_order Order
>
struct write_header<matrix<Scalar, Rows, Cols, Order>>
{
	static uint8_t* apply(uint8_t* p)
	{
		p[0] = scalar_code<Scalar>::value;
		p[1] = 2;
		p[2] = static_cast<uint8_t>(Order);
		*(uint32_t*)(p + 3) = Rows;
		*(uint32_t*)(p + 7) = Cols;
		return p + 11;
	}
};

template <class T, class... Ts>
struct write_header<T, Ts...>
{
	static void apply(uint8_t* p)
	{
		auto q = write_header<T>::apply(p);
		write_header<Ts...>::apply(q);
	}
};

template <class... Ts>
struct write_header<std::tuple<Ts...>>
{
	static void apply(uint8_t* p)
	{
		p[0] = archive_version;
		p[1] = static_cast<uint8_t>(platform_byte_order);
		p[2] = sizeof...(Ts);
		write_header<Ts...>::apply(p + 3);
	}
};

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

template <class InputType, class Scalar, std::size_t Size>
struct write_component<InputType, vector<Scalar, Size>>
{
	static_assert(InputType::IsVectorAtCompileTime, "Input type must be a vector.");

	using index = typename InputType::Index;
	using output_type = vector<Scalar, Size>;

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
	std::size_t Rows,
	std::size_t Cols,
	storage_order Order
>
struct write_component<InputType, matrix<Scalar, Rows, Cols, Order>>
{
	using output_type = matrix<Scalar, Rows, Cols, Order>;
	using index = typename InputType::Index;

	static constexpr auto can_use_copy =
	is_plain_object<InputType>::value;

	static CC_ALWAYS_INLINE uint8_t*
	apply(const InputType& m, uint8_t* p)
	{
		auto q = (Scalar*)p;

		if (
			can_use_copy &&
			((Order == row_major && m.IsRowMajor) ||
			(Order == column_major && !m.IsRowMajor))
		)
		{
			std::copy(m.data(), m.data() + Rows * Cols, q);
		}
		else if (m.IsRowMajor) {
			if (Order == row_major) {
				for (auto i = index{0}; i != m.rows(); ++i) {
				for (auto j = index{0}; j != m.cols(); ++j) {
					q[m.cols() * i + j] = m(i, j);
				}}
			}
			else {
				/*
				** We need to transpose the matrix while
				** indexing into a buffer that has the opposite
				** storage order.
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
			** traversal is more cache-friendly. The functionality
			** is the same.
			*/
			if (Order == column_major) {
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
	std::size_t Current,
	std::size_t Max,
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

template <std::size_t Max, class InputType, class OutputType>
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

template <class OutputType, context_type ContextType, bool UseDirectIO = true>
class forward_sink :
public neo::forward_sink<forward_sink<OutputType, ContextType, UseDirectIO>, int64_t>
{
public:
	using buffer_type = forward_write_buffer<ContextType, UseDirectIO>;
	using pointer     = typename buffer_type::pointer;
	using offset_type = int64_t;
private:
	using self = forward_sink<OutputType, ContextType, UseDirectIO>;
	using base = neo::forward_sink<self, offset_type>;

	static constexpr auto hdr_size =
	header_size<OutputType>::value;

	static constexpr auto elem_size =
	element_size<OutputType>::value;

	static constexpr auto min_size =
	hdr_size < elem_size ? hdr_size : elem_size;

	mutable buffer_type b;
	offset_type off{0};
public:
	explicit forward_sink(
		const char* path,
		const uint8_t extra_req_count
	) : b{path, extra_req_count, false, at_least{min_size}}
	{
		auto p = b.get(hdr_size).get();
		detail::write_header<OutputType>::apply(p);
		b.double_buffer(true).get();
	}

	void increment() noexcept { ++off; }
	void advance(const offset_type n) noexcept { off += n; }
	decltype(off) offset() const { return off; }

	template <class T>
	cc::expected<void> copy_write(const T& v)
	{
		auto p = b.get(elem_size);
		if (!p.valid()) { return p.exception(); }
		detail::write_element<T, OutputType>::apply(v, p.get());
		++off;
		return true;
	}

	template <class T>
	cc::expected<void> copy_write(const T* p, const std::size_t n)
	{
		/*
		** Keep track of the number of bytes transferred.
		*/
		auto c = std::size_t{};

		/*
		** Copy as much data as possible to the empty part at the end of
		** the active buffer.
		*/
		if (b.offset() != b.size()) {
			auto m = std::min(b.offset() - b.size(), n);
			auto dst = b.get(m);
			if (!dst.valid()) { return dst.exception(); }
			std::copy(p, p + m, dst.get());
			c += m;
		}

		/*
		** Copy chunks of size `b.size()` from the source buffer to the
		** destination buffer.
		*/
		while (n - c > b.size()) {
			auto dst = b.get(b.size());
			if (!dst.valid()) { return dst.exception(); }
			std::copy(p + c, p + c + b.size(), dst.get());
			c += b.size();
		}

		/*
		** Copy the last part of the source buffer to the destination
		** buffer.
		*/
		if (c != n) {
			auto dst = b.get(n - c);
			if (!dst.valid()) { return dst.exception(); }
			std::copy(p + c, p + n, dst.get());
		}
		return true;
	}

	template <class T>
	cc::expected<void> direct_write(const T* p, const std::size_t n)
	{ return b.direct_write(p, n); }

	cc::expected<void> flush()
	{
		/*
		** The number 8 appears because the size of the field used to
		** store the record count is 8 bytes. See
		** `notes/archive_format.md` for more information.
		*/
		auto p = b.get_at(8, hdr_size - 8);
		if (!p.valid()) { return p.exception(); }
		*(uint64_t*)p.get() = off;

		auto r = b.submit_positioned_write();
		if (!r.valid()) { return r.exception(); }
		return b.flush();
	}
};

}}

#endif
