/*
** File Name: basic_context.hpp
** Author:    Aditya Ramesh
** Date:      07/15/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZE5AAD833_5B78_42F0_BC8A_3AB46B63E946
#define ZE5AAD833_5B78_42F0_BC8A_3AB46B63E946

#include <neo/utility/mixin_base.hpp>

namespace neo {

template <class... Mixins>
class basic_context : public unary_mixin_base<
	basic_context<Mixins...>, Mixins...
>
{
	using base =
	unary_mixin_base<basic_context<Mixins...>, Mixins...>;

	friend class mixin_core_access;
	constexpr static auto name = "Context";
public:
	explicit basic_context() noexcept {}

	template <class... Args>
	explicit basic_context(Args&&... args) :
	base{std::forward<Args>(args)...} {}
};

template <class... Mixins>
constexpr const char* basic_context<Mixins...>::name;

template <class SizeType>
class with_line
{
	SizeType m_line;
public:
	explicit with_line() noexcept {}

	explicit with_line(SizeType line) noexcept
	: m_line{line} {}

	DEFINE_COPY_GETTER_SETTER(with_line, line, m_line)
};

template <class SizeType>
std::ostream& operator<<(std::ostream& os, const with_line<SizeType>& s)
{ return os << "Line: " << s.line(); }

template <class SizeType>
class with_column
{
	SizeType m_col;
public:
	explicit with_column() noexcept {}

	explicit with_column(SizeType column) noexcept
	: m_col{column} {}

	DEFINE_COPY_GETTER_SETTER(with_column, column, m_col)
};

template <class SizeType>
std::ostream& operator<<(std::ostream& os, const with_column<SizeType>& s)
{ return os << "Column: " << s.column(); }

template <class SizeType>
class with_element
{
	SizeType m_elem;
public:
	explicit with_element() noexcept {}

	explicit with_element(SizeType element) noexcept
	: m_elem{element} {}

	DEFINE_COPY_GETTER_SETTER(with_element, element, m_elem)
};

template <class SizeType>
std::ostream& operator<<(std::ostream& os, const with_element<SizeType>& s)
{ return os << "Element: " << s.element(); }

template <class SizeType>
class with_component
{
	SizeType m_comp;
public:
	explicit with_component() noexcept {}

	explicit with_component(SizeType component) noexcept
	: m_comp{component} {}

	DEFINE_COPY_GETTER_SETTER(with_component, component, m_comp)
};

template <class SizeType>
std::ostream& operator<<(std::ostream& os, const with_component<SizeType>& s)
{ return os << "Component: " << s.component(); }

class with_offset
{
	off_t m_off;
public:
	explicit with_offset() noexcept {}

	explicit with_offset(off_t offset) noexcept
	: m_off{offset} {}

	DEFINE_COPY_GETTER_SETTER(with_offset, offset, m_off)
};

std::ostream& operator<<(std::ostream& os, const with_offset& s)
{ return os << "Offset: " << s.offset(); }

}

#endif
