/*
** File Name: device.hpp
** Author:    Aditya Ramesh
** Date:      07/08/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZB9176DC8_611A_45A5_8CEA_978925C54EB0
#define ZB9176DC8_611A_45A5_8CEA_978925C54EB0

#include <type_traits>
#include <ccbase/utility.hpp>
#include <neo/core/device_capabilities.hpp>
#include <neo/core/file/buffer.hpp>
#include <neo/core/file/io_strategy.hpp>

namespace neo {
namespace file {

template <open_mode OpenMode>
class device
{
public:
	using handle_type = int;
	using offset_type = off_t;
	using ibuffer_type = file_buffer;
	using obuffer_type = file_buffer;
	using iobuffer_type = file_buffer;

	using capabilities = typename std::conditional<
		OpenMode == open_mode::read,
		device_capabilities::input_seekable,
		typename std::conditional<
			OpenMode == open_mode::replace,
			device_capabilities::output_seekable,
			device_capabilities::seekable
		>::type
	>::type;
private:
	io_strategy m_strat;
	int m_fd{-1};
public:
	explicit device(const char* path, const io_strategy& s)
	: m_strat{s} { m_fd = open<OpenMode>(path, s).get(); }

	~device() { if (m_fd != -1) safe_close(m_fd).get(); }

	DEFINE_COPY_GETTER(handle, m_fd)

	buffer_constraints& required_constraints()
	noexcept { return m_strat.required_constraints(); }

	buffer_constraints& preferred_constraints()
	noexcept { return m_strat.preferred_constraints(); }

	const buffer_constraints& required_constraints()
	const noexcept { return m_strat.required_constraints(); }

	const buffer_constraints& preferred_constraints()
	const noexcept { return m_strat.preferred_constraints(); }

	buffer_type allocate(const buffer_constraints& bc)
	{
		/*
		if (OpenMode == open_mode::read) {
			if (m_strat.read_method() == io_method::
		}
		else if (OpenMode == open_mode::replace ||
		         OpenMode == open_mode::modify)
		{

		}
		*/
	}

	// read
	// write
};

}}

#endif
