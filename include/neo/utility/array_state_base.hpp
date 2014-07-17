/*
** File Name: array_state_base.hpp
** Author:    Aditya Ramesh
** Date:      07/15/2014
** Contact:   _@adityaramesh.com
**
** This `array_state_base` class is a convenience class that can be derived to
** define the IO state for file formats that consist of a fixed-size header
** followed by a number of serialized elements, all of which have the same size.
*/

#ifndef Z4D2FDD59_CD00_4768_9BFE_CAF4E3FB2EA2
#define Z4D2FDD59_CD00_4768_9BFE_CAF4E3FB2EA2

#include <cstdint>
#include <ccbase/utility.hpp>

namespace neo {

template <class Derived, class OffsetType = uint64_t>
class array_state_base
{
	OffsetType m_elem_count{};
	OffsetType m_cur_elem{};
public:
	explicit array_state_base() noexcept {}

	DEFINE_COPY_GETTER_SETTER(array_state_base, element_count, m_elem_count)
	DEFINE_COPY_GETTER_SETTER(array_state_base, current_element, m_cur_elem)
};

}

#endif
