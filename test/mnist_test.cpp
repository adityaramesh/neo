/*
** File Name: mnist_test.hpp
** Author:    Aditya Ramesh
** Date:      07/16/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/format.hpp>
#include <ccbase/unit_test.hpp>
#include <neo/core/file.hpp>
#include <neo/io/mnist.hpp>

module("test read")
{
	namespace file = neo::file;
	namespace mnist = neo::mnist;
	using namespace neo;
	using file::open_mode;

	constexpr auto path = "data/mnist/train-images-idx3-ubyte";
	auto strat = file::strategy<io_mode::input>{path};
	strat.infer_defaults(access_mode::sequential);

	auto h = file::open<open_mode::read>(path, strat).move();
	auto is = mnist::image_io_state{};
	auto es = mnist::error_state{};
	auto bs = mnist::make_image_buffer_state();

	auto bc = *merge_strong(
		strat.preferred_constraints(io_mode::input),
		bs.preferred_constraints()
	);
	auto buf = file::allocate_ibuffer(h, strat, bc);
	file::read(h, 0, buf.size(), buf, strat).get();

	auto s = mnist::read_header(buf.data(), buf.size(), is, bs, es);
	require(!!(s & operation_status::success));

	file::read(h, bs.consumed(), buf.size(), buf, strat).get();
	s = mnist::deserialize(buf.data(), buf.size(), is, bs, es);
	require(!!(s & operation_status::success));

	auto m = is.element();
	cc::println(m);
}

suite("Tests the MNIST IO facilities.")
