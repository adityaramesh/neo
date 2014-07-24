/*
** File Name: basic_error_state.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
**
** This class is responsible for maintaining the log records produced by the
** thread performing the IO. While `pop_record()` is concurrency-safe,
** `push_record` is not.
*/

#ifndef ZEF4C5231_1876_4C70_A554_ADA3CB8EA942
#define ZEF4C5231_1876_4C70_A554_ADA3CB8EA942

#include <array>
#include <utility>
#include <vector>
#include <boost/range/iterator_range.hpp>

namespace neo {

template <class Record>
class basic_error_state
{
	using record_list = std::vector<Record>;
	using size_type   = typename record_list::size_type;
	using iterator    = typename record_list::const_iterator;
	using range       = boost::iterator_range<iterator>;

	std::array<size_t, 5> m_rec_counts{};
	record_list m_records{};
public:
	explicit basic_error_state() {}

	range records() const
	{
		return boost::make_iterator_range(
			m_records.cbegin(), m_records.cend()
		);
	}

	size_type record_count() const
	{ return m_records.size(); }

	size_type record_count(severity s) const
	{
		switch (s) {
		case severity::debug:    return m_rec_counts[0];
		case severity::info:     return m_rec_counts[1];
		case severity::warning:  return m_rec_counts[2];
		case severity::error:    return m_rec_counts[3];
		case severity::critical: return m_rec_counts[4];
		}
	}

	const Record& record(size_type n) const
	{ return m_records[n]; }

	template <class... Args>
	void push_record(severity s, Args&&... args)
	{
		switch (s) {
		case severity::debug:    ++m_rec_counts[0]; break;
		case severity::info:     ++m_rec_counts[1]; break;
		case severity::warning:  ++m_rec_counts[2]; break;
		case severity::error:    ++m_rec_counts[3]; break;
		case severity::critical: ++m_rec_counts[4]; break;
		}
		m_records.emplace_back(s, std::forward<Args>(args)...);
	}

	/*
	** TODO `pop_record`
	*/
};

}

#endif
