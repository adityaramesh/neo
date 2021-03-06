/*
** File Name: definitions.hpp
** Author:    Aditya Ramesh
** Date:      07/15/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z7C5C3505_051A_480E_B6EA_684915748EE8
#define Z7C5C3505_051A_480E_B6EA_684915748EE8

#include <cstdint>
#include <neo/core/access_mode.hpp>
#include <neo/core/basic_context.hpp>
#include <neo/core/basic_log_record.hpp>
#include <neo/core/basic_error_state.hpp>
#include <neo/core/buffer_state.hpp>
#include <boost/optional.hpp>
#include <ccbase/utility.hpp>
#include <Eigen/Core>

namespace neo {
namespace mnist {

using offset_type = uint_fast32_t;
static constexpr auto img_width = 28;
static constexpr auto img_size = img_width * img_width;

using context = basic_context<with_element<offset_type>>;
using log_record = basic_log_record<with_severity, with_context<context>, with_message>; 
using error_state = basic_error_state<log_record>;

class image_io_state
{
public:
	using value_type =
	Eigen::Map<Eigen::Matrix<uint8_t, img_width, img_width,
		Eigen::RowMajor, img_width, img_width>>;

	static constexpr auto hdr_size = 16;
private:
	value_type m_ref{nullptr};
	boost::optional<offset_type> m_elem_count{};
public:
	explicit image_io_state() noexcept {}
	size_t header_size() const { return hdr_size; }
	size_t element_size() const { return img_size; }
	DEFINE_REF_GETTER_SETTER(image_io_state, element, m_ref)

	offset_type element_count() const
	{
		assert(m_elem_count && "Element count uninitialized.");
		return *m_elem_count;
	}

	image_io_state& element_count(offset_type n)
	{
		m_elem_count = n;
		return *this;
	}
};

class label_io_state
{
	uint8_t m_val{};
	boost::optional<offset_type> m_elem_count{};
public:
	static constexpr auto hdr_size = 8;

	explicit label_io_state() noexcept {}
	size_t header_size() const { return hdr_size; }
	size_t element_size() const { return 1; }
	DEFINE_COPY_GETTER_SETTER(label_io_state, element, m_val)

	offset_type element_count() const
	{
		assert(m_elem_count && "Element count uninitialized.");
		return *m_elem_count;
	}

	label_io_state& element_count(offset_type n)
	{
		m_elem_count = n;
		return *this;
	}
};

buffer_state make_image_buffer_state() noexcept
{
	auto b = buffer_state{};
	b.required_constraints().at_least(img_size);
	b.preferred_constraints().at_least(img_size).multiple_of(img_size);
	return b;
}

buffer_state make_label_buffer_state() noexcept
{
	auto b = buffer_state{};
	b.required_constraints().at_least(label_io_state::hdr_size);
	b.preferred_constraints().at_least(label_io_state::hdr_size);
	return b;
}

}}

#endif
