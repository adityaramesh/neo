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

		apply<OpenMode>(m_strat, m_fd);

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

		m_mapped = (uint8_t*)::mmap(nullptr, m_mapped_size, prot, MAP_PRIVATE, m_fd, 0);
		if (m_mapped == (uint8_t*)-1) {
			m_mapped = nullptr;
			throw current_system_error();
		}
	}

	~seekable_file()
	{
		if (m_fd != -1) {
			safe_close(m_fd).get();
		}
		if (m_mapped != nullptr) {
			if (::munmap(m_mapped, m_mapped_size) == -1) {
				throw current_system_error();
			}
		}
	}

	DEFINE_COPY_GETTER(handle, m_fd)

	ENABLE_IF_HAS_READ_WRITE_ACCESS bool
	supports_dual_use_buffers() const
	{ return m_strat.read_method() == m_strat.write_method(); }

	const buffer_constraints&
	required_constraints(io_mode t) const noexcept
	{
		#ifndef NEO_NO_DEBUG
			if (open_mode_traits<OpenMode>::is_read_only) {
				assert(t == io_mode::input);
			}
			else if (open_mode_traits<OpenMode>::is_write_only) {
				assert(t == io_mode::output);
			}
		#endif
		return m_strat.required_constraints(t);
	}

	const buffer_constraints&
	preferred_constraints(io_mode t) const noexcept
	{
		#ifndef NEO_NO_DEBUG
			if (open_mode_traits<OpenMode>::is_read_only) {
				assert(t == io_mode::input);
			}
			else if (open_mode_traits<OpenMode>::is_write_only) {
				assert(t == io_mode::output);
			}
		#endif
		return m_strat.preferred_constraints(t);
	}

	ENABLE_IF_HAS_READ_ACCESS ibuffer_type
	allocate_ibuffer(const buffer_constraints& bc)
	{
		assert(m_strat.read_method());
		assert(bc.satisfies(required_constraints(io_mode::input)));

		if (m_strat.read_method() == io_method::mmap) {
			return buffer{io_mode::input, m_mapped, m_mapped_size};
		}
		else {
			return buffer{bc};
		}
	}

	ENABLE_IF_HAS_WRITE_ACCESS obuffer_type
	allocate_obuffer(const buffer_constraints& bc)
	{
		assert(m_strat.write_method());
		assert(bc.satisfies(required_constraints(io_mode::output)));

		if (m_strat.write_method() == io_method::mmap) {
			return buffer{io_mode::output, m_mapped, m_mapped_size};
		}
		else {
			return buffer{bc};
		}
	}

	ENABLE_IF_HAS_READ_WRITE_ACCESS iobuffer_type
	allocate_iobuffer(const buffer_constraints& bc)
	{
		assert(m_strat.read_method());
		assert(m_strat.write_method());
		assert(supports_dual_use_buffers());
		assert(bc.satisfies(required_constraints(io_mode::input | io_mode::output)));

		if (m_strat.read_method() == io_method::mmap) {
			if (m_strat.write_method() == io_method::mmap) {
				return buffer{io_mode::input | io_mode::output,
					m_mapped, m_mapped_size};
			}
			else {
				throw std::runtime_error{"Fused IO buffer not allowed."};
			}
		}
		else {
			if (m_strat.write_method() == io_method::mmap) {
				throw std::runtime_error{"Fused IO buffer not allowed."};
			}
			else {
				return buffer{bc};
			}
		}
	}

	ENABLE_IF_HAS_READ_ACCESS cc::expected<void>
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

	ENABLE_IF_HAS_WRITE_ACCESS cc::expected<void>
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
