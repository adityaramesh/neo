/*
** File Name: log_record_test.cpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/unit_test.hpp>
#include <neo/core/log_record.hpp>
#include <neo/core/error_state.hpp>

module("test log record")
{
	using namespace neo;
	using record = log_record<with_severity, with_message>;
	auto r = record{severity::info, "This is a test."};
	cc::println(r);
}

module("test error state")
{
	using namespace neo;
	using record = log_record<with_severity, with_message>;
	using state = error_state<record>;

	auto s = state{};
	s.emplace_record(severity::info, "This is a test.");
}

suite("Tests the core logging utilities.")
