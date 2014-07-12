/*
** File Name: handle_test.cpp
** Author:    Aditya Ramesh
** Date:      07/11/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/format.hpp>
#include <ccbase/unit_test.hpp>
#include <neo/core/file/handle.hpp>

module("test open")
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
	auto s2 = file::strategy<input | output>{path}.infer_defaults(sequential);
	auto h2 = file::open<modify>(path, s2).move();
}

suite("Tests the handle class.")
