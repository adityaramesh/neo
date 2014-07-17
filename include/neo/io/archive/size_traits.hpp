/*
** File Name:	size_traits.hpp
** Author:	Aditya Ramesh
** Date:	05/23/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z73ADF2DA_1347_43DA_ADED_82F2B6F129DF
#define Z73ADF2DA_1347_43DA_ADED_82F2B6F129DF

#include <tuple>
#include <neo/io/archive/tensor.hpp>

namespace neo {
namespace archive {

/*
** Measures the size of the header for the archive file with the given type.
*/

template <class... Ts>
struct header_size;

template <class Scalar>
struct header_size<Scalar>
{
	static constexpr auto value = 2;
};

template <class Scalar, size_t Size>
struct header_size<vector<Scalar, Size>>
{
	// scalar type + dimensions + storage order + extents
	static constexpr auto value = 2 + 1 * 4;
};

template <
	class Scalar,
	size_t Rows,
	size_t Cols,
	storage_order Order
>
struct header_size<matrix<Scalar, Rows, Cols, Order>>
{
	// scalar type + dimensions + storage order + extents
	static constexpr auto value = 3 + 2 * 4;
};

template <class T, class... Ts>
struct header_size<T, Ts...>
{
	static constexpr auto value =
	header_size<T>::value + header_size<Ts...>::value;
};

template <class... Ts>
struct header_size<std::tuple<Ts...>>
{
	static constexpr auto value =
	11 + header_size<Ts...>::value;
};

/*
** Measures the size of a single element of the given type.
*/

template <class... Ts>
struct element_size;

template <class Scalar>
struct element_size<Scalar>
{
	static constexpr auto value = sizeof(Scalar);
};

template <class Scalar, size_t Size>
struct element_size<vector<Scalar, Size>>
{
	static constexpr auto value = Size * sizeof(Scalar);
};

template <
	class Scalar,
	size_t Rows,
	size_t Cols,
	storage_order Order
>
struct element_size<matrix<Scalar, Rows, Cols, Order>>
{
	static constexpr auto value = Rows * Cols * sizeof(Scalar);
};

template <class T, class... Ts>
struct element_size<T, Ts...>
{
	static constexpr auto value =
	element_size<T>::value + element_size<Ts...>::value;
};

template <class... Ts>
struct element_size<std::tuple<Ts...>>
{
	static constexpr auto value =
	element_size<Ts...>::value;
};

/*
** Measures the extents of a single element of the given type.
*/

template <class T>
struct element_extents
{
	static constexpr auto value = 1;
};

template <class... Ts>
struct element_extents<std::tuple<Ts...>>
{
	static constexpr auto value = sizeof...(Ts);
};

}}

#endif
