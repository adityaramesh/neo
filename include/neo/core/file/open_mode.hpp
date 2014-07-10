/*
** File Name: open_mode.hpp
** Author:    Aditya Ramesh
** Date:      06/30/2014
** Contact:   _@adityaramesh.com
**
** This file defines the `open_mode` enum, which is used to specify the kind of
** operations that will be performed on a file.
**
**   - `read`: Grants read access to an existing file.
**   - `create_if_not_exists`: Creates a new file iff the file does not already
**   exist. Provides write access.
**   - `create_or_replace`: Create a new file, replacing an existing file with
**   the same path if one exists. Provides write access.
**   - `replace_if_exists`: Create a new file iff a file with the same path
**   already exists. Provides write access.
**   - `modify`. Grants both read and write access to an existing file.
**
** The idea is that these enum values allow the programmer to express their
** intent more clearly and easily than the usual bitwise combinations of read,
** write, and create.
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
namespace file {

enum class open_mode : unsigned
{
	read,
	create_if_not_exists,
	create_or_replace,
	replace_if_exists,
	modify,
};

template <open_mode OpenMode>
struct open_mode_traits;

template <>
struct open_mode_traits<open_mode::read>
{
	static constexpr auto has_read_access = true;
	static constexpr auto has_write_access = false;
	static constexpr auto is_read_only = has_read_access && !has_write_access;
	static constexpr auto is_write_only = !has_read_access && has_write_access;
};

template <>
struct open_mode_traits<open_mode::create_if_not_exists>
{
	static constexpr auto has_read_access = false;
	static constexpr auto has_write_access = true;
	static constexpr auto is_read_only = has_read_access && !has_write_access;
	static constexpr auto is_write_only = !has_read_access && has_write_access;
};

template <>
struct open_mode_traits<open_mode::create_or_replace>
{
	static constexpr auto has_read_access = false;
	static constexpr auto has_write_access = true;
	static constexpr auto is_read_only = has_read_access && !has_write_access;
	static constexpr auto is_write_only = !has_read_access && has_write_access;
};

template <>
struct open_mode_traits<open_mode::replace_if_exists>
{
	static constexpr auto has_read_access = false;
	static constexpr auto has_write_access = true;
	static constexpr auto is_read_only = has_read_access && !has_write_access;
	static constexpr auto is_write_only = !has_read_access && has_write_access;
};

template <>
struct open_mode_traits<open_mode::modify>
{
	static constexpr auto has_read_access = true;
	static constexpr auto has_write_access = true;
	static constexpr auto is_read_only = has_read_access && !has_write_access;
	static constexpr auto is_write_only = !has_read_access && has_write_access;
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
struct to_posix<open_mode::create_if_not_exists>
{
	/*
	** We use `O_RDWR` instead of `O_WRONLY`, because some functions such as
	** `mmap` may require `O_RDWR`.
	*/
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		static constexpr auto value = O_RDWR | O_CREAT | O_NOATIME;
	#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		static constexpr auto value = O_RDWR | O_CREAT;
	#endif
};

template <>
struct to_posix<open_mode::create_or_replace>
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
struct to_posix<open_mode::replace_if_exists>
{
	/*
	** We use `O_RDWR` instead of `O_WRONLY`, because some functions such as
	** `mmap` may require `O_RDWR`.
	*/
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		static constexpr auto value = O_RDWR | O_CREAT | O_EXCL | O_TRUNC | O_NOATIME;
	#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		static constexpr auto value = O_RDWR | O_CREAT | O_EXCL | O_TRUNC;
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

}}

#endif
