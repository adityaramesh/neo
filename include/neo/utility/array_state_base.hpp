/*
** File Name: array_state_base.hpp
** Author:    Aditya Ramesh
** Date:      07/15/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z4D2FDD59_CD00_4768_9BFE_CAF4E3FB2EA2
#define Z4D2FDD59_CD00_4768_9BFE_CAF4E3FB2EA2

#include <cstdint>
#include <ccbase/utility.hpp>

namespace neo {

template <class Derived, class SizeType = uint64_t>
class array_state_base
{
	size_t m_hdr_size{};
	SizeType m_elem_count{};
	SizeType m_cur_elem{};
public:
	explicit array_state_base() noexcept {}

	DEFINE_COPY_GETTER_SETTER(array_state_base, m_hdr_size, header_size)
	DEFINE_COPY_GETTER_SETTER(array_state_base, m_elem_count, element_count)
	DEFINE_COPY_GETTER_SETTER(array_state_base, m_cur_elem, current_element)
};

}

#endif
