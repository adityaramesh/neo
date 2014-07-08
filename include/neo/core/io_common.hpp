/*
** File Name: io_common.hpp
** Author:    Aditya Ramesh
** Date:      07/01/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z8935F853_2245_41F3_BAC1_D72ECE5A35FD
#define Z8935F853_2245_41F3_BAC1_D72ECE5A35FD

#include <system_error>
#include <tuple>
#include <ccbase/error.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX || \
    PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
	#include <fcntl.h>
#else
	#error "Unsupported kernel."
#endif

namespace neo {

using blksize_t = decltype(::stat::st_blksize);

std::system_error current_system_error()
{ return std::system_error{errno, std::system_category()}; }

cc::expected<int>
safe_open(const char* path, int flags)
{
	auto r = int{};
	do {
		r = ::open(path, flags, S_IRUSR | S_IWUSR);
	}
	while (r == -1 && errno == EINTR);

	if (r == -1) { return current_system_error(); }
	return r;
}

cc::expected<void>
safe_close(int fd)
{
	auto r = ::close(fd);
	if (r == 0) { return true; }
	if (errno != EINTR) { return current_system_error(); }

	for (;;) {
		r = ::close(fd);
		if (errno == EBADF) { return true; }
		if (errno != EINTR) { return current_system_error(); }
	}
}

cc::expected<struct stat>
safe_stat(const char* path)
{
	struct stat st;
	auto r = ::stat(path, &st);
	if (r == -1) { return current_system_error(); }
	return st;
}

cc::expected<struct stat>
safe_stat(int fd)
{
	struct stat st;
	auto r = ::fstat(fd, &st);
	if (r == -1) { return current_system_error(); }
	return st;
}

cc::expected<ssize_t>
full_read(int fd, uint8_t* buf, size_t count, off_t offset)
{
	auto c = size_t{0};
	do {
		auto r = ::pread(fd, buf + c, count - c, offset + c);
		if (r > 0) {
			c += r;
		}
		else if (r == 0) {
			return c;
		}
		else {
			if (errno == EINTR) { continue; }
			return current_system_error();
		}
	}
	while (c < count);
	return c;
}

cc::expected<ssize_t>
full_write(int fd, uint8_t* buf, size_t count, off_t offset)
{
	auto c = size_t{0};
	do {
		auto r = ::pwrite(fd, buf + c, count - c, offset + c);
		if (r > 0) {
			c += r;
		}
		else if (r == 0) {
			return c;
		}
		else {
			if (errno == EINTR) { continue; }
			return current_system_error();
		}
	}
	while (c < count);
	return c;
}

cc::expected<void>
safe_truncate(int fd, off_t fs)
{
	auto r = int{};
	do {
		r = ::ftruncate(fd, fs);
	}
	while (r == -1 && errno == EINTR);
	if (r == -1) { return current_system_error(); }
	return true;
}

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX

cc::expected<void>
safe_fadvise_sequential(int fd, off_t fs)
{
	// It turns out that this makes things slower on the machines that I
	// tested.
	// ::posix_fadvise(fd, 0, fs, POSIX_FADV_WILLNEED);
	if (::posix_fadvise(fd, 0, fs, POSIX_FADV_NOREUSE) == -1) {
		return current_system_error();
	}
	if (::posix_fadvise(fd, 0, fs, POSIX_FADV_SEQUENTIAL) == -1) {
		return current_system_error();
	}
	return true;
}

cc::expected<void>
safe_preallocate(int fd, size_t len)
{
	auto r = int{};
	do {
		r = ::fallocate(fd, 0, 0, len);
	}
	while (r == -1 && errno == EINTR);
	if (r == -1) { return current_system_error(); }
	return true;
}

#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU

cc::expected<void>
safe_nocache(int fd)
{ 
	if (::fcntl(fd, F_NOCACHE, 1) == -1) {
		return current_system_error();
	}
	return true;
}

cc::expected<void>
safe_rdadvise(int fd, off_t fs)
{
	auto rd = radvisory{};
	rd.ra_offset = 0;
	rd.ra_count = fs;
	if (::fcntl(fd, F_RDADVISE, &rd) == -1) {
		return current_system_error();
	}
	return true;
}

cc::expected<void>
safe_rdahead(int fd)
{
	if (::fcntl(fd, F_RDAHEAD, 1) == -1) {
		return current_system_error();
	}
	return true;
}

cc::expected<void>
safe_preallocate(int fd, size_t len)
{
	using fstore = struct fstore;
	auto fs = fstore{};
	fs.fst_posmode = F_ALLOCATECONTIG;
	fs.fst_offset = F_PEOFPOSMODE;
	fs.fst_length = len;

	/*
	** XXX: We do not distinguish between failure to allocate because of
	** insufficient disk space and failure to allocate because of other
	** reasons (e.g. bad file descriptor). What is the error code returned
	** in the former case? The man pages are not clear on this.
	*/
	if (::fcntl(fd, F_PREALLOCATE, &fs) == -1) {
		fs.fst_posmode = F_ALLOCATEALL;
		if (::fcntl(fd, F_PREALLOCATE, &fs) == -1) {
			return current_system_error();
		}
	}
	return true;
}

#endif

}

#endif
