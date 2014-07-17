/*
** File Name: error_state_test.cpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/unit_test.hpp>
#include <neo/core/basic_log_record.hpp>
#include <neo/core/basic_error_state.hpp>

module("test log record")
{
	using namespace neo;
	using record = basic_log_record<with_severity, with_message>;
	auto r = record{severity::info, "This is a test."};
	cc::println(r);
}

module("test error state")
{
	using namespace neo;
	using record = basic_log_record<with_severity, with_message>;
	using state = basic_error_state<record>;

	auto s = state{};
	s.push_record(severity::info, "This is a test.");
}

suite("Tests the core logging facilities.")
