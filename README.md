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

# TODO

- Note about concepts: do not implement Device, Serializer, or Deserializer. The
cost of using CRTP everywhere to force the relevant classes to obey the
interface has no tangible benefits.

- Stage 1: low-level IO classes.

    - Implement `satisfies` function for `buffer_constraints`; add checks in
    `file::allocate_*` functions.
    - Revise `file::buffer` so that it satisfies concept requirements.
    - Implement `file::device`.
        - Implement `required_constraints` and `preferred_constraints` with
	`io_type` parameter.
        - Implement `allocate_ibuffer`, `allocate_obuffer`, `allocate_iobuffer`.

- Stage 2: serialization and deserialization classes.
    - Implement `io_status.hpp`.
    - Implement `io_state.hpp`.
    - Implement `severity.hpp`.
    - Implement `buffer_state.hpp`.

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
