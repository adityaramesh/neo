/*
** File Name: severity.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z9DEB5CCB_EFA5_4462_A4CC_241F86A1EBF2
#define Z9DEB5CCB_EFA5_4462_A4CC_241F86A1EBF2

#include <ostream>

namespace neo {

enum class severity : unsigned
{
	debug,
	info,
	warning,
	error,
	critical
};

std::ostream& operator<<(std::ostream& os, severity s)
{
	switch (s) {
	case severity::debug:    return os << "debug";
	case severity::info:     return os << "info";
	case severity::warning:  return os << "warning";
	case severity::error:    return os << "error";
	case severity::critical: return os << "critical";
	}
	return os;
}

}

#endif
