/*
** File Name: io_mode.hpp
** Author:    Aditya Ramesh
** Date:      06/30/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZB6B93A39_C548_4CAC_91DE_F28807079624
#define ZB6B93A39_C548_4CAC_91DE_F28807079624

namespace neo {

enum class io_mode : unsigned
{
	input,
	output,
	bidirectional,
	input_seekable,
	output_seekable,
	seekable,
	dual_seekable,
	bidirectional_seekable
};

}

#endif
