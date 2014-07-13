/*
** File Name: allocate_test.cpp
** Author:    Aditya Ramesh
** Date:      07/11/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/format.hpp>
#include <ccbase/unit_test.hpp>
#include <neo/core/file/allocate.hpp>

module("test allocate")
{
	namespace file = neo::file;

	using neo::access_mode::sequential;
	using neo::access_mode::random;

	using neo::io_mode::input;
	using neo::io_mode::output;

	using file::open_mode::read;
	using file::open_mode::create_if_not_exists;
	using file::open_mode::create_or_replace;
	using file::open_mode::replace_if_exists;
	using file::open_mode::modify;

	auto path = "data/text/moby_dick.txt";

	auto s1 = file::strategy<input>{path}.infer_defaults(sequential);
	auto h1 = file::open<read>(path, s1).move();
	auto b1 = file::allocate_ibuffer(h1, s1);
	require(b1.size() > 0);

	auto s2 = file::strategy<input>{path}.infer_defaults(random);
	auto h2 = file::open<read>(path, s2).move();
	auto b2 = file::allocate_ibuffer(h2, s2);
	require(b2.size() > 0);

	auto s3 = file::strategy<input | output>{path}.infer_defaults(sequential);
	auto h3 = file::open<modify>(path, s3).move();
	auto b3 = file::allocate_ibuffer(h3, s3);
	require(b3.size() > 0);

	auto s4 = file::strategy<input | output>{path}.infer_defaults(random);
	auto h4 = file::open<modify>(path, s4).move();
	auto b4 = file::allocate_ibuffer(h4, s4);
	require(b4.size() > 0);
}

suite("Tests the allocate header.")
