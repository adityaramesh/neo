/*
** File Name: strategy.hpp
** Author:    Aditya Ramesh
** Date:      07/01/2014
** Contact:   _@adityaramesh.com
**
** # Introduction
**
** The `strategy` class describes a set of platform-specific IO optimizations
** that can be applied to a given file handle. Basic usage of this class
** proceeds as follows:
**
**   1. Create a `strategy` object using the constructor that accepts a file
**   path.
**   2. If you are writing to the file and know the maximum file size, supply
**   this information using the corresponding setter function.
**   3. Invoke the `infer_defaults` member function to set parameters to default
**   values based on the information about the file that you provided. These
**   values are chosen based on [this benchmark][io_benchmark].
**   4. Obtain a handle a file using the `open` function.
**
** It may well be the case that the default choices perform poorly on your
** platform. This is because the "best" IO methods to use for reading, writing,
** and copying files vary based on several factors, including the hard drive,
** file system, kernel version, and so on.
**
** If you are interested in optimizing the IO performance on your platform, then
** I suggest running the benchmark on your platform. The results of the
** benchmark can then be used to manually tune the `strategy` parameters. If you
** are able to, please send the benchmark results to me, so that the library can
** work better on more platforms like yours, without any manual tuning.
**
** [io_benchmark]: http://adityaramesh.com/io_benchmark/
**
** # Reusing a `strategy` object
**
** The strategy object is quite large, because it contains many fields. If you
** are going to be performing the same operation on many files, it is possible
** (and recommended!) to reuse the same `strategy` object for IO. Here is how
** this can be done.
**
**   1. Construct the `strategy` object using the same steps described above for
**   the first file.
**   2. For subsequent files, update the `strategy` object using the
**   `update_with` function.
**   3. If the current or maximum file sizes differ significantly from the
**   previous values (say, by more than 50%), invoke `infer_defaults` again.
**   4. Proceed to use the `strategy` object as usual.
*/

#ifndef Z2F4061B0_C200_4AB5_97E0_C98EE1ED1A4B
#define Z2F4061B0_C200_4AB5_97E0_C98EE1ED1A4B

#include <cassert>
#include <ostream>
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>
#include <neo/core/access_mode.hpp>
#include <neo/core/io_mode.hpp>
#include <neo/core/buffer_constraints.hpp>
#include <neo/core/file/io_method.hpp>
#include <neo/core/file/system.hpp>

namespace neo {
namespace file {

template <io_mode IOMode>
class strategy
{
	buffer_constraints m_ireq{};
	buffer_constraints m_ipref{};
	buffer_constraints m_oreq{};
	buffer_constraints m_opref{};
	buffer_constraints m_ioreq{};
	buffer_constraints m_iopref{};
	boost::optional<off_t> m_cur_fs{};
	boost::optional<off_t> m_max_fs{};
	boost::optional<blksize_t> m_blksize{};
	boost::optional<io_method> m_read_mtd{};
	boost::optional<io_method> m_write_mtd{};
	bool m_rdahead{};
	bool m_preallocate{};
public:
	/*
	** Constructs a `strategy` object by inferring the necessary information
	** by invoking `stat` on `path`. The value of `path` depends on the IO
	** mode:
	**
	**   - `input` or `input | output`: The file path.
	**   - `output`: Path to any file on the partition to contain the file
	**   (possibly the parent directory).
	**
	** In no case should `path` refer to a non-existent file.
	*/
	explicit strategy(
		const char* path,
		boost::optional<off_t> max_fs = boost::none
	) noexcept : m_max_fs{max_fs}
	{
		if (m_max_fs) { assert(!!(IOMode & io_mode::output)); }
		update_with(path);
	}

	explicit strategy(
		boost::optional<off_t> cur_fs      = boost::none,
		boost::optional<off_t> max_fs      = boost::none,
		boost::optional<blksize_t> blksize = boost::none
	) noexcept : m_cur_fs{cur_fs}, m_max_fs{max_fs}, m_blksize{blksize}
	{
		if (!!(IOMode & io_mode::input)) {
			assert(m_cur_fs);
			assert(m_max_fs);
		}
	} 

	strategy& infer_defaults(access_mode);

	/*
	** Updates the file size and block size by invoking `stat` on `path`.
	** The `infer_defaults` function must be called explicitly in order to
	** recalibrate the other parameters based on the new information. If the
	** file size and block size have not changed much, calling
	** `infer_defaults` may be unnecessary.
	*/
	strategy& update_with(const char* path)
	{
		auto st = safe_stat(path).move();
		if (!!(IOMode & io_mode::input)) {
			m_cur_fs = st.st_size;
			m_blksize = st.st_blksize;
		}
		else {
			m_blksize = st.st_blksize;
		}

		if (m_cur_fs && m_max_fs) {
			assert(m_max_fs > m_cur_fs);
		}
		return *this;
	}

