/*
** File Name: buffer_constraints.hpp
** Author:    Aditya Ramesh
** Date:      06/30/2014
** Contact:   _@adityaramesh.com
**
** The size of the buffer can considerably influence the execution time of IO
** operations. See [this benchmark][io_benchmark] for more details. The
** `buffer_constraints` class allows each agent in an IO pipeline to express the
** following:
**
**   - Required features of the IO buffer.
**   - Optional features that may accelerate processing.
**
** If each agent in an IO pipeline is to use the same buffer processed by the
** previous agent, then we must be able to satisfy the constraints on the IO
** buffer imposed by all agents in the pipeline. Otherwise, we must copy data
** between buffers partway through the pipeline.
**
** The `buffer_constraints` is useful because of the following key operations
** that it allows us to perform:
**
**   - `merge_strong`: merge the two `buffer_constraints` objects into a single
**   object that satisfies both sets of constraints. This may not always be
**   possible.
**   - `merge_weak`: attempt to modify a given `buffer_constraints` object by
**   applying hints drawn from another `buffer_constraints` object, without
**   violating any of the constraints imposed by the first object. This
**   operation always succeeds, but it may not make any changes.
**
** [io_benchmark]: http://adityaramesh.com/io_benchmark
*/

#ifndef Z1691F69E_56F3_4173_8732_4BC556320E36
#define Z1691F69E_56F3_4173_8732_4BC556320E36

#include <cassert>
#include <cstdlib>
#include <ostream>
#include <boost/optional.hpp>
#include <boost/math/common_factor.hpp>

namespace neo {

/*
** Checks to ensure that the constraints are consistent, i.e. a feasible
** solution exists.
*/
bool is_consistent(
	boost::optional<size_t> at_least,
	boost::optional<size_t> at_most,
	boost::optional<size_t> multiple_of,
	boost::optional<size_t> align_to
)
{
	if (at_least) {
		if (at_least == 0) {
			return false;
		}
	}
	if (at_most) {
		if (at_least) {
			if (at_most < at_least) {
				return false;
			}
		}
		if (multiple_of) {
			if (at_most < multiple_of) {
				return false;
			}
		}
		else {
			if (at_most == 0) {
				return false;
			}
		}
	}
	if (multiple_of) {
		if (multiple_of == 0) {
			return false;
		}
		if (at_least && at_most) {
			auto l = at_least.get();
			auto m = multiple_of.get();
			if (l % m != 0) {
				if (l + m - l % m > at_most) {
					return false;
				}
			}
		}
	}
	if (align_to) {
		if (align_to == 0) {
			return false;
		}
	}
	return true;
}

class buffer_constraints
{
	boost::optional<size_t> m_at_least{};
	boost::optional<size_t> m_at_most{};
	boost::optional<size_t> m_multiple_of{};
	boost::optional<size_t> m_align_to{};
public:
	buffer_constraints(
		boost::optional<size_t> at_least    = boost::none,
		boost::optional<size_t> at_most     = boost::none,
		boost::optional<size_t> multiple_of = boost::none,
		boost::optional<size_t> align_to    = boost::none
	) noexcept : m_at_least{at_least}, m_at_most{at_most},
	m_multiple_of{multiple_of}, m_align_to{align_to}
	{ assert(is_consistent(m_at_least, m_at_most, m_multiple_of, m_align_to)); }

	decltype(m_at_least)    at_least()    const { return m_at_least;    }
	decltype(m_at_most)     at_most()     const { return m_at_most;     }
	decltype(m_multiple_of) multiple_of() const { return m_multiple_of; }
	decltype(m_align_to)    align_to()    const { return m_align_to;    }

	buffer_constraints& at_least(boost::optional<size_t> n)
	{
		m_at_least = n;
		assert(is_consistent(m_at_least, m_at_most, m_multiple_of, m_align_to));
		return *this;
	}

