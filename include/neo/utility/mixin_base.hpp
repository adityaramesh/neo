/*
** File Name: mixin_base.hpp
** Author:    Aditya Ramesh
** Date:      07/15/2014
** Contact:   _@adityaramesh.com
**
** The `mixin_base` class can be derived in order to define variadic mixin
** classes whose constructors forward the $i$th argument to the $i$th mixin. As
** the name suggests, the `mixin_base` class assumes that each mixin has an
** unary constructor.
**
** In addition, this file also includes facilities for printing classes derived
** from `mixin_base`. It is assumed that the derived class defines a public,
** printable `name` field, and that each mixin class is itself printable.
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
	template <class T>
	static constexpr const char* name()
	{ return T::name(); }
};

template <class Derived, class... Mixins>
class mixin_base : public Mixins...
{
	using mixins = mpl::sequence<Mixins...>;
	
	template <class T>
	using decay = typename std::decay<T>::type;
public:
	explicit mixin_base() noexcept {}

	template <class Arg1, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 1, int
	>::type = 0>
	mixin_base(Arg1&& arg1) noexcept(
		std::is_nothrow_constructible<decay<Arg1>, Arg1>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)} {}

	template <class Arg1, class Arg2, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 2, int
	>::type = 0>
	mixin_base(Arg1&& arg1, Arg2&& arg2) noexcept(
		std::is_nothrow_constructible<decay<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<decay<Arg2>, Arg2>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)} {}

	template <class Arg1, class Arg2, class Arg3, typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 3, int
	>::type = 0>
	mixin_base(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3) noexcept(
		std::is_nothrow_constructible<decay<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<decay<Arg2>, Arg2>::value &&
		std::is_nothrow_constructible<decay<Arg3>, Arg3>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)},
	    mpl::at<2, mixins>{std::forward<Arg3>(arg3)} {}

	template <class Arg1, class Arg2, class Arg3, class Arg4,
	typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 4, int
	>::type = 0>
	mixin_base(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)
	noexcept(
		std::is_nothrow_constructible<decay<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<decay<Arg2>, Arg2>::value &&
		std::is_nothrow_constructible<decay<Arg3>, Arg3>::value &&
		std::is_nothrow_constructible<decay<Arg4>, Arg4>::value
	) : mpl::at<0, mixins>{std::forward<Arg1>(arg1)},
	    mpl::at<1, mixins>{std::forward<Arg2>(arg2)},
	    mpl::at<2, mixins>{std::forward<Arg3>(arg3)},
	    mpl::at<3, mixins>{std::forward<Arg4>(arg4)} {}

	template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5,
	typename std::enable_if<
		std::is_same<Arg1, Arg1>::value && sizeof...(Mixins) == 5, int
	>::type = 0>
	mixin_base(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)
	noexcept(
		std::is_nothrow_constructible<decay<Arg1>, Arg1>::value &&
		std::is_nothrow_constructible<decay<Arg2>, Arg2>::value &&
		std::is_nothrow_constructible<decay<Arg3>, Arg3>::value &&
		std::is_nothrow_constructible<decay<Arg4>, Arg4>::value &&
		std::is_nothrow_constructible<decay<Arg5>, Arg5>::value
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
		const mixin_base<Derived, Mixins...>& r
	)
	{
		cc::write(os, "\"$\": ", mixin_core_access::name<T>());
		print_data(os, static_cast<const T&>(r));
		cc::write(os, ", ");
		print_mixin_base<Ts...>::apply(os, r);
	}
};

template <class T>
struct print_mixin_base<T>
{
	template <class Derived, class... Mixins>
	static void apply(
		std::ostream& os,
		const mixin_base<Derived, Mixins...>& r
	)
	{
		cc::write(os, "\"$\": ", mixin_core_access::name<T>());
		print_data(os, static_cast<const T&>(r));
		cc::write(os, "}}");
	}
};

}

template <class Derived, class... Mixins>
std::ostream& operator<<(
	std::ostream& os,
	const mixin_base<Derived, Mixins...>& r
)
{
	cc::write(os, "\"$\": {{", mixin_core_access::name<Derived>());
	detail::print_mixin_base<Mixins...>::apply(os, r);
	return os;
}

}

#endif
