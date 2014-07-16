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

- Stage 2: IO state facilities.
    - `mnist::context`
    - MNIST IO functions.

- Stage 3: MNIST and tuple support.
    - `mnist::header_reader.hpp`.
    - `mnist::deserializer.hpp`.
    - Test both of the above.
    - `dsa::input_state`.
    - `dsa::header_reader.hpp`.
    - `dsa::deserializer.hpp`.
    - Test both of the above.
    - `dsa::header_writer.hpp`.
    - `dsa::header_deserializer.hpp`.
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
