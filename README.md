<!--
  ** File Name: README.md
  ** Author:    Aditya Ramesh
  ** Date:      06/30/2014
  ** Contact:   _@adityaramesh.com
-->

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
    - **Note:** when deserializing, update the tuple stored within the io_state
    instead of returning a value from the process_component function.
    - How to deal with changing buffer offset and ensuring that constraints are
    still met after each element in the buffer is processed?
    - `archive/deserialize.hpp`.
    - Test the above.
    - `archive/write_header.hpp`.
    - Test the above.
    - `archive/serialize.hpp`.
    - Test the above.
    - `archive/io.hpp`.

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
