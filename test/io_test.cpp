/*
** File Name: io_test.cpp
** Author:    Aditya Ramesh
** Date:      07/12/2014
** Contact:   _@adityaramesh.com
*/

#include <cstring>
#include <ccbase/format.hpp>
#include <ccbase/unit_test.hpp>
#include <neo/core/file.hpp>

constexpr auto str1 = "**The Project Gutenberg Etext of Moby Dick, by Herman Melville**";
constexpr auto str2 = "his Project Gutenberg etext of Moby Dick, by Herman Melville\r\n\r\n";

module("test read")
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

	constexpr auto path = "data/text/moby_dick.txt";

	auto s = file::strategy<input>{path}.infer_defaults(random);
	auto h = file::open<read>(path, s).move();
	auto b = file::allocate_ibuffer(h, s);
	
	file::read(h, 0, 64, b, s);
	require(std::memcmp(b.data(), str1, 64) == 0);

	file::read(h, s.current_file_size().get() - 64, 64, b, s);
	require(std::memcmp(b.data(), str2, 61) == 0);
}

suite("Tests the file IO functionality.")
