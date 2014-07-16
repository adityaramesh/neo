/*
** File Name: mixin_base.hpp
** Author:    Aditya Ramesh
** Date:      07/15/2014
** Contact:   _@adityaramesh.com
**
** The `unary_mixin_base` class can be derived in order to define variadic mixin
** classes whose constructors forward the $i$th argument to the $i$th mixin. As
** the name suggests, the `unary_mixin_base` class assumes that each mixin has
** an unary constructor.
**
** In addition, this file also includes facilities for printing classes derived
** from `unary_mixin_base`. It is assumed that the derived class defines a
** public, printable `name` field, and that each mixin class is itself
** printable.
*/

#ifndef ZD0817A0E_02FA_4D80_8ACB_36173DA25BFC
#define ZD0817A0E_02FA_4D80_8ACB_36173DA25BFC

#include <type_traits>
#include <mpl/sequence.hpp>
#include <ccbase/format.hpp>
#include <ccbase/utility/accessors.hpp>

namespace neo {

class mixin_core_access
{
public:
	template <class Derived>
	static constexpr const char* name()
	{ return Derived::name; }
};

template <class Derived, class... Mixins>
class unary_mixin_base : public Mixins...
{
	using mixins = mpl::sequence<Mixins...>;
	
	template <class T>
	using strip = typename std::remove_reference<
		typename std::remove_cv<T>::type
	>::type;
public:
	explicit unary_mixin_base() noexcept {}

	template <class Arg1, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 1, int
	>::type = 0>
	unary_mixin_base(Arg1&& arg1) noexcept(
		std::is_nothrow_constructible<strip<Arg1>, Arg1>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)} {}

	template <class Arg1, class Arg2, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 2, int
	>::type = 0>
	unary_mixin_base(Arg1&& arg1, Arg2&& arg2) noexcept(
		std::is_nothrow_constructible<strip<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<strip<Arg2>, Arg2>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)} {}

	template <class Arg1, class Arg2, class Arg3, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 3, int
	>::type = 0>
	unary_mixin_base(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3) noexcept(
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
	unary_mixin_base(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)
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
	unary_mixin_base(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)
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
struct print_mixin_base;

template <class T, class... Ts>
struct print_mixin_base<T, Ts...>
{
	template <class Derived, class... Mixins>
	static void apply(
		std::ostream& os,
		const unary_mixin_base<Derived, Mixins...>& r
	)
	{
		os << " * " << static_cast<const T&>(r) << "\n";
		print_mixin_base<Ts...>::apply(os, r);
	}
};

template <class T>
struct print_mixin_base<T>
{
	template <class Derived, class... Mixins>
	static void apply(
		std::ostream& os,
		const unary_mixin_base<Derived, Mixins...>& r
	)
	{
		os << " * " << static_cast<const T&>(r);
	}
};

}

template <class Derived, class... Mixins>
std::ostream& operator<<(
	std::ostream& os,
	const unary_mixin_base<Derived, Mixins...>& r
)
{
	cc::writeln(os, "$:", mixin_core_access::name<Derived>());
	detail::print_mixin_base<Mixins...>::apply(os, r);
	return os;
}

}

#endif