	bool supports_dual_use_buffers() const
	{
		return m_read_mtd && m_write_mtd &&
		((m_read_mtd == io_method::mmap && m_write_mtd == io_method::mmap) ||
		(m_read_mtd != io_method::mmap && m_write_mtd != io_method::mmap));
	}

	buffer_constraints&
	required_constraints(io_mode m)
	{
		switch (m) {
		case io_mode::input:
			assert(!!(IOMode & io_mode::input));
			return m_ireq;
		case io_mode::output:
			assert(!!(IOMode & io_mode::output));
			return m_oreq;
		case io_mode::input | io_mode::output:
			assert(IOMode == (io_mode::input | io_mode::output));
			return m_ioreq;
		}
	}

	buffer_constraints&
	preferred_constraints(io_mode m)
	{
		switch (m) {
		case io_mode::input:
			assert(!!(IOMode & io_mode::input));
			return m_ipref;
		case io_mode::output:
			assert(!!(IOMode & io_mode::output));
			return m_opref;
		case io_mode::input | io_mode::output:
			assert(IOMode == (io_mode::input | io_mode::output));
			return m_iopref;
		}
	}

	const buffer_constraints&
	required_constraints(io_mode m) const
	{
		switch (m) {
		case io_mode::input:
			assert(!!(IOMode & io_mode::input));
			return m_ireq;
		case io_mode::output:
			assert(!!(IOMode & io_mode::output));
			return m_oreq;
		case io_mode::input | io_mode::output:
			assert(IOMode == (io_mode::input | io_mode::output));
			return m_ioreq;
		}
	}

	const buffer_constraints&
	preferred_constraints(io_mode m) const
	{
		switch (m) {
		case io_mode::input:
			assert(!!(IOMode & io_mode::input));
			return m_ipref;
		case io_mode::output:
			assert(!!(IOMode & io_mode::output));
			return m_opref;
		case io_mode::input | io_mode::output:
			assert(IOMode == (io_mode::input | io_mode::output));
			return m_iopref;
		}
	}

	strategy&
	required_constraints(io_mode m, buffer_constraints& bc)
	{
		switch (m) {
		case io_mode::input:
			assert(!!(IOMode & io_mode::input));
			m_ireq = bc;
			break;
		case io_mode::output:
			assert(!!(IOMode & io_mode::output));
			m_oreq = bc;
			break;
		case io_mode::input | io_mode::output:
			assert(IOMode == (io_mode::input | io_mode::output));
			m_ioreq = bc;
			break;
		}
		return *this;
	}

	strategy&
	preferred_constraints(io_mode m, buffer_constraints& bc)
	{
		switch (m) {
		case io_mode::input:
			assert(!!(IOMode & io_mode::input));
			m_ipref = bc;
			break;
		case io_mode::output:
			assert(!!(IOMode & io_mode::output));
			m_opref = bc;
			break;
		case io_mode::input | io_mode::output:
			assert(IOMode == (io_mode::input | io_mode::output));
			m_iopref = bc;
			break;
		}
		return *this;
	}

	strategy&
	current_file_size(off_t fs)
	{
		if (m_max_fs) {
			assert(m_max_fs >= fs && "Maximum file size must be "
				"greater than or equal to the current file "
				"size. Truncate the file either before or "
				"after processing.");
		}
		m_cur_fs = fs;
		return *this;
	}

	strategy&
	maximum_file_size(off_t fs)
	{
		assert(!!(IOMode & io_mode::output));
		if (m_cur_fs) {
			assert(fs >= m_cur_fs && "Maximum file size must be "
				"greater than or equal to the current file "
				"size. Truncate the file either before or "
				"after processing.");
		}
		m_cur_fs = fs;
		return *this;
	}

	strategy&
	read_method(io_method m)
	{
		assert(!!(IOMode & io_mode::input));
		assert(m_cur_fs && "Reading requires known current file size.");
		m_read_mtd = m;
		return *this;
	}

	strategy&
	write_method(io_method m)
	{
		assert(!!(IOMode & io_mode::output));
		if (m == io_method::mmap) {
			assert(m_max_fs && "mmap requires known maximum file size.");
			m_preallocate = true;
		}
		m_write_mtd = m;
		return *this;
	}

	strategy& read_ahead(bool b)
	{
		assert(!!(IOMode & io_mode::input));
		m_rdahead = b;
		return *this;
	}

	strategy& preallocate(bool b)
	{
		assert(!!(IOMode & io_mode::output));
		assert(m_max_fs && "preallocate requires maximum file size.");
		m_preallocate = b;
		return *this;
	}

