Mapping line to offset
======================

This repo contains several version of an algorithm to map newlines to offsets.
Said otherwise, it takes a file as input and returns a vector ``Offsets`` where
``Offsets[i]`` returns the offset where the ``i`` th line begins.

This algorithm is used (at least) in LLVM.

This repo provides both validation and performance measurement for various
versions. The existing implementations are:

- ``ref.cpp``: the reference implementation
- ``seq.cpp``: the reference implementation, with some short-circuit and branch
  hint.
- ``bithack.cpp``: loads a word at a time and uses bit twiddling for some fast
  short-circuit
- ``sse_align.cpp``: legacy SSE implementaion which enforces alignment
- ``sse.cpp``: SSE implementation without any alignement enforcment.

Usage
=====

from the `src` directory, run

```
$ make perf
```

to get some performance measurements and

```
$ make check
```

to run the testsuite (well, there's only one test, but you got the idea).
