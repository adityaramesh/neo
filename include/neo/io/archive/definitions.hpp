/*
** File Name: definitions.hpp
** Author:    Aditya Ramesh
** Date:      07/16/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z1FFDB7DB_502A_4714_A127_B431A08D7837
#define Z1FFDB7DB_502A_4714_A127_B431A08D7837

#include <cstdint>
#include <neo/core/basic_context.hpp>
#include <neo/core/basic_log_record.hpp>
#include <neo/core/basic_error_state.hpp>
#include <neo/core/buffer_state.hpp>
#include <neo/io/archive/byte_order.hpp>
#include <neo/io/archive/eigen_traits.hpp>
#include <neo/io/archive/size_traits.hpp>
#include <neo/utility/array_state_base.hpp>
#include <ccbase/utility.hpp>

namespace neo {
namespace archive {

static constexpr auto version = 1;

using offset_type = uint_fast64_t;

using context = basic_context<
	with_element<offset_type>, with_component<uint8_t>
>;

using log_record = basic_log_record<
	with_severity, with_context<context>, with_message
>;

using error_state = basic_error_state<log_record>;

template <class SerializedType>
buffer_state make_buffer_state() noexcept
{
	static constexpr auto hdr_size = element_size<SerializedType>::value;
	auto b = buffer_state{};
	b.required_constraints().at_least(hdr_size);
	return b;
}

template <class SerializedType>
class io_state
{
public:
	using value_type = eigen_type<SerializedType>;
private:
	static constexpr auto hdr_size  = header_size<SerializedType>::value;
	static constexpr auto elem_size = element_size<SerializedType>::value;
	static constexpr auto matrices  = count_matrices<SerializedType>::value;

	value_type m_val{};

	/*
	** Used to indicate whether the $i$th matrix in each element needs to be
	** transposed after being read in order to accommodate for the storage
	** order of the input type.
	*/
	std::array<bool, matrices> m_trans;
	// Do we need to flip the byte order of integers?
	bool m_flip_ints;
	// Do we need to flip the byte order of floats?
	bool m_flip_floats;
public:
	explicit io_state() noexcept {}
	size_t header_size() const { return hdr_size; }
	size_t element_size() const { return elem_size; }

	bool transpose_matrix(size_t n)
	const noexcept { return m_trans[n]; }

	io_state transpose_matrix(size_t n, bool t)
	noexcept
	{
		m_trans[n] = t;
		return *this;
	}

	DEFINE_REF_GETTER_SETTER(io_state, element, m_val)
	DEFINE_COPY_GETTER_SETTER(io_state, flip_integers, m_flip_ints)
	DEFINE_COPY_GETTER_SETTER(io_state, flip_floats, m_flip_floats)
};

}}

#endif
