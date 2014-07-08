/*
** File Name: io_strategy.hpp
** Author:    Aditya Ramesh
** Date:      07/01/2014
** Contact:   _@adityaramesh.com
**
** The `io_strategy` class describes a set of platform-specific IO optimizations
** that can be applied to a given file descriptor. The following three items
** describe the basic usage of this class.
**
**   1. Obtain a default `io_strategy` object via the `default_io_strategy`
**   function.
**   2. Obtain a file descriptor to a path via the `open` function.
**   3. Apply the optimizations to the file descriptor using the `apply`
**   function.
**
** Important notes:
**
**   - Any file descriptors used with the `io_strategy` class **must** be
**   obtained via the `open` function. This is because the `apply` function
**   makes assumptions about which flags were used in the `open` system call.
**   - IO buffers **must** abide by the constraints returned by the
**   `required_constraints` member function. For performance reasons, it is
**   recommended that the IO buffer also abide by the constraints returned by
**   the `preferred_constraints` member function.
**
** All default values are based on the results of [this
** benchmark][io_benchmark]. It may well be the case that the default choices
** perform poorly on your platform. This is because the "best" IO methods to use
** for reading, writing, and copying files vary based on several factors,
** including the hard drive, file system, kernel version, and so on.
**
** If you are interested in optimizing the IO performance on your platform, then
** I suggest running the benchmark on your platform. The results of the
** benchmark can then be used to manually tune the `io_strategy` parameters. If
** you are able to, please send the benchmark results to me, so that the library
** can work better on more platforms like yours, without any manual tuning.
**
** [io_benchmark]: http://adityaramesh.com/io_benchmark/
*/

#ifndef Z2F4061B0_C200_4AB5_97E0_C98EE1ED1A4B
#define Z2F4061B0_C200_4AB5_97E0_C98EE1ED1A4B

#include <cassert>
#include <ostream>
#include <neo/core/access_mode.hpp>
#include <neo/core/buffer_constraints.hpp>
#include <neo/core/io_common.hpp>
#include <neo/core/io_method.hpp>
#include <neo/core/open_mode.hpp>
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>

namespace neo {

class io_strategy
{
	buffer_constraints m_req{};
	buffer_constraints m_pref{};
	off_t m_cur_fs;
	boost::optional<off_t> m_new_fs{};
	blksize_t m_blksize;
	io_method m_read_mtd;
	io_method m_write_mtd;
	bool m_rdahead;
	bool m_preallocate;
public:
	explicit io_strategy(const char* path)
	{
		auto st = safe_stat(path).get();
		m_cur_fs = st.st_size;
		m_blksize = st.st_blksize;
	}

