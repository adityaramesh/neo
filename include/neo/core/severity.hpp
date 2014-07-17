/*
** File Name: severity.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
**
** This enum is intended to describe the severity of log records that are
** produced during IO. The intended usage is as follows:
**
**   - `debug`: Information that would ordinarily only be useful to developers
**   of the library.
**   - `info`: Information during IO that the programmer might find useful when
**   debugging.
**   - `warning`: Used to indicate bad or dangerous practice, or minor technical
**   violations.
**   - `error`: Used to indicate errors that are likely recoverable.
**   - `critical`: Used to indicate errors that are likely to cause further IO
**   operations to fail.
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
