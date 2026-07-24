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

- `ccc.utility` - Utility tools that implement higher-version C++ STL features in lower C++ standards
- [ccc.pipe_operator_helper](docs/en/pipe_operator_helper.md) - Pipe-style function invocation helpers
- [ccc.expected](docs/en/expected.md) - Monadic expected with optional criterion support
- [ccc.inplace](docs/en/inplace/inplace.md) - Stack-allocated, compile-time fixed-capacity "in-place" containers
- [ccc.resource_guard](docs/en/resource_guard.md) - RAII-based resource management with defer, try_finally, and with patterns
- [ccc.when](docs/en/when.md) - Generalized switch-like expression inspired by Kotlin's when

## Building

This project uses CMake. To build:

```bash
cmake -S . -B build
cmake --build build
```

For module support, configure with:

```bash
cmake -DCCC_USE_CPP_MODULES=ON ..
```

## Testing

Run tests with:

```bash
cmake -S . -B build -DCCC_BUILD_TESTS=ON
cmake --build build
cd build && ctest .
```

## Building with mcpp

This project can also be built with [mcpp](https://github.com/mcpp-community/mcpp) for C++23 module-based builds:

```bash
mcpp build         # Build the library (module mode)
mcpp run -p test   # Build and run all tests
mcpp pack          # Package the release artifact
```

## License

This project is licensed under the MIT License.
