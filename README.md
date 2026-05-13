# ccc

English | [简体中文](./README_zh.md)

---

ccc is a C++ utility library that provides modern C++ tools, including pipe operators, monadic expected, and more.

## Features

- Header-only and module-supported design for maximum flexibility.
- C++14 and later support (some features require C++17 or C++20).

## Quick Start

### Include the library

```cpp
#include "ccc/all.hh"
```

Or, for specific components:

```cpp
#include "ccc/expected.hh"
#include "ccc/pipe_operator_helper.hh"
```

When using C++20 modules (if configured):

```cpp
import ccc;  // import all
import ccc.expected;  // import expected component only
```

## Documentation

- [ccc.pipe_operator_helper](./documents/en/pipe_operator_helper.md) - Pipe-style function invocation helpers
- [ccc.expected](./documents/en/expected.md) - Monadic expected with optional criterion support

## Building

This project uses CMake. To build:

```bash
cmake -S . -B build
cmake --build build
```

For module support, configure with:

```bash
cmake -DCPP_PIPE_OPERATOR_HELPER_USE_MODULES=ON ..
```

## Testing

Run tests with:

```bash
cmake -S . -B build -DCCC_BUILD_TESTS=ON
cmake --build build
cd build && ctest .
```

## License

This project is licensed under the MIT License.
