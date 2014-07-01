<!--
  ** File Name: concepts.md
  ** Author:    Aditya Ramesh
  ** Date:      06/29/2014
  ** Contact:   _@adityaramesh.com
-->

# Concepts

- Mode:
    - Input
    - Output
    - Bidirectional
    - Input-seekable
    - Output-seekable
    - Seekable
    - Dual-seekable
    - Bidirectional-seekable

- Access type:
    - Sequential
    - Random

- Open mode:
    - Read
    - Create
    - Update

- Platform-specific optimizations for device:
    - Use optionals to store the optimizations, so that default choices can be
    made where appropriate.
    - The optimizations should be related to the open mode. Perhaps make three
    different optimization structures for each platform, one for each open mode.

- Device:
    - Traits:
	- mode
        - char_type
	- access_type
	- pointer
	- offset_type
    - Functions:
        - Constructor
	    - Accepts initial offset and size.
	    - Accepts default-initialized auxiliary struct to override default
	    optimizations with user-chosen ones (e.g. preallocation). This
	    struct is platform-specific.
	    - This struct includes the expected length of the file, for
	    preallocation.
        - `buffer_constraints preferred_constraints()`
	- `buffer_constraints required_constraints()`
	- IO:
            - Non-seekable device:
                - If input device: `expected<size_t> read(size_t cnt, buffer& buf);`
                - If output device: `expected<size_t> write(size_t cnt, buffer& buf);`
            - Seekable device:
                - If input device: `expected<size_t> read(offset_type off, size_t cnt, buffer& buf);`
                - If output device: `expected<size_t> write(offset_type off, size_t cnt, buffer& buf);`
    - Examples:
        - basic_file
        - basic_gpu_buffer (?)

- Global IO functions:
    - `cc::expected<void> copy(Device1& src, Device2& dst)`
        - Make sure that the modes of the given devices are compatible, i.e. we
        must be able to read from `src` and write to `dst`.

- `buffer`
    - `char_type`
    - Capacity
    - Size
    - Pointer to buffer (which implies alignment)

- `buffer_constraints`
    - Minimum size
    - Multiple of
    - Alignment

- Global buffer functions:
    - `buffer allocate(const buffer_constraints& bc)`
    - `cc::expected<buffer_constraints> strong_merge(const buffer_constraints& a, const buffer_constraints& b)`
        - Attempts to merge two `buffer_constraints` objects such that the
	constraints imposed by both objects are satisfied.
    - `buffer_constraints weak_merge(const buffer_constraints& a, const buffer_constraints& b)`
        - Attempts to merge two `buffer_constraints` objects by applying `b` as
	a hint that modifies `a`, without violating any of `a`'s existing
	constraints (e.g. alignment, size multiple).

- `io_status`
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

`io_state`
    - Responsible for maintaining IO state for serializers and deserializers.
    - Stores list of log messages, each associated with a severity.
    - `io_status status() const`: Returns the status of the last IO operation.
    - `void log(const char* msg, severity s)`

- `buffer_state`
    - `size_t consumed()`
	- Number of bytes from the previous buffer that were consumed (i.e. read
	or written to).
    - `size_t next()`
        - Recommended size of next buffer.

- Deserializer (converts character buffers into objects):
    - Traits:
        - `character_type`
	- `value_type`
	- `is_incremental` (i.e. does reads and writes partially, because it
	cannot determine the size of successive elements in advance)
    - Functions:
        - `buffer_constraints preferred_constraints()`
        - `buffer_constraints required_constraints()`
	- Non-incremental deserializer:
	    - `optional<value_type> deserialize(const pointer p, size_t n, io_state& is, buffer_state& bs)`
		- Reads the next chunk
        - Incremental deserializer:
            - `void deserialize_partial(const pointer p, size_t n, io_state& is, buffer_state& bs)`
		- The class will **not** automatically begin deserializing the
		next object using the extra characters.
	    - `optional<value_type> get()`
		- Returns the previously deserialized object.

- Serializer (writes objects to character buffers):
    - Traits:
        - `character_type`
	- `value_type`
	- `is_incremental`
	- `is_direct` (i.e. the `value_type` can be converted directly into a
	pointer for IO)
    - Functions:
        - `buffer_constraints preferred_constraints()`
        - `buffer_constraints required_constraints()`
        - Non-incremental deserializer:
	    - `void serialize_copy(const T& t, const pointer p, size_t n, io_state& is, buffer_state& bs)`
	    - If direct serialization supported:
	        - `optional<pointer> serialize_direct(const T& t, io_state& ic)`
	- Incremental serializer:
	    - `void serialize_copy_partial(const T& t, const pointer p, size_t n, io_state& is, buffer_state& bs)`
	    - If direct serialization supported:
	        - `optional<pointer> serialize_direct(const T& t, io_state& ic)`
		- It does not make sense to support partial direct
		serialization

- Header reader (model of incremental deserializer, since it returns parameter
object describing the file format):

- Header writer (model of serializer):
