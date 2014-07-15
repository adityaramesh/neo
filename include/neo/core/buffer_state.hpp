/*
** File Name: buffer_state.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z2C88C0B2_F505_4ABB_9A26_9C1D0007F803
#define Z2C88C0B2_F505_4ABB_9A26_9C1D0007F803

#include <cstdlib>
#include <neo/core/buffer_constraints.hpp>
#include <ccbase/utility/accessors.hpp>

namespace neo {

class buffer_state
{
	buffer_constraints m_req{};
	buffer_constraints m_pref{};
	size_t m_cns{};
public:
	explicit buffer_state() noexcept {}

	DEFINE_COPY_GETTER_SETTER(buffer_state, consumed, m_cns) 
	DEFINE_REF_GETTER(m_req, required_constraints)
	DEFINE_REF_GETTER(m_pref, preferred_constraints)
};

}

#endif
