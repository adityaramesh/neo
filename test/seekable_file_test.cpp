/*
** File Name: seekable_file_test.cpp
** Author:    Aditya Ramesh
** Date:      07/10/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/unit_test.hpp>

/*
#include <neo/core/file/seekable_file.hpp>

module("test allocation")
{
	namespace file = neo::file;
	using neo::access_mode::sequential;
	using neo::io_mode::input;
	using neo::io_mode::output;
	using file::open_mode::read;
	using file::open_mode::create_if_not_exists;
	using file::open_mode::modify;

	constexpr auto path = "data/text/moby_dick.txt";
	auto st = file::safe_stat(path).get();

	auto s1 = file::io_strategy(st.st_size, boost::none, st.st_blksize)
		.set_defaults<read, sequential>();
	auto s2 = file::io_strategy(st.st_size, boost::none, st.st_blksize)
		.set_defaults<create_if_not_exists, sequential>();
	auto s3 = file::io_strategy(st.st_size, boost::none, st.st_blksize)
		.set_defaults<modify, sequential>();

	auto f1 = file::seekable_file<read>{path, s1};
	auto f2 = file::seekable_file<create_if_not_exists>{path, s2};
	auto f3 = file::seekable_file<modify>{path, s3};

	auto c1 = f1.preferred_constraints(input);
	auto c2 = f2.preferred_constraints(output);
	auto c3 = f3.preferred_constraints(input | output);

	auto b1 = f1.allocate_ibuffer(c1);
	auto b2 = f2.allocate_obuffer(c2);
	require(b1.size() > 0);
	require(b2.size() > 0);

	if (f3.supports_dual_use_buffers()) {
		auto b3 = f3.allocate_iobuffer(c3);
		require(b3.size() > 0);
	}
}
*/

module("test read")
{

}

suite("Tests the seekable_file class.")
