/*
** File Name: strategy_test.cpp
** Author:    Aditya Ramesh
** Date:      07/02/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/format.hpp>
#include <ccbase/unit_test.hpp>
#include <neo/core/file/strategy.hpp>

module("test construction")
{
	namespace file = neo::file;
	using neo::access_mode::sequential;
	using neo::access_mode::random;
	using neo::io_mode::input;
	using neo::io_mode::output;

	constexpr auto path = "data/text/moby_dick.txt";
	auto s1 = file::strategy<input>{path}.infer_defaults(sequential);
	auto s2 = file::strategy<output>{path}.infer_defaults(sequential);
	auto s3 = file::strategy<input | output>{path}.infer_defaults(sequential);
	auto s4 = file::strategy<input>{path}.infer_defaults(random);
	auto s5 = file::strategy<output>{path}.infer_defaults(random);
	auto s6 = file::strategy<input | output>{path}.infer_defaults(random);

	require(!s1.supports_dual_use_buffers());
	require(!s2.supports_dual_use_buffers());
	require(!s4.supports_dual_use_buffers());
	require(!s5.supports_dual_use_buffers());
}

suite("Tests the strategy class.")
