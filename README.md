<!--
  ** File Name: README.md
  ** Author:    Aditya Ramesh
  ** Date:      06/30/2014
  ** Contact:   _@adityaramesh.com
-->

# Introduction

`neo` is a C++ library for high-performance, binary file IO. It's geared towards
streaming data from a variety of binary- and ASCII-based file formats. IO is
highly optimized using using results from a [comprehensive benchmark
suite][io_benchmark]. The library itself is composed largely of stateless free
functions, lending itself to use in parallel data-processing pipelines.

# Architecture

The library is structured into the following two modules.
  - The **core module** provides a small set of actions used to manipulate
  system resources and propagate buffer constaints. Error handling is achived
  using `expected` and `optional` types rather than by throwing exceptions or
  returning error codes.
  - The **IO module** interfaces with specfic file formats. Each file format is
  associated with a set of functions for reading and writing headers, and
  scanning (deserializing) and formatting (serializing) data.

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
