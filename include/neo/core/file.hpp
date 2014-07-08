/*
** File Name: file.hpp
** Author:    Aditya Ramesh
** Date:      07/08/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZB9176DC8_611A_45A5_8CEA_978925C54EB0
#define ZB9176DC8_611A_45A5_8CEA_978925C54EB0

#include <type_traits>
#include <neo/core/mode.hpp>
#include <neo/core/io_strategy.hpp>

namespace neo {

template <open_mode OpenMode>
class file
{
public:
	using category = typename std::conditional<
		OpenMode == open_mode::read,
		mode::input_seekable,
		typename std::conditional<
			OpenMode == open_mode::replace,
			mode::output_seekable,
			mode::seekable
		>::type
	>::type;
};

}

#endif
