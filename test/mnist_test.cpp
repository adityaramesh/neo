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
	strat.infer_defaults(access_mode::random);

	auto h = file::open<open_mode::read>(path, strat).move();
	auto buf = file::allocate_ibuffer(h, strat);

	auto is = mnist::image_io_state{};
	auto es = mnist::error_state{};
	auto bs = mnist::make_image_buffer_state();
	
	file::read(h, 0, buf.size(), buf, strat);
	auto s = mnist::read_header(buf.data(), buf.size(), is, bs, es);
	require(!!(s & operation_status::success));

	s = mnist::deserialize(
		buf.data() + bs.consumed(),
		buf.size() + bs.consumed(),
		is, bs, es
	);
	require(!!(s & operation_status::success));

	auto m = is.element();
	cc::println(m);
}

suite("Tests the MNIST IO facilities.")
