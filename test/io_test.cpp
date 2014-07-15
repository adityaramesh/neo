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
	using namespace neo;
	namespace file = neo::file;
	using file::open_mode;
	constexpr auto path = "data/text/moby_dick.txt";

	auto s = file::strategy<io_mode::input>{path};
	s.infer_defaults(access_mode::random);

	auto h = file::open<open_mode::read>(path, s).move();
	auto b = file::allocate_ibuffer(h, s);
	
	file::read(h, 0, 64, b, s);
	require(std::memcmp(b.data(), str1, 64) == 0);

	file::read(h, s.current_file_size().get() - 64, 64, b, s);
	require(std::memcmp(b.data(), str2, 64) == 0);
}

suite("Tests the file IO functionality.")
