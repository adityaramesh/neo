/*
** File Name: io_strategy.hpp
** Author:    Aditya Ramesh
** Date:      07/01/2014
** Contact:   _@adityaramesh.com
**
** The `io_strategy` class describes a set of platform-specific IO optimizations
** that can be applied to a given file descriptor. Basic usage of this class
** proceeds as follows:
**
**   1. Construct an `io_strategy` object.
**   2. Invoke the `set_defaults` member function to set the default parameters,
**   which were chosen based on [this benchmark][io_benchmark].
**   2. Obtain a file descriptor to a path using the `open` function.
**   3. Before performing IO, apply optimizations to the file descriptor using
**   the `apply` function.
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
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>
#include <neo/core/access_mode.hpp>
#include <neo/core/buffer_constraints.hpp>
#include <neo/core/io_type.hpp>
#include <neo/core/file/open_mode.hpp>
#include <neo/core/file/io_method.hpp>
#include <neo/core/file/system.hpp>

namespace neo {
namespace file {

class io_strategy
{
	buffer_constraints m_ireq{};
	buffer_constraints m_ipref{};
	buffer_constraints m_oreq{};
	buffer_constraints m_opref{};
	buffer_constraints m_ioreq{};
	buffer_constraints m_iopref{};
	boost::optional<off_t> m_cur_fs{};
	boost::optional<off_t> m_new_fs{};
	boost::optional<blksize_t> m_blksize{};
	boost::optional<io_method> m_read_mtd{};
	boost::optional<io_method> m_write_mtd{};
	bool m_rdahead{};
	bool m_preallocate{};
public:
	explicit io_strategy(
		boost::optional<off_t> cur_fs      = boost::none,
		boost::optional<off_t> new_fs      = boost::none,
		boost::optional<blksize_t> blksize = boost::none
	) noexcept : m_cur_fs{cur_fs}, m_new_fs{new_fs},
	m_blksize{blksize} {} 

	template <open_mode OpenMode, access_mode AccessMode>
	io_strategy& set_defaults();

	buffer_constraints&
	required_constraints(io_type t)
	{
		switch (t) {
		case io_type::input: return m_ireq;
		case io_type::output: return m_oreq;
		case io_type::input | io_type::output: return m_ioreq;
		}
	}

	buffer_constraints&
	preferred_constraints(io_type t)
	{
		switch (t) {
		case io_type::input: return m_ipref;
		case io_type::output: return m_opref;
		case io_type::input | io_type::output: return m_iopref;
		}
	}

	const buffer_constraints&
	required_constraints(io_type t) const
	{
		switch (t) {
		case io_type::input: return m_ireq;
		case io_type::output: return m_oreq;
		case io_type::input | io_type::output: return m_ioreq;
		}
	}

	const buffer_constraints&
	preferred_constraints(io_type t) const
	{
		switch (t) {
		case io_type::input: return m_ipref;
		case io_type::output: return m_opref;
		case io_type::input | io_type::output: return m_iopref;
		}
	}

	io_strategy&
	required_constraints(io_type t, buffer_constraints& bc)
	{
		switch (t) {
		case io_type::input: m_ireq = bc; break;
		case io_type::output: m_oreq = bc; break;
		case io_type::input | io_type::output: m_ioreq = bc; break;
		}
		return *this;
	}

	io_strategy&
	preferred_constraints(io_type t, buffer_constraints& bc)
	{
		switch (t) {
		case io_type::input: m_ipref = bc; break;
		case io_type::output: m_opref = bc; break;
		case io_type::input | io_type::output: m_iopref = bc; break;
		}
		return *this;
	}

	DEFINE_COPY_GETTER_SETTER(io_strategy, current_file_size, m_cur_fs)
	DEFINE_COPY_GETTER_SETTER(io_strategy, expected_file_size, m_new_fs)
	DEFINE_COPY_GETTER_SETTER(io_strategy, block_size, m_blksize)
	DEFINE_COPY_GETTER_SETTER(io_strategy, read_method, m_read_mtd)
	DEFINE_COPY_GETTER_SETTER(io_strategy, write_method, m_write_mtd)
	DEFINE_COPY_GETTER_SETTER(io_strategy, read_ahead, m_rdahead)
	DEFINE_COPY_GETTER_SETTER(io_strategy, preallocate, m_preallocate)
};

/*
** Sets the parameters to default values baesd on the results of [this
** benchmark][io_benchmark].
**
** [io_benchmark]: http://adityaramesh.com/io_benchmark/
*/
template <open_mode OpenMode, access_mode AccessMode>
io_strategy& io_strategy::set_defaults()
{
	if (open_mode_traits<OpenMode>::is_read_only) {
		assert(m_cur_fs && "Current file size required.");
		assert(m_blksize && "Block size required.");

		if (AccessMode == access_mode::sequential) {
			m_read_mtd = io_method::paging;
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				m_rdahead = false;
				m_ipref.at_least(4_KB)
					.at_most(256_KB)
					.align_to(m_blksize.get());
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				m_rdahead = true;
				if ((uint64_t)m_cur_fs.get() < 256_MB) {
					m_ipref.at_least(4_KB)
						.at_most(1024_KB)
						.align_to(m_blksize.get());
				}
				else {
					m_ipref.at_least(4096_KB)
						.align_to(m_blksize.get());
				}
			#endif
		}
		else {
			m_read_mtd = io_method::direct;
			m_rdahead = false;
			m_ireq.align_to(m_blksize.get());
			m_ipref.align_to(m_blksize.get());
		}
	}
	else if (open_mode_traits<OpenMode>::is_write_only) {
		if (m_new_fs) {
			m_preallocate = true;
		}
		if (AccessMode == access_mode::sequential) {
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				if (m_new_fs && m_new_fs.get() < 256_MB) {
					m_write_mtd = io_method::paging;
					m_opref.at_least(4096_KB)
						.align_to(m_blksize.get());
				}
				else {
					m_write_mtd = io_method::mmap;
				}
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				m_write_mtd = io_method::paging;
				m_opref.at_least(1024_KB);
			#endif
		}
		else {
			m_write_mtd = io_method::direct;
			m_oreq.align_to(m_blksize.get());
			m_opref.align_to(m_blksize.get());
		}
	}
	else {
		/*
		** XXX: These settings use more guessing/extrapolation than the
		** others, so they are more liable to be ineffective. Back these
		** up with benchmarks when time permits.
		*/

		assert(m_cur_fs && "Current file size required.");
		assert(m_blksize && "Block size required.");

		if (m_new_fs && m_new_fs > m_cur_fs) {
			m_preallocate = true;
		}

		if (AccessMode == access_mode::sequential) {
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				m_read_mtd = io_method::paging;
				m_write_mtd = io_method::paging;
				m_ipref.align_to(m_blksize.get());
				m_opref.align_to(m_blksize.get());
				m_iopref.align_to(m_blksize.get());
				m_rdahead = true;
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				m_rdahead = false;
				if (
					(!m_new_fs && (uint64_t)m_cur_fs.get() > 256_MB) ||
					(m_new_fs  && (uint64_t)m_new_fs.get() > 256_MB)
				) {
					m_read_mtd = io_method::paging;
					m_write_mtd = io_method::paging;
					m_ipref.align_to(m_blksize.get());
					m_opref.align_to(m_blksize.get());
					m_iopref.align_to(m_blksize.get());
				}
				else {
					m_read_mtd = io_method::mmap;
					m_write_mtd = io_method::mmap;
				}
			#endif
		}
		else {
			m_read_mtd = io_method::direct;
			m_write_mtd = io_method::direct;
			m_ireq.align_to(m_blksize.get());
			m_oreq.align_to(m_blksize.get());
			m_ioreq.align_to(m_blksize.get());
			m_ipref.align_to(m_blksize.get());
			m_opref.align_to(m_blksize.get());
			m_iopref.align_to(m_blksize.get());
			m_rdahead = false;
		}
	}
	return *this;
}


