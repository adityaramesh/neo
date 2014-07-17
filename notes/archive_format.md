<!--
  ** File Name:	archive_format.md
  ** Author:	Aditya Ramesh
  ** Date:	05/04/2014
  ** Contact:	_@adityaramesh.com
-->

# Introduction

This document describes the data set archive (DSA) format, which is intended to
store tuples of matrices. Each matrix in the tuple can have varying dimensions
and scalar type.

# Header

  - Version                   (1 byte)
  - Endianness information    (1 byte)
  - Tuple size                (1 byte)
  - Component 1 scalar type   (1 byte)
  - Component 1 dimensions    (1 byte)
  - Component 1 storage order (1 byte, optional)
  - Component 1 extents       (n 32-bit integers, optional)
  - ...
  - Component n scalar type   (1 byte)
  - Component n dimensions    (1 byte)
  - Component n storage order (1 byte, optional)
  - Component n extents       (n 32-bit integers, optional)
  - Record count              (8 bytes)
  - Data

Required information during compile-time:

  - Size of tuple
  - Element scalar types
  - Element dimensions
  - Storage order (can be different from the storage order in the file; by
  default, assumed to be the same).
  - Element extents (optional)
