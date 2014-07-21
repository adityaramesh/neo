<!--
  ** File Name: README.md
  ** Author:    Aditya Ramesh
  ** Date:      06/30/2014
  ** Contact:   _@adityaramesh.com
-->

# Introduction

`neo` is a C++ framework for high-performance, pipelined IO. It's geared towards
streaming data to and from various binary and text-based file formats. The
library is structured into the following three independent modules whose
interaction is coordinated by the high-level programmer interface.

  - The IO interface. The purpose of this interface is threefold:
    1. To provide a uniform interface to access the platform-specific system
    calls used to read data from files into byte buffers, and to write data from
    byte buffers to files.
    2. To automatically apply the IO optimizations suggested by the results of
    [this benchmark][io_benchmark], and allow the user to manually tune these
    options.
    3. To implement clean and robust error-checking using modern programming
    idioms.
  - The serialization and deserialization interface. This interface provides
  efficient, stateless functions for various file formats to deserialize objects
  from byte buffers and serialize objects to byte buffers.
  - The flow graph interface. This interface has the following purposes:
    1. To allow the programmer to compose flow graphs to describe the
    interaction between producer-consumer agents.
    2. To coordinate the communication of data between agents in the flow graph.
    3. To schedule the execution of the agents to maximize usage of available
    system resources.

[io_benchmark]: http://adityaramesh.com/io_benchmark/

# Future Ideas

- Use the DSA file format to implement boosting for data sets that cannot fit in
RAM. One component of the tuple should be used to store the weight associated
with each example.
- Implement `file::copy`, with offsets and an enum to control replacement
behavior. Refer to Boost.Filesystem's `copy_file` for this.
- Add CSV support to `neo`. This may be involved because it requires parsing
date times, floats, phone numbers, addresses, etc. Should custom types be
provided for these?
- The `vector` owned by the `error_state` class is not thread-safe. If the
logging messages need to be printed on different thread(s) than the one that is
performing the IO, then we need to replace `vector` with a SPSC/SPMC queue.

# TODO

- Note about concepts: do not implement Device, Serializer, or Deserializer. The
cost of using CRTP everywhere to force the relevant classes to obey the
interface has no tangible benefits.

- **Note:** it is the responsibility of the IO agent to deal with premature EOF
errors.

- Create header with `deserialize`, `serialize`, `from_string`, and `to_string`
functions for useful primitive types.

- Stage 3: archive support.
    - Add `AccessMode` template parameter to `buffer_state`.
    - Set the buffer constraints for MNIST and archive based on the access mode.
    For sequential access patterns, make the buffer size at least the smallest
    multiple of the element size that is larger than the header size. Also
    require the buffer size to be a multiple of the element size.
    - For random access patters, first set the required buffer size to max of
    the header size and element size.
    - `archive/write_header.hpp`.
    - Test the above.
    - `archive/serialize.hpp`.
    - Test the above.
    - `archive/io.hpp` -- just include the other headers.

- Stage 4: Flow graph support.
    - ...

- MNIST demo based on LeNet-5.
- MNIST demo with more of the latest NN techniques, e.g.:
    - Linear threshold functions.
    - DropOut, MaxOut, etc.
    - Nesterov momentum.
    - Local response normalization.
    - Local contrast normalization.
    - Pooling (average, maximum), including overlapping pooling regions.
- MNIST demo with affine distortions.
- CIFAR demo.
- FFmpeg support.
- Support for replicating network efficiently.