	DEFINE_COPY_GETTER_SETTER(strategy, block_size, m_blksize)
	DEFINE_COPY_GETTER(current_file_size, m_cur_fs)
	DEFINE_COPY_GETTER(maximum_file_size, m_max_fs)
	DEFINE_COPY_GETTER(read_method, m_read_mtd)
	DEFINE_COPY_GETTER(write_method, m_write_mtd)
	DEFINE_COPY_GETTER(read_ahead, m_rdahead)
	DEFINE_COPY_GETTER(preallocate, m_preallocate)
};

/*
** Sets the parameters to default values based on the results of [this
** benchmark][io_benchmark].
**
** [io_benchmark]: http://adityaramesh.com/io_benchmark/
*/
template <io_mode IOMode>
strategy<IOMode>& strategy<IOMode>::infer_defaults(access_mode m)
{
	if (IOMode == io_mode::input) {
		assert(m_cur_fs && "Current file size required.");
		assert(m_blksize && "Block size required.");

		if (m == access_mode::sequential) {
			m_read_mtd = io_method::paging;
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				m_rdahead = true;
				m_ipref.at_least(40_KB)
					.at_most(256_KB)
					.align_to(*m_blksize);
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				m_rdahead = true;
				if ((uint64_t)*m_cur_fs < 256_MB) {
					m_ipref.at_least(4_KB)
						.at_most(1024_KB)
						.align_to(*m_blksize);
				}
				else {
					m_ipref.at_least(4096_KB)
						.align_to(*m_blksize);
				}
			#endif
		}
		else {
			m_read_mtd = io_method::direct;
			m_rdahead = false;
			m_ireq.align_to(*m_blksize);
			m_ipref.align_to(*m_blksize);
		}
	}
	else if (IOMode == io_mode::output) {
		if (m_max_fs) {
			m_preallocate = true;
		}
		if (m == access_mode::sequential) {
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				m_write_mtd = io_method::paging;
				m_opref.at_least(4096_KB).align_to(*m_blksize);
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				m_write_mtd = io_method::paging;
				m_opref.at_least(1024_KB);
			#endif
		}
		else {
			m_write_mtd = io_method::direct;
			m_oreq.align_to(*m_blksize);
			m_opref.align_to(*m_blksize);
		}
	}
	else {
		/*
		** XXX: These settings use more guessing/extrapolation than the
		** others, so they are more liable to be ineffective. Back these
		** up with benchmarks when time permits.
		*/

		assert(m_blksize && "Block size required.");

		if (m_max_fs && m_max_fs > m_cur_fs) {
			m_preallocate = true;
		}

		if (m == access_mode::sequential) {
			m_rdahead = true;
			m_read_mtd = io_method::paging;
			m_write_mtd = io_method::paging;

			m_ipref.align_to(*m_blksize);
			m_opref.align_to(*m_blksize);
			m_iopref.align_to(*m_blksize);
		}
		else {
			m_rdahead = false;
			m_read_mtd = io_method::direct;
			m_write_mtd = io_method::direct;

			m_ireq.align_to(*m_blksize);
			m_oreq.align_to(*m_blksize);
			m_ioreq.align_to(*m_blksize);

			m_ipref.align_to(*m_blksize);
			m_opref.align_to(*m_blksize);
			m_iopref.align_to(*m_blksize);
		}
	}
	return *this;
}

template <io_mode IOMode>
std::ostream& operator<<(std::ostream& os, const strategy<IOMode>& s)
{
	cc::writeln(os, "strategy object:");

	if (s.read_method()) {
		cc::writeln(os, " * Read method: $.", *s.read_method());
	}
	else {
		cc::writeln(os, " * Read method: none.");
	}

	if (s.write_method()) {
		cc::writeln(os, " * Write method: $.", *s.write_method());
	}
	else {
		cc::writeln(os, " * Write method: none.");
	}

	if (s.current_file_size()) {
		cc::writeln(os, " * Current file size: $.", *s.current_file_size());
	}
	else {
		cc::writeln(os, " * Current file size: not provided.");
	}

	if (s.maximum_file_size()) {
		cc::writeln(os, " * Maximum file size: $.", *s.maximum_file_size());
	}
	else {
		cc::writeln(os, " * Maximum file size: not provided.");
	}

	if (s.block_size()) {
		cc::writeln(os, " * Block size: $.", *s.block_size());
	}
	else {
		cc::writeln(os, " * Block size: not provided.");
	}

	cc::writeln(os, " * Read ahead: $.", s.read_ahead());
	cc::writeln(os, " * Preallocate: $.", s.preallocate());
	return os;
}

}}

#endif
