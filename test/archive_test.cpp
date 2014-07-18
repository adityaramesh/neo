/*
** File Name: archive_test.cpp
** Author:    Aditya Ramesh
** Date:      07/17/2014
** Contact:   _@adityaramesh.com
*/

#include <typeinfo>
#include <ccbase/format.hpp>
#include <ccbase/unit_test.hpp>
#include <neo/core/file.hpp>
#include <neo/io/archive.hpp>

module("test traits")
{
	using namespace neo;
	using namespace archive;

	using t1 = double;
	using t2 = archive::vector<double, 10>;
	using t3 = archive::matrix<double, 10, 10, storage_order::row_major>;
	using t4 = std::tuple<t1, t2, t3>;

	/*
	** Expected header size:
	**   - Header size: 1 + 1 + 1 = 3
	**   - Component 1: 1 + 1 = 2
	**   - Component 2: 1 + 1 + 4 = 6
	**   - Component 3: 1 + 1 + 1 + 4 + 4 = 11
	**   - Record count: 8
	**   - Total: 30
	*/
	require(header_size<t4>::value == 30);

	/*
	** Expected element size:
	**   - Component 1: sizeof(double).
	**   - Component 2: 10 * sizeof(double).
	**   - Component 3: 10 * 10 * sizeof(double).
	*/
	require(element_size<t4>::value == 111 * sizeof(double));
}

module("test deserialization")
{
	namespace file = neo::file;
	namespace archive = neo::archive;
	using namespace neo;
	using file::open_mode;
	using archive::storage_order;

	constexpr auto path = "data/archive/test.dsa";
	auto strat = file::strategy<io_mode::input>{path};
	strat.infer_defaults(access_mode::sequential);

	using n1 = double;
	using n2 = archive::vector<double, 37>;
	using n3 = archive::matrix<double, 13, 37, storage_order::row_major>;
	using n4 = archive::matrix<int16_t, 38, 53, storage_order::row_major>;
	using n5 = archive::vector<int8_t, 4>;
	using n6 = int32_t;
	using input_type = std::tuple<n1, n2, n3, n4, n5, n6>;

	auto h = file::open<open_mode::read>(path, strat).move();
	auto is = archive::io_state<input_type>{};
	auto es = archive::error_state{};
	auto bs = archive::make_buffer_state<input_type>();
	
	auto bc = *merge_strong(
		strat.preferred_constraints(io_mode::input),
		bs.preferred_constraints()
	);
	auto buf = file::allocate_ibuffer(h, strat, bc);
	file::read(h, 0, buf.size(), buf, strat).get();

	auto s = archive::read_header(buf.data(), buf.size(), is, bs, es);
	require(!!(s & operation_status::success));
	require(is.element_count() == 3);

	file::read(h, bs.consumed(), buf.size(), buf, strat).get();
	s = archive::deserialize(buf.data(), buf.size(), is, bs, es);
	require(!!(s & operation_status::success));

	auto t = is.element();
	require(std::get<0>(t) == 1.0);
	require(std::get<5>(t) == -1);

	auto a2 = std::get<1>(t);
	auto a3 = std::get<2>(t);
	auto a4 = std::get<3>(t);
	auto a5 = std::get<4>(t);

	for (auto i = 0; i != a2.size(); ++i) {
		require(a2(i) == i);
	}
	for (auto i = 0; i != a3.rows(); ++i) {
		for (auto j = 0; j != a3.cols(); ++j) {
			require(a3(i, j) == a3.cols() * i + j);
		}
	}
	for (auto j = 0; j != a4.cols(); ++j) {
		for (auto i = 0; i != a4.rows(); ++i) {
			require(a4(i, j) == a4.cols() * i + j);
		}
	}
	for (auto i = 0; i != a5.size(); ++i) {
		require(a5(i) == i);
	}
}

suite("Tests the archive IO facilities.")