	DEFINE_REF_GETTER_SETTER(io_strategy, required_constraints, m_req)
	DEFINE_REF_GETTER_SETTER(io_strategy, preferred_constraints, m_pref)
	DEFINE_COPY_GETTER_SETTER(io_strategy, current_file_size, m_cur_fs)
	DEFINE_COPY_GETTER_SETTER(io_strategy, expected_file_size, m_new_fs)
	DEFINE_COPY_GETTER_SETTER(io_strategy, block_size, m_blksize)
	DEFINE_COPY_GETTER_SETTER(io_strategy, read_method, m_read_mtd)
	DEFINE_COPY_GETTER_SETTER(io_strategy, write_method, m_write_mtd)
	DEFINE_COPY_GETTER_SETTER(io_strategy, read_ahead, m_rdahead)
	DEFINE_COPY_GETTER_SETTER(io_strategy, preallocate, m_preallocate)
};

std::ostream& operator<<(std::ostream& os, const io_strategy& s)
{
	cc::writeln(os, "io_strategy object:");

	switch (s.read_method()) {
	case io_method::normal:
		cc::writeln(os, " * Read method: normal.");
		break;
	case io_method::direct:
		cc::writeln(os, " * Read method: direct.");
		break;
	case io_method::mmap:
		cc::writeln(os, " * Read method: mmap.");
		break;
	}

	switch (s.write_method()) {
	case io_method::normal:
		cc::writeln(os, " * Write method: normal.");
		break;
	case io_method::direct:
		cc::writeln(os, " * Write method: direct.");
		break;
	case io_method::mmap:
		cc::writeln(os, " * Write method: mmap.");
		break;
	}

	cc::writeln(os, " * Current file size: $.", s.current_file_size());
	if (s.expected_file_size()) {
		cc::writeln(os, " * Expected file size: $.", s.expected_file_size());
	}
	else {
		cc::writeln(os, " * Expected file size: not provided.");
	}

	cc::writeln(os, " * Block size: $.", s.block_size());
	cc::writeln(os, " * Read ahead: $.", s.read_ahead());
	cc::writeln(os, " * Preallocate: $.", s.preallocate());

	cc::write(os, " * Required constraints:\n");
	print_as_list_item(os, s.required_constraints());
	cc::write(os, " * Preferred constraints:\n");
	print_as_list_item(os, s.preferred_constraints());
	return os;
}

template <open_mode OpenMode, access_mode AccessMode>
io_strategy
default_io_strategy(
	const char* path,
	boost::optional<off_t> new_fs = boost::none
)
{
	static constexpr auto kb = off_t{1024};
	static constexpr auto mb = off_t{1048576};
	auto s = io_strategy(path);
	s.preferred_constraints().align_to(s.block_size());

	if (OpenMode == open_mode::read) {
		if (AccessMode == access_mode::sequential) {
			s.read_method(io_method::normal);
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				s.read_ahead(false);
				s.preferred_constraints().
					at_least(4 * kb).
					at_most(256 * kb);
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				s.read_ahead(true);
				if (s.current_file_size() < 256 * mb) {
					s.preferred_constraints().
						at_least(4 * kb).
						at_most(1024 * kb);
				}
				else {
					s.preferred_constraints().
						at_least(4096 * kb);
				}
			#endif
		}
		else {
			s.read_method(io_method::direct).read_ahead(false);
			s.required_constraints().align_to(s.block_size());
		}
	}
	else if (OpenMode == open_mode::replace) {
		if (new_fs) {
			s.expected_file_size(new_fs).preallocate(true);
		}
		#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
			if (new_fs && new_fs < 256 * mb) {
				s.write_method(io_method::normal);
				s.preferred_constraints().at_least(4096 * kb);
			}
			else {
				s.write_method(io_method::mmap);
			}
		#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
			s.write_method(io_method::normal);
		#endif
	}
	else {
		if (new_fs && new_fs > s.current_file_size()) {
			s.expected_file_size(new_fs).preallocate(true);
		}
		if (AccessMode == access_mode::sequential) {
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				s.read_method(io_method::normal).
					write_method(io_method::normal).
					read_ahead(true);
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				s.read_ahead(false);
				if (
					(!new_fs && s.current_file_size() > 256 * mb) ||
					(new_fs  && new_fs > 256 * mb)
				) {
					s.read_method(io_method::normal).
						write_method(io_method::normal);
				}
				else {
					s.read_method(io_method::mmap).
						write_method(io_method::mmap);
				}
			#endif
		}
		else {
			s.read_method(io_method::direct).
				write_method(io_method::direct).
				read_ahead(false);
			s.required_constraints().align_to(s.block_size());
		}
	}
	return s;
}

template <open_mode OpenMode>
cc::expected<int>
open(const char* path, const io_strategy& s)
{
	constexpr auto flags = to_posix<OpenMode>::value;
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		if (
			(s.read_method() == io_method::direct &&
			(OpenMode == open_mode::read || OpenMode == open_mode::modify)) ||
			(s.write_method() == io_method::direct &&
			(OpenMode == open_mode::replace || OpenMode == open_mode::modify))
		) {
			return safe_open(path, flags | O_DIRECT);
		}
		else {
			return safe_open(path, flags);
		}
	#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		(void)s;
		return safe_open(path, flags);
	#endif
}

template <open_mode OpenMode>
void apply(const io_strategy& s, int fd)
{
	static constexpr auto mb = off_t{1048576};

	/*
	** Under Linux, the file descriptor should have already been opened
	** using `O_DIRECT`, so we do not need to do anything.
	*/
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		if (s.read_method() == io_method::direct) {
		     safe_nocache(fd).get();
		}
	#endif

	if (OpenMode == open_mode::read || OpenMode == open_mode::modify) {
		if (s.read_ahead()) {
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				safe_fadvise_sequential(fd, s.current_file_size()).get();
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				if (s.current_file_size() < 256 * mb) {
					safe_rdadvise(fd, s.current_file_size()).get();
				}
				else {
					safe_rdahead(fd).get();
				}
			#endif
		}
	}
	if (OpenMode == open_mode::replace || OpenMode == open_mode::modify) {
		if (s.preallocate() && s.expected_file_size()) {
			safe_preallocate(fd, s.expected_file_size().get()).get();
		}
	}
}

}

#endif
