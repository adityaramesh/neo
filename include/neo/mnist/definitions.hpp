/*
** File Name: definitions.hpp
** Author:    Aditya Ramesh
** Date:      07/15/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z7C5C3505_051A_480E_B6EA_684915748EE8
#define Z7C5C3505_051A_480E_B6EA_684915748EE8

#include <cstdint>
#include <neo/core/log_record.hpp>
#include <neo/core/error_state.hpp>
#include <neo/utility/array_state_base.hpp>
#include <ccbase/utility.hpp>
#include <Eigen/Core>

namespace neo {
namespace mnist {

using log_record = basic_log_record<with_severity, with_context, with_message>; 

class image_io_state : public array_state_base<
	image_io_state,
	Eigen::Map<Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor, 28, 28>>,
	uint_fast32_t
>
{
	using reference =
	Eigen::Map<Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor, 28, 28>>;

	reference m_ref{nullptr};
public:
	explicit image_io_state() noexcept {}
	size_t element_size() const { return 28 * 28; }
	DEFINE_REF_GETTER_SETTER(image_io_state, m_ref, element)
};

class label_io_state : public array_state_base<
	label_io_state, uint8_t, uint_fast32_t
>
{
	uint8_t m_val{};
public:
	explicit label_io_state() noexcept {}
	size_t element_size() const { return 1; }
	DEFINE_COPY_GETTER_SETTER(label_io_state, m_val, element)
};

}}

#endif
