<!--
  ** File Name: design.md
  ** Author:    Aditya Ramesh
  ** Date:      06/29/2014
  ** Contact:   _@adityaramesh.com
-->

# Design

## Concepts

- Deserializer (converts character buffers into objects):
    - Traits:
	- `value_type`
	- `is_incremental` (i.e. does reads and writes partially, because it
	cannot determine the size of successive elements in advance)
    - Functions:
        - `buffer_constraints preferred_constraints()`
        - `buffer_constraints required_constraints()`
	- Non-incremental deserializer:
	    - `optional<value_type> deserialize(const pointer p, size_t n, operation_state& is, buffer_state& bs)`
		- Reads the next chunk
        - Incremental deserializer:
            - `void deserialize_partial(const pointer p, size_t n, operation_state& is, buffer_state& bs)`
		- The class will **not** automatically begin deserializing the
		next object using the extra characters.
	    - `optional<value_type> get()`
		- Returns the previously deserialized object.

- Serializer (writes objects to character buffers):
    - Traits:
	- `value_type`
	- `is_incremental`
	- `is_direct` (i.e. the `value_type` can be converted directly into a
	pointer for IO)
    - Functions:
        - `buffer_constraints preferred_constraints()`
        - `buffer_constraints required_constraints()`
        - Non-incremental deserializer:
	    - `void serialize_copy(const T& t, const pointer p, size_t n, operation_state& is, buffer_state& bs)`
	    - If direct serialization supported:
	        - `optional<pointer> serialize_direct(const T& t, operation_state& ic)`
	- Incremental serializer:
	    - `void serialize_copy_partial(const T& t, const pointer p, size_t n, operation_state& is, buffer_state& bs)`
	    - If direct serialization supported:
	        - `optional<pointer> serialize_direct(const T& t, operation_state& ic)`
		- It does not make sense to support partial direct
		serialization

- Header reader (model of incremental deserializer, since it returns parameter
object describing the file format):

- Header writer (model of serializer):


## Classes

## Enumerations

- `operation_status`
    - `working`: The last element is still being processed.
    - `done` The last element was successfully processed, though there may have
    been warnings or log messages.
    - `recoverable_error`: The last element was not successfully processed, but
    serialization/deserialization can continue.
    - `fatal_error`: The last element was not successfully processed, and the
    resulting error is irrecoverable.

- `severity`:
    - `debug`
    - `info`
    - `warning`
    - `error`
    - `critical`

- `context`:
    - Object that tracks location in file at which the error occurred.
    - Dependent on the particular file format (e.g. for CSVs, it would consist
    of the line and column number).
    - Function to return string representation.

- `operation_state`
    - Responsible for maintaining IO state for serializers and deserializers.
    - Stores list of log messages, each associated with a severity.
    - `io_status status() const`: Returns the status of the last IO operation.
    - `void log(severity s, const context& c, const char* msg)`

- `buffer_state`
    - `size_t consumed()`
	- Number of bytes from the previous buffer that were consumed (i.e. read
	or written to).
    - `size_t next()`
        - Recommended size of next buffer.
