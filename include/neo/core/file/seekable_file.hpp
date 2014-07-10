/*
** File Name: seekable_file.hpp
** Author:    Aditya Ramesh
** Date:      07/08/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZB9176DC8_611A_45A5_8CEA_978925C54EB0
#define ZB9176DC8_611A_45A5_8CEA_978925C54EB0

#include <cassert>
#include <type_traits>
#include <ccbase/utility.hpp>
#include <neo/core/device_capabilities.hpp>
#include <neo/core/file/buffer.hpp>
#include <neo/core/file/io_strategy.hpp>
#include <neo/core/file/system.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX || \
    PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
	#include <sys/mman.h>
#else
	#error "Unsupported kernel."
#endif

namespace neo {
namespace file {

template <open_mode OpenMode>
class seekable_file
{
public:
	using handle_type = int;
	using offset_type = off_t;
	using ibuffer_type = buffer;
	using obuffer_type = buffer;
	using iobuffer_type = buffer;

	static constexpr auto capabilities = 
	(open_mode_traits<OpenMode>::is_read_only ?
		device_capabilities::input_seekable :
		(open_mode_traits<OpenMode>::is_write_only ?
			device_capabilities::output_seekable :
			device_capabilities::seekable));
private:
	uint8_t* m_mapped{};
	size_t m_mapped_size{};
	int m_fd{-1};
	const io_strategy& m_strat;
public:
	explicit seekable_file(const char* path, const io_strategy& s)
	: m_strat{s}
	{
		m_fd = open<OpenMode>(path, s).get();
		#ifndef NEO_NO_DEBUG
			assert(is_seekable(m_fd));
		#endif

		apply(m_strat, m_fd);

		auto use_mmap =
		(open_mode_traits<OpenMode>::has_read_access &&
		 	s.read_method() == io_method::mmap) ||
		(open_mode_traits<OpenMode>::has_write_access &&
		 	s.write_method() == io_method::mmap);

		if (!use_mmap) return;

		auto prot = int{};

		auto mmap_rw =
		open_mode_traits<OpenMode>::has_read_access  &&
		open_mode_traits<OpenMode>::has_write_access &&
		s.read_method() == io_method::mmap           &&
		s.write_method() == io_method::mmap;

		if (!mmap_rw && s.read_method() == io_method::mmap) {
			assert(s.current_file_size() &&
				"Current file size is required for mmap.");
			m_mapped_size = s.current_file_size().get();
			prot = PROT_READ;
		}
		else if (!mmap_rw && s.write_method() == io_method::mmap) {
			assert(s.current_file_size() &&
				"Expected file size is required for mmap.");
			m_mapped_size = s.expected_file_size().get();
			prot = PROT_WRITE;
		}
		else {
			assert(s.current_file_size() &&
				"Current file size is required for mmap.");
			if (
				!s.expected_file_size() ||
				s.expected_file_size() <= s.current_file_size()
			) {
				m_mapped_size = s.current_file_size().get();
			}
			else {
				m_mapped_size = s.current_file_size().get();
			}
			prot = PROT_READ | PROT_WRITE;
		}

		m_mapped = ::mmap(nullptr, m_mapped_size, prot, MAP_PRIVATE, m_fd, 0);
		if (m_mapped == (uint8_t*)-1) {
			throw current_system_error();
		}
	}

	~seekable_file()
	{
		if (m_fd != -1) {
			safe_close(m_fd).get();
		}
		if (m_mapped != (uint8_t*)-1) {
			if (::munmap(m_mapped, m_mapped_size) == -1) {
				throw current_system_error();
			}
		}
	}

	DEFINE_COPY_GETTER(handle, m_fd)

	const boost::optional<buffer_constraints>&
	required_constraints(io_type t) const noexcept
	{ return m_strat.required_constraints(t); }

	const boost::optional<buffer_constraints>&
	preferred_constraints(io_type t) const noexcept
	{ return m_strat.preferred_constraints(t); }

	ibuffer_type
	allocate_ibuffer(const buffer_constraints& bc)
	{
		assert(m_strat.read_method());
		assert(bc.satisfies(required_constraints(io_type::input)));

		if (m_strat.read_method() == io_method::mmap) {
			return buffer{io_type::input, m_mapped, m_mapped_size};
		}
		else {
			return buffer{bc};
		}
	}

	obuffer_type
	allocate_obuffer(const buffer_constraints& bc)
	{
		assert(m_strat.write_method());
		assert(bc.satisfies(required_constraints(io_type::output)));

		if (m_strat.write_method() == io_method::mmap) {
			return buffer{io_type::output, m_mapped, m_mapped_size};
		}
		else {
			return buffer{bc};
		}
	}

	boost::optional<iobuffer_type>
	allocate_iobuffer(const buffer_constraints& bc)
	{
		assert(m_strat.read_method());
		assert(m_strat.write_method());
		assert(bc.satisfies(required_constraints(io_type::input | io_type::output)));

		if (m_strat.read_method() == io_method::mmap) {
			if (m_strat.write_method() == io_method::mmap) {
				return buffer{io_type::input | io_type::output,
					m_mapped, m_mapped_size};
			}
			else {
				return boost::none;
			}
		}
		else {
			if (m_strat.write_method() == io_method::mmap) {
				return boost::none;
			}
			else {
				return buffer{bc};
			}
		}
	}

	cc::expected<void>
	read(offset_type off, size_t n, buffer& buf)
	{
		assert(n > 0);
		assert(m_strat.read_method());
		assert(m_strat.current_file_size());

		if (open_mode_traits<OpenMode>::is_read_only) {
			assert(off + n <= m_strat.current_file_size().get());
		}

		if (m_strat.read_method() != io_method::mmap) {
			return full_read(m_fd, buf.data(), n, off);
		}
		else {
			if (buf.mapped() && buf.base_pointer() == m_mapped) {
				buf.offset(off);
			}
			else {
				std::copy_n(m_mapped + off, n, buf.data());
			}
			return true;
		}
	}

	cc::expected<void>
	write(offset_type off, size_t n, const buffer& buf)
	{
		assert(m_strat.write_method());

		if (m_strat.write_method() != io_method::mmap) {
			return full_write(m_fd, buf.data(), n, off);
		}
		else if (buf.base_pointer() != m_mapped) {
			std::copy_n(buf.data(), n, m_mapped);
			return true;
		}
	}
};

}}

#endif
