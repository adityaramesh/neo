/*
** File Name: io_strategy_test.cpp
** Author:    Aditya Ramesh
** Date:      07/02/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/unit_test.hpp>
#include <neo/core/file/io_strategy.hpp>
#include <neo/core/file/system.hpp>

module("tests default_io_strategy")
{
	namespace file = neo::file;
	using neo::access_mode::sequential;
	using file::open_mode::read;

	constexpr auto path = "data/text/moby_dick.txt";
	auto st = file::safe_stat(path).get();
	auto strat = file::io_strategy(st.st_size, boost::none, st.st_blksize)
		.set_defaults<read, sequential>();
	auto fd = file::open<read>(path, strat).get();
	file::safe_close(fd).get();
	cc::println(strat);
}

suite("Tests the io_strategy class.")
