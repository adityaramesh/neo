/*
** File Name: device_capabilities.hpp
** Author:    Aditya Ramesh
** Date:      07/08/2014
** Contact:   _@adityaramesh.com
**
** This concept is taken directly from Boost.Iostreams. It is documented
** [here][iostreams_mode].
**
** [iostreams_mode]:
** http://www.boost.org/doc/libs/1_55_0/libs/iostreams/doc/index.html
*/

#ifndef Z7A4B1DAE_D234_4812_A552_E6BE2F9ED933
#define Z7A4B1DAE_D234_4812_A552_E6BE2F9ED933

namespace neo {

enum class device_capabilities : unsigned
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
