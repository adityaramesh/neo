/*
** File Name: io_strategy_test.cpp
** Author:    Aditya Ramesh
** Date:      07/02/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/unit_test.hpp>
#include <neo/core/io_common.hpp>
#include <neo/core/io_strategy.hpp>

module("tests default_io_strategy")
{
	using neo::open_mode::read;
	using neo::access_mode::sequential;

	constexpr auto path = "data/text/moby_dick.txt";
	auto st = neo::default_io_strategy<read, sequential>(path);
	auto fd = neo::open<read>(path, st).get();
	neo::safe_close(fd).get();
	cc::println(st);
}

suite("Tests the io_strategy class.")
