/*
** File Name: log_record.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z760EC79C_8B72_433C_861B_0944D751DFFB
#define Z760EC79C_8B72_433C_861B_0944D751DFFB

#include <string>
#include <type_traits>
#include <neo/core/severity.hpp>
#include <mpl/sequence.hpp>
#include <ccbase/utility/accessors.hpp>

namespace neo {

template <class... Mixins>
class log_record : public Mixins...
{
	using mixins = mpl::sequence<Mixins...>;
	
	template <class T>
	using strip = typename std::remove_reference<
		typename std::remove_cv<T>::type
	>::type;
public:
	log_record() noexcept {}

	template <class Arg1, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 1, int
	>::type = 0>
	log_record(Arg1&& arg1) noexcept(
		std::is_nothrow_constructible<strip<Arg1>, Arg1>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)} {}

	template <class Arg1, class Arg2, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 2, int
	>::type = 0>
	log_record(Arg1&& arg1, Arg2&& arg2) noexcept(
		std::is_nothrow_constructible<strip<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<strip<Arg2>, Arg2>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)} {}

	template <class Arg1, class Arg2, class Arg3, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 3, int
	>::type = 0>
	log_record(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3) noexcept(
		std::is_nothrow_constructible<strip<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<strip<Arg2>, Arg2>::value &&
		std::is_nothrow_constructible<strip<Arg3>, Arg3>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)},
	    mpl::at<2, mixins>{std::forward<Arg3>(arg3)} {}

	template <class Arg1, class Arg2, class Arg3, class Arg4,
	typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 4, int
	>::type = 0>
	log_record(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)
	noexcept(
		std::is_nothrow_constructible<strip<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<strip<Arg2>, Arg2>::value &&
		std::is_nothrow_constructible<strip<Arg3>, Arg3>::value &&
		std::is_nothrow_constructible<strip<Arg4>, Arg4>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)},
	    mpl::at<2, mixins>{std::forward<Arg3>(arg3)},
	    mpl::at<3, mixins>{std::forward<Arg4>(arg4)} {}

	template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5,
	typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 5, int
	>::type = 0>
	log_record(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)
	noexcept(
		std::is_nothrow_constructible<strip<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<strip<Arg2>, Arg2>::value &&
		std::is_nothrow_constructible<strip<Arg3>, Arg3>::value &&
		std::is_nothrow_constructible<strip<Arg4>, Arg4>::value &&
		std::is_nothrow_constructible<strip<Arg5>, Arg5>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)},
	    mpl::at<2, mixins>{std::forward<Arg3>(arg3)},
	    mpl::at<3, mixins>{std::forward<Arg4>(arg4)},
	    mpl::at<4, mixins>{std::forward<Arg5>(arg5)} {}
};

namespace detail {

template <class... Ts>
struct print_log_record;

template <class T, class... Ts>
struct print_log_record<T, Ts...>
{
	template <class... Mixins>
	static void apply(std::ostream& os, const log_record<Mixins...>& r)
	{
		os << " * " << static_cast<const T&>(r) << "\n";
		print_log_record<Ts...>::apply(os, r);
	}
};

template <class T>
struct print_log_record<T>
{
	template <class... Mixins>
	static void apply(std::ostream& os, const log_record<Mixins...>& r)
	{
		os << " * " << static_cast<const T&>(r);
	}
};

}

template <class... Mixins>
std::ostream& operator<<(std::ostream& os, const log_record<Mixins...>& r)
{
	os << "Log record:\n";
	detail::print_log_record<Mixins...>::apply(os, r);
	return os;
}

class with_message
{
	std::string m_msg{};
public:
	with_message() {}

	template <class T>
	with_message(T&& msg) noexcept
	: m_msg{std::forward<T>(msg)} {}

	DEFINE_REF_GETTER_SETTER(with_message, message, m_msg)
};

std::ostream& operator<<(std::ostream& os, const with_message& m)
{ return os << "Message: " << m.message(); }

class with_severity
{
	severity m_sev{};
public:
	with_severity() {}
	
	template <class T>
	with_severity(T&& sev) noexcept
	: m_sev{std::forward<T>(sev)} {}

	DEFINE_COPY_GETTER_SETTER(with_severity, severity, m_sev)
};

std::ostream& operator<<(std::ostream& os, const with_severity& s)
{ return os << "Severity: " << s.severity(); }

template <class Code>
class with_code
{
	Code m_code;
public:
	with_code() {}

	template <class T>
	with_code(T&& code) noexcept
	: m_code{std::forward<T>(code)} {}

	DEFINE_COPY_GETTER_SETTER(with_code, code, m_code)
};

template <class Code>
std::ostream& operator<<(std::ostream& os, const with_code<Code>& s)
{ return os << "Code: " << s.code(); }

template <class Context>
class with_context
{
	Context m_ctx;
public:
	with_context() {}

	template <class T>
	with_context(T&& ctx) noexcept
	: m_ctx{std::forward<T>(ctx)} {}

	DEFINE_REF_GETTER_SETTER(with_context, context, m_ctx)
};

template <class Context>
std::ostream& operator<<(std::ostream& os, const with_context<Context>& s)
{ return os << "Context: " << s.context(); }

}

#endif
