/*
** File Name: error_state.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZEF4C5231_1876_4C70_A554_ADA3CB8EA942
#define ZEF4C5231_1876_4C70_A554_ADA3CB8EA942

#include <utility>
#include <vector>
#include <boost/range/iterator_range.hpp>

namespace neo {

template <class Record>
class error_state
{
	using record_list = std::vector<Record>;
	using size_type = typename record_list::size_type;
	using iterator = typename record_list::const_iterator;
	using range = boost::iterator_range<iterator>;

	record_list m_records{};
public:
	explicit error_state() {}

	range records() const
	{
		return boost::make_iterator_range(
			m_records.cbegin(), m_records.cend()
		);
	}

	const size_type record_count() const
	{ return m_records.size(); }

	const Record& record(size_type n) const
	{ return m_records[n]; }

	template <class... Args>
	void push_record(Args&&... args)
	{ m_records.emplace_back(std::forward<Args>(args)...); }

	/*
	** TODO `pop_record`
	*/
};

}

#endif