	buffer_constraints& at_most(boost::optional<size_t> n)
	{
		m_at_most = n;
		assert(is_consistent(m_at_least, m_at_most, m_multiple_of, m_align_to));
		return *this;
	}

	buffer_constraints& multiple_of(boost::optional<size_t> n)
	{
		m_multiple_of = n;
		assert(is_consistent(m_at_least, m_at_most, m_multiple_of, m_align_to));
		return *this;
	}

	buffer_constraints& align_to(boost::optional<size_t> n)
	{
		m_align_to = n;
		assert(is_consistent(m_at_least, m_at_most, m_multiple_of, m_align_to));
		return *this;
	}

	bool operator==(const buffer_constraints& rhs)
	{
		return m_at_least    == rhs.m_at_least    &&
		       m_at_most     == rhs.m_at_most     &&
		       m_multiple_of == rhs.m_multiple_of &&
		       m_align_to    == rhs.m_align_to;
	}

	bool operator!=(const buffer_constraints& rhs)
	{
		return !(*this == rhs);
	}
};

std::ostream& operator<<(std::ostream& os, const buffer_constraints& bc)
{
	os << "Buffer constraints:\n";

	os << " * At least: ";
	if (bc.at_least()) {
		os << bc.at_least().get() << "\n";
	}
	else {
		os << "none\n";
	}

	os << " * At most: ";
	if (bc.at_most()) {
		os << bc.at_most().get() << "\n";
	}
	else {
		os << "none\n";
	}

	os << " * Multiple of: ";
	if (bc.multiple_of()) {
		os << bc.multiple_of().get() << "\n";
	}
	else {
		os << "none\n";
	}

	os << " * Align to: ";
	if (bc.align_to()) {
		os << bc.align_to().get() << "\n";
	}
	else {
		os << "none\n";
	}
	return os;
}

/*
** Returns the minimum permissible size allowed by the given constraints.
*/
boost::optional<size_t>
min_size(const buffer_constraints& bc)
{
	if (!bc.at_least()) { return boost::none; }
	if (!bc.multiple_of()) { return bc.at_least(); }
	
	auto l = bc.at_least().get();
	auto m = bc.multiple_of().get();
	return l % m == 0 ? m : l + m - l % m;
}

/*
** Returns the maximum permissible size allowed by the given constraints.
*/
boost::optional<size_t>
max_size(const buffer_constraints& bc)
{
	if (!bc.at_most()) { return boost::none; }
	if (!bc.multiple_of()) { return bc.at_most(); }

	auto g = bc.at_most().get();
	auto m = bc.multiple_of().get();
	return g % m == 0 ? g : g - g % m;
}

/*
** Attempts to merge two `buffer_constraints` into one, such that both sets of
** constraints are satisfied.
*/
boost::optional<buffer_constraints>
merge_strong(const buffer_constraints& x, const buffer_constraints& y)
{
	auto at_least    = boost::optional<size_t>{};
	auto at_most     = boost::optional<size_t>{};
	auto multiple_of = boost::optional<size_t>{};
	auto align_to    = boost::optional<size_t>{};

	if (x.at_least()) {
		if (y.at_least()) {
			at_least = std::max(x.at_least().get(), y.at_least().get());
		}
		else {
			at_least = x.at_least();
		}
	}
	else if (y.at_least()) {
		at_least = y.at_least();
	}

	if (x.at_most()) {
		if (y.at_most()) {
			at_most = std::min(x.at_most().get(), y.at_most().get());
		}
		else {
			at_most = x.at_most();
		}
	}
	else if (y.at_most()) {
		at_most = y.at_most();
	}

	if (x.multiple_of()) {
		if (y.multiple_of()) {
			multiple_of = boost::math::lcm(
				x.multiple_of().get(),
				y.multiple_of().get()
			);
		}
		else {
			multiple_of = x.multiple_of();
		}
	}
	else if (y.multiple_of()) {
		multiple_of = y.multiple_of();
	}

	if (x.align_to()) {
		if (y.align_to()) {
			align_to = boost::math::lcm(
				x.align_to().get(),
				y.align_to().get()
			);
		}
		else {
			align_to = x.align_to();
		}
	}
	else if (y.align_to()) {
		align_to = y.align_to();
	}

	if (!is_consistent(at_least, at_most, multiple_of, align_to)) {
		return boost::none;
	}
	return buffer_constraints{at_least, at_most, multiple_of, align_to};
}

/*
** Attempts to incorporate the constraints described by `hint` with the
** constraints described by `bc`, without violating any of `bc`'s constraints.
** This operation always succeeds, though it may not change any of `bc`'s
** constraints.
*/
buffer_constraints
merge_weak(const buffer_constraints& bc, const buffer_constraints& hint)
{
	auto at_least    = boost::optional<size_t>{};
	auto at_most     = boost::optional<size_t>{};
	auto multiple_of = boost::optional<size_t>{};
	auto align_to    = boost::optional<size_t>{};

	/*
	** Try to maintain the `multiple_of` requirements above everything else,
	** since these tend to be the most important constraints.
	*/
	if (bc.multiple_of()) {
		if (hint.multiple_of()) {
			multiple_of = boost::math::lcm(
				bc.multiple_of().get(),
				hint.multiple_of().get()
			);
		}
		else {
			multiple_of = bc.multiple_of();
		}
	}
	else if (hint.multiple_of()) {
		multiple_of = hint.multiple_of();
	}

	/*
	** See if the `at_least` and `at_most` requirements imposed by `bc` are
	** compatible with our choice of `mulitple_of`. If not, revert the
	** choice of `multiple_of` to that of `bc`.
	*/
	if (bc.at_least()) {
		at_least = bc.at_least();
		if (bc.at_most()) {
			at_most = bc.at_most();
			if (multiple_of) {
				auto l = bc.at_least().get();
				auto g = bc.at_most().get();
				auto m = multiple_of.get();
				if (l % m != 0 && l + m - l % m > g) {
					multiple_of = bc.multiple_of();
				}
			}
		}
	}
	else if (bc.at_most()) {
		at_most = bc.at_most();
		if (multiple_of && multiple_of > at_most) {
			multiple_of = bc.multiple_of();
		}
	}

	/*
	** If `hint` imposes `at_least` or `at_most` constraints, try to
	** incorporate them with the other constraints.
	*/
	if (hint.at_least()) {
		if (at_least) {
			if (hint.at_least() > at_least) {
				if (at_most) {
					auto l = hint.at_least().get();
					auto g = at_most.get();
					if (l <= g) {
						if (multiple_of) {
							auto m = multiple_of.get();
							if (l % m == 0 || l + m - l % m <= g) {
								at_least = hint.at_least();
							}
						}
						else {
							at_least = hint.at_least();
						}
					}
				}
				else {
					at_least = hint.at_least();
				}
			}
		}
		else {
			at_least = hint.at_least();
		}
	}
	if (hint.at_most()) {
		if (at_most) {
			if (hint.at_most() < at_most) {
				if (at_least) {
					auto l = at_least.get();
					auto g = hint.at_most().get();
					if (l <= g) {
						if (multiple_of) {
							auto m = multiple_of.get();
							if (l % m == 0 || l + m - l % m <= g) {
								at_most = hint.at_most();
							}
						}
						else {
							at_most = hint.at_most();
						}
					}
				}
				else {
					at_most = hint.at_most();
				}
			}
		}
		else {
			at_most = hint.at_most();
		}
	}

	/*
	** The `align_to` constraints do not interact with any of the others, so
	** these are easy to deal with.
	*/
	if (bc.align_to()) {
		if (hint.align_to()) {
			align_to = boost::math::lcm(
				bc.align_to().get(),
				hint.align_to().get()
			);
		}
		else {
			align_to = bc.align_to();
		}
	}
	else if (hint.align_to()) {
		align_to = hint.align_to();
	}
	return buffer_constraints{at_least, at_most, multiple_of, align_to};
}

}

#endif
