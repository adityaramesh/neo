/*
** File Name:	eigen_traits.hpp
** Author:	Aditya Ramesh
** Date:	05/23/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z07E9F626_6843_4B92_9468_80CD886A8586
#define Z07E9F626_6843_4B92_9468_80CD886A8586

#include <type_traits>
#include <neo/utility/archive/tensor.hpp>
#include <neo/utility/archive/storage_order.hpp>
#include <Eigen/Core>

namespace neo {
namespace archive {

/*
** Returns the Eigen storage order constant corresponding to a `neo` storage
** order enum.
*/

template <storage_order Order>
struct eigen_storage_order;

template <>
struct eigen_storage_order<storage_order::row_major>
{
	using type = typename std::underlying_type<decltype(Eigen::RowMajor)>::type;
	static constexpr type value = Eigen::RowMajor;
};

template <>
struct eigen_storage_order<storage_order::column_major>
{
	using type = typename std::underlying_type<decltype(Eigen::ColMajor)>::type;
	static constexpr type value = Eigen::ColMajor;
};

/*
** Returns the Eigen type corresponding to one of the following `neo` types:
**   - scalar<class>
**   - vector<class, std::size_t>
**   - matrix<class, std::size_t, std::size_t, storage_order>
*/

template <class InputType>
struct eigen_type_impl;

template <class Scalar, size_t Rows, size_t Cols, storage_order Order>
struct eigen_type_impl<matrix<Scalar, Rows, Cols, Order>>
{
	using type = Eigen::Matrix<
		Scalar, Rows, Cols,
		eigen_storage_order<Order>::value
	>;
};

template <class Scalar, size_t Size>
struct eigen_type_impl<vector<Scalar, Size>>
{
	using type = Eigen::Matrix<Scalar, Size, 1>;
};

template <class Scalar>
struct eigen_type_impl
{
	using type = Scalar;
};

template <class... Ts>
struct eigen_type_impl<std::tuple<Ts...>>
{
	using type = std::tuple<typename eigen_type_impl<Ts>::type...>;
};

template <class T>
using eigen_type = typename eigen_type_impl<T>::type;

/*
** Returns the mapped Eigen type corresponding to one of the following `neo`
** types:
**   - scalar<class>
**   - vector<class, std::size_t>
**   - matrix<class, std::size_t, std::size_t, storage_order>
*/

template <class InputType>
struct mapped_eigen_type_impl;

template <class Scalar, size_t Rows, size_t Cols, storage_order Order>
struct mapped_eigen_type_impl<matrix<Scalar, Rows, Cols, Order>>
{
	using input_type = matrix<Scalar, Rows, Cols, Order>;
	using type = Eigen::Map<eigen_type<input_type>>;
};

template <class Scalar, size_t Size>
struct mapped_eigen_type_impl<vector<Scalar, Size>>
{
	using input_type = vector<Scalar, Size>;
	using type = Eigen::Map<eigen_type<input_type>>;
};

template <class Scalar>
struct mapped_eigen_type_impl
{
	using type = Scalar;
};

template <class... Ts>
struct mapped_eigen_type_impl<std::tuple<Ts...>>
{
	using type = std::tuple<typename mapped_eigen_type_impl<Ts>::type...>;
};

template <class T>
using mapped_eigen_type = typename mapped_eigen_type_impl<T>::type;

/*
** Various useful Eigen traits.
*/

template <class T>
struct is_eigen_matrix
{
	static constexpr auto value =
	std::is_base_of<Eigen::MatrixBase<T>, T>::value;
};

template <class T>
struct is_plain_object
{
	static constexpr auto value =
	std::is_same<T, typename T::PlainObject>::value;
};

template <class T>
struct is_plain_object<Eigen::Map<T>>
{
	static constexpr auto value = true;
};

template <class T>
struct eigen_matrix_traits
{
	static_assert(is_eigen_matrix<T>::value, "Type derive from Eigen::MatrixBase.");

	using scalar = typename Eigen::internal::traits<T>::Scalar;
	static constexpr auto rows = T::RowsAtCompileTime;
	static constexpr auto cols = T::ColsAtCompileTime;

	static_assert(rows != Eigen::Dynamic, "Row count must be fixed.");
	static_assert(cols != Eigen::Dynamic, "Column count must be fixed.");

	static constexpr auto size = rows * cols;
	static constexpr auto is_row_major = T::IsRowMajor;
	static constexpr auto is_col_major = !is_row_major;
	static constexpr auto is_vector = T::IsVectorAtCompileTime;

	static constexpr auto storage_order =
	is_row_major ? storage_order::row_major : storage_order::column_major;

	using archive_type = typename std::conditional<
		is_vector,
		vector<scalar, size>,
		matrix<scalar, rows, cols, storage_order>
	>::type;
};

/*
** Returns the archive type associated with a tuple of Eigen types.
*/

template <class T, bool IsScalar>
struct archive_type_impl2;

template <class T>
struct archive_type_impl2<T, true>
{
	static_assert(std::is_arithmetic<T>::value, "Type must be scalar.");
	using type = T;
};

template <class T>
struct archive_type_impl2<T, false>
{
	using type = typename eigen_matrix_traits<T>::archive_type;
};

template <class T>
struct archive_type_impl
{
	static constexpr auto is_matrix = is_eigen_matrix<T>::value;
	using type = typename archive_type_impl2<T, !is_matrix>::type;
};

template <class... Ts>
struct archive_type_impl<std::tuple<Ts...>>
{
	using type = std::tuple<typename archive_type_impl<Ts>::type...>;
};

template <class T>
using archive_type = typename archive_type_impl<T>::type;

}}

#endif
