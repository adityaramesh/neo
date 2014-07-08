/*
** File Name: open_mode.hpp
** Author:    Aditya Ramesh
** Date:      06/30/2014
** Contact:   _@adityaramesh.com
**
** Defines the kinds of access that one can obtain to a file:
**
**   - `read`. This mode provides read access only. The intended use of this
**   mode is for parsing existing files.
**   - `replace`. This mode always creates a new file, even if it means
**   replacing an existing file with the same path. The intended use of this
**   mode is for creating new files.
**   - `modify`. This mode provides both read and write access to an existing
**   file. The intended use of this mode is for updating existing files.
*/

#ifndef ZA9F7E783_C875_4731_8F5F_64952B504891
#define ZA9F7E783_C875_4731_8F5F_64952B504891

#include <ccbase/platform.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX || \
    PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
	// For `O_*` macros.
	#include <fcntl.h>
#else
	#error "Unsupported platform."
#endif

namespace neo {

enum class open_mode : unsigned
{
	read,
	replace,
	modify,
};

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX || \
    PLATFORM_KERNEL == PLATFORM_KERNEL_XNU

template <open_mode OpenMode>
struct to_posix;

template <>
struct to_posix<open_mode::read>
{
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		static constexpr auto value = O_RDONLY | O_NOATIME;
	#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		static constexpr auto value = O_RDONLY;
	#endif
};

template <>
struct to_posix<open_mode::replace>
{
	/*
	** We use `O_RDWR` instead of `O_WRONLY`, because some functions such as
	** `mmap` may require `O_RDWR`.
	*/
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		static constexpr auto value = O_RDWR | O_CREAT | O_TRUNC | O_NOATIME;
	#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		static constexpr auto value = O_RDWR | O_CREAT | O_TRUNC;
	#endif
};

template <>
struct to_posix<open_mode::modify>
{
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		static constexpr auto value = O_RDWR | O_NOATIME;
	#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		static constexpr auto value = O_RDWR;
	#endif
};

#endif

}

#endif
