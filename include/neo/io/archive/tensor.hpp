/*
** File Name:	tensor.hpp
** Author:	Aditya Ramesh
** Date:	05/23/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z752D0158_D6B5_4523_8902_01B0391DF330
#define Z752D0158_D6B5_4523_8902_01B0391DF330

#include <cstdint>
#include <limits>
#include <neo/io/archive/storage_order.hpp>

namespace neo {
namespace archive {

/*
** The scalar code is used to encode the type of the scalar in the header.
*/

template <class Scalar>
struct scalar_code;

template <> struct scalar_code<int8_t>   { static constexpr uint8_t value = 0; };
template <> struct scalar_code<int16_t>  { static constexpr uint8_t value = 1; };
template <> struct scalar_code<int32_t>  { static constexpr uint8_t value = 2; };
template <> struct scalar_code<int64_t>  { static constexpr uint8_t value = 3; };
template <> struct scalar_code<uint8_t>  { static constexpr uint8_t value = 4; };
template <> struct scalar_code<uint16_t> { static constexpr uint8_t value = 5; };
template <> struct scalar_code<uint32_t> { static constexpr uint8_t value = 6; };
template <> struct scalar_code<uint64_t> { static constexpr uint8_t value = 7; };
template <> struct scalar_code<float>    { static constexpr uint8_t value = 8; };
template <> struct scalar_code<double>   { static constexpr uint8_t value = 9; };

static constexpr auto dynamic = std::numeric_limits<std::size_t>::max();

template <class Scalar, std::size_t Size = dynamic>
struct vector {};

template <
	class Scalar,
	std::size_t Rows = dynamic,
	std::size_t Cols = dynamic,
	storage_order Order = storage_order::column_major
>
struct matrix {};

template <class T>
struct is_matrix
{
	static constexpr auto value = false;
};

template <
	class Scalar,
	std::size_t Rows,
	std::size_t Cols,
	storage_order Order
>
struct is_matrix<matrix<Scalar, Rows, Cols, Order>>
{
	static constexpr auto value = true;
};

template <class T>
struct count_matrices
{
	static constexpr auto value = is_matrix<T>::value;
};

template <class T, class... Ts>
struct count_matrices<std::tuple<T, Ts...>>
{
	static constexpr auto value = is_matrix<T>::value +
		count_matrices<std::tuple<Ts...>>::value;
};

template <class T>
struct count_matrices<std::tuple<T>>
{
	static constexpr auto value = is_matrix<T>::value;
};

}}

#endif
