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

# TODO

- Note about concepts: do not implement Device, Serializer, or Deserializer. The
cost of using CRTP everywhere to force the relevant classes to obey the
interface has no tangible benefits.

- Stage 2: serialization and deserialization classes.
    - `io_status.hpp`
    - `buffer_state.hpp`
    - `mnist::context`?
    - `mnist::context.hpp`
    - `mnist::error_state.hpp` `(aliased to std::vector<log_record>)`
    - `mnist::input_state`
    - `mnist::log_record.hpp`

- Stage 3: MNIST and tuple support.
    - Implement `mnist::header_reader.hpp`.
    - Implement `mnist::deserializer.hpp`.
    - Test both of the above.
    - Implement `dsa::header_reader.hpp`.
    - Implement `dsa::deserializer.hpp`.
    - Test both of the above.
    - Implement `dsa::header_writer.hpp`.
    - Implement `dsa::header_deserializer.hpp`.
    - Test both of the above.

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
