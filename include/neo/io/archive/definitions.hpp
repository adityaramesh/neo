/*
** File Name: definitions.hpp
** Author:    Aditya Ramesh
** Date:      07/16/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z1FFDB7DB_502A_4714_A127_B431A08D7837
#define Z1FFDB7DB_502A_4714_A127_B431A08D7837

#include <neo/utility/archive/byte_order.hpp>
#include <neo/utility/archive/eigen_traits.hpp>
#include <neo/utility/archive/size_traits.hpp>

namespace neo {
namespace archive {

static constexpr auto version = 1;

using offset_type = uint_fast64_t;
using context = basic_context<with_element<offset_type>>;
using log_record = basic_log_record<with_severity, with_context<context>, with_message>; 
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
	static constexpr auto hdr_size = header_size<SerializedType>::value;
	static constexpr auto elem_size = element_size<SerializedType>::value;
	value_type m_val{};
public:
	explicit io_state() noexcept {}
	constexpr size_t header_size() const { return hdr_size; }
	constexpr size_t element_size() const { return img_size; }
	DEFINE_REF_GETTER_SETTER(io_state, element, m_val)
};

}}

#endif
