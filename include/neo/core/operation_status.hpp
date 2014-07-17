/*
** File Name: operation_status.hpp
** Author:    Aditya Ramesh
** Date:      07/14/2014
** Contact:   _@adityaramesh.com
**
** The `operation_status` enum is used to summarize the changes that were
** brought about by the previous IO transaction. These changes can be put into
** three categories:
**
**   - Serialization/deserialization status: the `in_progress`, `success`, and
**   `failure` flags are used to indicate the progress in serializing or
**   deserializing the current object.
**
**   - Error status: the `recoverable_error` and `fatal_error` flags are set iff
**   the `failure` flag is set. They are used to indicate the consequences of
**   the previous error.
**
**   - Buffer update stats: the `req_constr_update` and `pref_constr_update`
**   flags are used to indicate that the required and preferred buffer
**   constraints for serialization or deserialization have changed. In general,
**   it is inadvisable to resize buffers during IO, though it may be worth doing
**   so if the file being read or written to is very large.
*/

#ifndef ZA8E5AEB9_8701_4C48_8EA4_DB2078AB3558
#define ZA8E5AEB9_8701_4C48_8EA4_DB2078AB3558

#include <neo/utility/bitmask_enum.hpp>

namespace neo {

enum class operation_status : unsigned
{
	incomplete         = 0x01,
	success            = 0x02,
	failure            = 0x04,
	recoverable_error  = 0x08,
	fatal_error        = 0x10,
	req_constr_update  = 0x20,
	pref_constr_update = 0x40,
};

DEFINE_ENUM_BITWISE_OPERATORS(operation_status)

}

#endif
