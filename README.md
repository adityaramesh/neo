<!--
  ** File Name: README.md
  ** Author:    Aditya Ramesh
  ** Date:      06/30/2014
  ** Contact:   _@adityaramesh.com
-->

# TODO

- Note about concepts: do not implement Device, Serializer, or Deserializer. The
cost of using CRTP everywhere to force the relevant classes to obey the
interface has no benefit.

- Stage 1: low-level IO classes.
    - Test `buffer_constraints.hpp`.
    - Implement `buffer.hpp`, along with allocation routine in the same file.
    - Implement `file.hpp`.

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
