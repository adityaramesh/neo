/*
** File Name: handle.hpp
** Author:    Aditya Ramesh
** Date:      07/10/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z7C27BC1A_7DE8_46D3_8F94_9C61EE88C34C
#define Z7C27BC1A_7DE8_46D3_8F94_9C61EE88C34C

#include <neo/core/file/open_mode.hpp>
#include <neo/core/file/strategy.hpp>
#include <neo/core/file/system.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX || \
    PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
	#include <sys/mman.h>
#else
	#error "Unsupported kernel."
#endif

namespace neo {
namespace file {

template <io_mode IOMode>
class handle
{
	static constexpr auto prot_flags =
	(IOMode == io_mode::input ?
		PROT_READ :
		(IOMode == io_mode::output ?
		 	PROT_WRITE :
			PROT_READ | PROT_WRITE
		)
	);

	uint8_t* m_map{nullptr};
	size_t m_map_size{};
	int m_fd{-1};
public:
	explicit handle() noexcept {}
	explicit handle(int fd) noexcept : m_fd{fd} {}

	explicit handle(int fd, size_t size)
	: m_map_size{size}, m_fd{fd}
	{
		m_map = (uint8_t*)::mmap(nullptr, m_map_size, prot_flags,
			MAP_SHARED, m_fd, 0);
		if (m_map == (uint8_t*)-1) {
			m_map = nullptr;
			throw current_system_error();
		}
	}

	handle(const handle&) = delete;

	handle(handle&& rhs) noexcept :
	m_map{rhs.m_map}, m_fd{rhs.m_fd}
	{
		rhs.m_map = nullptr;
		rhs.m_fd = -1;
	}

	handle& operator=(const handle&) = delete;

	handle& operator=(handle&& rhs)
	{
		m_map = rhs.m_map;
		m_fd = rhs.m_fd;
		rhs.m_map = nullptr;
		rhs.m_fd = -1;
		return *this;
	}

	~handle()
	{
		close();
		unmap();
	}

	bool mapped() const { return m_map != nullptr; }

	size_t map_size() const
	{
		assert(mapped());
		return m_map_size;
	}

	uint8_t* map() const
	{
		assert(mapped());
		return m_map;
	}

	int descriptor() const
	{
		assert(m_fd != -1);
		return m_fd;
	}

	handle& close()
	{
		if (m_fd != -1) {
			*safe_close(m_fd);
		}
		return *this;
	}

	handle& unmap()
	{
		if (m_map != nullptr && ::munmap(m_map, m_map_size) == -1) {
			throw current_system_error();
		}
		return *this;
	}
};

/*
** Returns a handle to the file at corresponding path, and applies the
** optimizations described by the given `strategy` object.
*/
template <open_mode OpenMode, io_mode IOMode>
cc::expected<handle<IOMode>>
open(const char* path, const strategy<IOMode>& s)
noexcept
{
	constexpr auto flags = to_posix<OpenMode>::value;
	auto fd = int{};

	/*
	** Check to ensure that the `io_mode` agrees with the `open_mode`.
	*/
	if (open_mode_traits<OpenMode>::has_read_access) {
		assert(!!(IOMode & io_mode::input));
	}
	if (open_mode_traits<OpenMode>::has_write_access) {
		assert(!!(IOMode & io_mode::output));
	}

	/*
	** Open the file descriptor using the appropriate flags.
	*/
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
		if (
			(!!(*s.read_method() & io_method::direct) &&
			!!(IOMode & io_mode::input)) ||
			((*s.write_method() & io_method::direct) &&
			!!(IOMode & io_mode::output))
		) {
			auto r = safe_open(path, flags | O_DIRECT);
			if (!r) { return r.exception(); }
			else { fd = *r; }
		}
		else {
			auto r = safe_open(path, flags);
			if (!r) { return r.exception(); }
			else { fd = *r; }
		}
	#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		auto r = safe_open(path, flags);
		if (!r) { return r.exception(); }
		else { fd = *r; }
	#endif
	
	/*
	** Apply the IO optimizations to the handle.
	*/
	#if PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
		if (!!(*s.read_method() & io_method::direct)) {
			auto r1 = safe_nocache(fd);
			if (!r1) {
				auto r2 = safe_close(fd);
				if (!r2) { return r2.exception(); }
				return r1.exception();
			}
		}
	#endif

	if (!!(IOMode & io_mode::input)) {
		if (s.read_ahead()) {
			#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
				auto r1 = safe_fadvise_sequential(fd, *s.current_file_size());
				if (!r1) {
					auto r2 = safe_close(fd);
					if (!r2) { return r2.exception(); }
					return r1.exception(); }
				}
			#elif PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
				if ((uint64_t)*s.current_file_size() < 256_MB) {
					auto r1 = safe_rdadvise(fd, *s.current_file_size());
					if (!r1) {
						auto r2 = safe_close(fd);
						if (!r2) { return r2.exception(); }
						return r1.exception();
					}
				}
				else {
					auto r1 = safe_rdahead(fd);
					if (!r1) {
						auto r2 = safe_close(fd);
						if (!r2) { return r2.exception(); }
						return r1.exception();
					}
				}
			#endif
		}
	}
	if (!!(IOMode & io_mode::output)) {
		if (s.preallocate() && s.maximum_file_size()) {
			auto r1 = safe_preallocate(fd, *s.maximum_file_size());
			if (!r1) {
				auto r2 = safe_close(fd);
				if (!r2) { return r2.exception(); }
				return r1.exception();
			}
		}
	}

	if (s.write_method() && !!(*s.write_method() & io_method::mmap)) {
		return handle<IOMode>{fd, (size_t)*s.maximum_file_size()};
	}
	else if (s.read_method() && !!(*s.read_method() & io_method::mmap)) {
		return handle<IOMode>{fd, (size_t)*s.current_file_size()};
	}
	else {
		return handle<IOMode>{fd};
	}
}

}}

#endif
