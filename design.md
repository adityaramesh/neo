<!--
  ** File Name: design.md
  ** Author:    Aditya Ramesh
  ** Date:      06/29/2014
  ** Contact:   _@adityaramesh.com
-->

# Design

## Concepts

- Device:
    - Traits:
        - `handle`
	- `category`
	- `offset_type`
	- `ibuffer_type`
	- `obuffer_type`
	- `iobuffer_type`
    - Member variables:
        - `preferred_constraints`
	- `required_constraints`
    - Member functions:
        - `required_constraints(io_type)`
	- `preferred_constraints(io_type)`
    	- `allocate_ibuffer(bc)` (if appropriate)
	- `allocate_obuffer(bc)` (if_appropriate)
	- `allocate_iobuffer(bc)` (if_appropriate)
        - If non-seekable:
            - If input device:
	    	`expected<void> read(size_t n, const buffer& buf);`
            - If output device:
	    	`expected<void> write(size_t n, const buffer& buf);`
        - If seekable:
            - If input device:
	    	`expected<void> read(offset_type off, size_t n, const buffer& buf);`
            - If output device:
	    	`expected<void> write(offset_type off, size_t n, const buffer& buf);`
    - Models:
        - file
        - gpu_buffer (?)

- Buffer:
    - `data` (returns `uint8_t*`)
    - `readable`
    - `writable`
    - `mapped`
    - `resizable`
    - `resize(bc)` (optional)

<!---
**
** UNIMPLEMENTED
**
** - Deserializer (converts character buffers into objects):
**     - Traits:
**         - `character_type`
** 	- `value_type`
** 	- `is_incremental` (i.e. does reads and writes partially, because it
** 	cannot determine the size of successive elements in advance)
**     - Functions:
**         - `buffer_constraints preferred_constraints()`
**         - `buffer_constraints required_constraints()`
** 	- Non-incremental deserializer:
** 	    - `optional<value_type> deserialize(const pointer p, size_t n, operation_state& is, buffer_state& bs)`
** 		- Reads the next chunk
**         - Incremental deserializer:
**             - `void deserialize_partial(const pointer p, size_t n, operation_state& is, buffer_state& bs)`
** 		- The class will **not** automatically begin deserializing the
** 		next object using the extra characters.
** 	    - `optional<value_type> get()`
** 		- Returns the previously deserialized object.
** 
** - Serializer (writes objects to character buffers):
**     - Traits:
**         - `character_type`
** 	- `value_type`
** 	- `is_incremental`
** 	- `is_direct` (i.e. the `value_type` can be converted directly into a
** 	pointer for IO)
**     - Functions:
**         - `buffer_constraints preferred_constraints()`
**         - `buffer_constraints required_constraints()`
**         - Non-incremental deserializer:
** 	    - `void serialize_copy(const T& t, const pointer p, size_t n, operation_state& is, buffer_state& bs)`
** 	    - If direct serialization supported:
** 	        - `optional<pointer> serialize_direct(const T& t, operation_state& ic)`
** 	- Incremental serializer:
** 	    - `void serialize_copy_partial(const T& t, const pointer p, size_t n, operation_state& is, buffer_state& bs)`
** 	    - If direct serialization supported:
** 	        - `optional<pointer> serialize_direct(const T& t, operation_state& ic)`
** 		- It does not make sense to support partial direct
** 		serialization
** 
** - Header reader (model of incremental deserializer, since it returns parameter
** object describing the file format):
** 
** - Header writer (model of serializer):
-->

## Classes

- `buffer_constraints`:
    - Member variables:
        - `at_least`
        - `at_most`
        - `multiple_of`
        - `align_to`
    - Member functions:
        - `bool satisfies(const buffer_constraints& bc)`
    - Global functions:
        - `min_size`
	- `max_size`
	- `merge_weak`
        - `merge_strong`

- `io_strategy`
    - Global functions:
        - `default_io_strategy`
	- `apply`
	- `open`

- `file_buffer`

## Functions

- Device-to-device copy:
- TODO: implement in `neo/core/copy.hpp`.
- Make sure that the categories of the given devices are compatible. I.e. we
must be able to read from `src` and write to `dst`.

	template <class SeekableDevice1, class SeekableDevice2>
	cc::expected<void>
	copy(
		SeekableDevice1& src,
		typename SeekableDevice1::offset_type src_off,
		SeekableDevice2& dst,
		typename SeekableDevice2::offset_type dst_off,
		size_t count
	)

- Also implement analogous functions for non-seekable devices which do not use
the offset parameters.

## Enumerations

- `mode`:
    - `input`
    - `output`
    - `bidirectional`
    - `input-seekable`
    - `output-seekable`
    - `seekable`
    - `dual-seekable`
    - `bidirectional-seekable`

- `access_type`:
    - `sequential`
    - `random`

- `open_mode`:
    - `read`
    - `replace`
    - `modify`

<!---
**
** UNIMPLEMENTED
** 
** - `operation_status`
**     - `working`: The last element is still being processed.
**     - `done` The last element was successfully processed, though there may have
**     been warnings or log messages.
**     - `recoverable_error`: The last element was not successfully processed, but
**     serialization/deserialization can continue.
**     - `fatal_error`: The last element was not successfully processed, and the
**     resulting error is irrecoverable.
** UNIMPLEMENTED
**
** - `severity`:
**     - `debug`
**     - `info`
**     - `warning`
**     - `error`
** 
** `operation_state`
**     - Responsible for maintaining IO state for serializers and deserializers.
**     - Stores list of log messages, each associated with a severity.
**     - `io_status status() const`: Returns the status of the last IO operation.
**     - `void log(const char* msg, severity s)`
** 
** - `buffer_state`
**     - `size_t consumed()`
** 	- Number of bytes from the previous buffer that were consumed (i.e. read
** 	or written to).
**     - `size_t next()`
**         - Recommended size of next buffer.
-->
