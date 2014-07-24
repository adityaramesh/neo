/*
** File Name: basic_log_record.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
**
** This file defines a log record class that uses variadic mixins to allow for
** configurable functionality. Each mixin is assumed to have a default
** constructor and an unary constructor. By default, the following mixins are
** provided:
**
**   - `with_message`: Associates the record with a message string.
**   - `with_severity`: Associates the record with a `severity` level defined in
**   `severity.hpp`.
**   - `with_code`: Associates the record with a status code. This is useful if
**   the program needs to respond to errors based on their types.
**   - `with_context`: Associates the record with a specific location in the
**   file. For CSV files, this could be the line and column numbers.
**
** The log record class is printable, and as such, all of its mixins must also
** be printable.
*/

#ifndef Z760EC79C_8B72_433C_861B_0944D751DFFB
#define Z760EC79C_8B72_433C_861B_0944D751DFFB

#include <string>
#include <neo/core/severity.hpp>
#include <neo/utility/mixin_base.hpp>

namespace neo {

template <class... Mixins>
class basic_log_record : public mixin_base<
	basic_log_record<Mixins...>, Mixins...
>
{
	using base =
	mixin_base<basic_log_record<Mixins...>, Mixins...>;

	friend class mixin_core_access;
public:
	explicit basic_log_record() noexcept {}

	template <class... Args>
	explicit basic_log_record(Args&&... args) :
	base{std::forward<Args>(args)...} {}

	static constexpr const char* name() { return "log record"; }
};

class with_message
{
	std::string m_msg{};

	friend class mixin_core_access;
public:
	explicit with_message() noexcept {}

	template <class T>
	explicit with_message(T&& msg) noexcept
	: m_msg{std::forward<T>(msg)} {}

	static constexpr const char* name() { return "message"; }

	DEFINE_REF_GETTER_SETTER(with_message, message, m_msg)
};

void print_data(std::ostream& os, const with_message& m)
{ os << "\"" << m.message() << "\""; }

class with_severity
{
	severity m_sev{};

	friend class mixin_core_access;
public:
	explicit with_severity() noexcept {}
	
	explicit with_severity(severity sev) noexcept
	: m_sev{sev} {}

	static constexpr const char* name() { return "severity"; }

	DEFINE_COPY_GETTER_SETTER(with_severity, severity, m_sev)
};

void print_data(std::ostream& os, const with_severity& s)
{ os << s.severity(); }

template <class Code>
class with_code
{
	Code m_code;

	friend class mixin_core_access;
public:
	explicit with_code() noexcept {}

	explicit with_code(Code code) noexcept
	: m_code{code} {}

	static constexpr const char* name() { return "code"; }

	DEFINE_COPY_GETTER_SETTER(with_code, code, m_code)
};

template <class Code>
void print_data(std::ostream& os, const with_code<Code>& s)
{ os << s.code(); }

template <class Context>
class with_context
{
	Context m_ctx;

	friend class mixin_core_access;
public:
	explicit with_context() noexcept {}

	template <class T>
	explicit with_context(T&& ctx) noexcept
	: m_ctx{std::forward<T>(ctx)} {}

	static constexpr const char* name() { return "context"; }

	DEFINE_REF_GETTER_SETTER(with_context, context, m_ctx)
};

template <class Context>
void print_data(std::ostream& os, const with_context<Context>& s)
{ os << s.context(); }

}

#endif