std::ostream& operator<<(std::ostream& os, const io_strategy& s)
{
	cc::writeln(os, "io_strategy object:");

	if (s.read_method()) {
		cc::writeln(os, " * Read method: $.", s.read_method().get());
	}
	else {
		cc::writeln(os, " * Read method: none.");
	}

	if (s.write_method()) {
		cc::writeln(os, " * Write method: $.", s.write_method().get());
	}
	else {
		cc::writeln(os, " * Write method: none.");
	}

	if (s.current_file_size()) {
		cc::writeln(os, " * Current file size: $.", s.current_file_size().get());
	}
	else {
		cc::writeln(os, " * Current file size: not provided.");
	}

	if (s.expected_file_size()) {
		cc::writeln(os, " * Expected file size: $.", s.expected_file_size().get());
	}
	else {
		cc::writeln(os, " * Expected file size: not provided.");
	}

	if (s.block_size()) {
		cc::writeln(os, " * Block size: $.", s.block_size().get());
	}
	else {
		cc::writeln(os, " * Block size: not provided.");
	}

	cc::writeln(os, " * Read ahead: $.", s.read_ahead());
	cc::writeln(os, " * Preallocate: $.", s.preallocate());
	return os;
}

/*
** Returns a file descriptor to the corresponding path, with the appropriate
** flags enabled so that the optimizations described by the given `io_strategy`
** object will work.
*/
template <open_mode OpenMode>
cc::expected<int>
open(const char* path, const io_strategy& s)
{
	constexpr auto flags = to_posix<OpenMode>::value;

	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		if (
			(s.read_method() == io_method::direct &&
			(open_mode_traits<OpenMode>::has_read_access)) ||
			(s.write_method() == io_method::direct &&
			(open_mode_traits<OpenMode>::has_write_access))
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
	/*
	** Under Linux, the file descriptor should have already been opened
	** using `O_DIRECT`, so we do not need to do anything.
	*/
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		if (s.read_method() == io_method::direct) {
		     safe_nocache(fd).get();
		}
	#endif

	if (open_mode_traits<OpenMode>::has_read_access) {
		if (s.read_ahead()) {
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				safe_fadvise_sequential(fd, s.current_file_size().get()).get();
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				if ((uint64_t)s.current_file_size().get() < 256_MB) {
					safe_rdadvise(fd, s.current_file_size().get()).get();
				}
				else {
					safe_rdahead(fd).get();
				}
			#endif
		}
	}
	if (open_mode_traits<OpenMode>::has_write_access) {
		if (s.preallocate() && s.expected_file_size()) {
			safe_preallocate(fd, s.expected_file_size().get()).get();
		}
	}
}

}}

#endif
