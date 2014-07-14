/*
** File Name: buffer_constraints_test.cpp
** Author:    Aditya Ramesh
** Date:      06/30/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/unit_test.hpp>
#include <neo/core/buffer_constraints.hpp>

module("test min_size")
{
	using boost::none;

	auto bc1 = neo::buffer_constraints{none, 10, none, none};
	require(min_size(bc1) == none);

	auto bc2 = neo::buffer_constraints{10, 10, 10, none};
	require(*min_size(bc2) == 10);

	auto bc3 = neo::buffer_constraints{10, 20, 10, none};
	require(*min_size(bc3) == 10);

	auto bc4 = neo::buffer_constraints{10, 20, 4, none};
	require(*min_size(bc4) == 12);
}

module("test max_size")
{
	using boost::none;

	auto bc1 = neo::buffer_constraints{10, none, none, none};
	require(max_size(bc1) == none);

	auto bc2 = neo::buffer_constraints{10, 10, 10, none};
	require(*max_size(bc2) == 10);

	auto bc3 = neo::buffer_constraints{10, 20, 10, none};
	require(*max_size(bc3) == 20);

	auto bc4 = neo::buffer_constraints{10, 20, 4, none};
	require(*max_size(bc4) == 20);
}

module("test merge_strong")
{
	using boost::none;

	auto a1 = neo::buffer_constraints{10, 40, none, none};
	auto a2 = neo::buffer_constraints{20, 30, none, none};
	auto a3 = merge_strong(a1, a2);
	require(*a3 == a2);

	auto b1 = neo::buffer_constraints{10, 20, none, none};
	auto b2 = neo::buffer_constraints{30, 40, none, none};
	auto b3 = merge_strong(b1, b2);
	require(!b3);

	auto c1 = neo::buffer_constraints{none, none, 5, none};
	auto c2 = neo::buffer_constraints{none, none, 7, none};
	auto c3 = merge_strong(c1, c2);
	require(c3.get().multiple_of().get() == 35);

	auto d1 = neo::buffer_constraints{none, 10, 5, none};
	auto d2 = neo::buffer_constraints{none, 10, 7, none};
	auto d3 = merge_strong(d1, d2);
	require(!d3);
}

module("test merge_weak")
{
	using boost::none;

	auto a1 = neo::buffer_constraints{10, 40, none, none};
	auto a2 = neo::buffer_constraints{20, 30, none, none};
	auto a3 = merge_weak(a1, a2);
	require(a3 == a2);

	auto b1 = neo::buffer_constraints{10, 20, none, none};
	auto b2 = neo::buffer_constraints{30, 40, none, none};
	auto b3 = merge_weak(b1, b2);
	require(b3 == b1);

	auto c1 = neo::buffer_constraints{none, none, 5, none};
	auto c2 = neo::buffer_constraints{none, none, 7, none};
	auto c3 = merge_weak(c1, c2);
	require(*c3.multiple_of() == 35);

	auto d1 = neo::buffer_constraints{none, 10, 5, none};
	auto d2 = neo::buffer_constraints{none, 10, 7, none};
	auto d3 = merge_weak(d1, d2);
	require(d3 == d1);

	auto e1 = neo::buffer_constraints{10, 50, 5, none};
	auto e2 = neo::buffer_constraints{60, 70, 7, none};
	auto e3 = neo::buffer_constraints{10, 50, 35, none};
	auto e4 = merge_weak(e1, e2);
	require(e4 == e3);
}

module("test satisfies")
{
	using boost::none;

	auto a1 = neo::buffer_constraints{10, 40, none, none};
	auto a2 = neo::buffer_constraints{20, 30, none, none};
	require(!a1.satisfies(a2));
	require(a2.satisfies(a1));

	auto b1 = neo::buffer_constraints{10, 20, none, none};
	auto b2 = neo::buffer_constraints{30, 40, none, none};
	require(!b1.satisfies(b2));
	require(!b2.satisfies(b1));

	auto c1 = neo::buffer_constraints{none, none, 5, none};
	auto c2 = neo::buffer_constraints{none, none, 10, none};
	require(!c1.satisfies(c2));
	require(c2.satisfies(c1));
}

suite("Tests the buffer_constraints class.")
