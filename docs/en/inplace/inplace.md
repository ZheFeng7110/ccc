# ccc.inplace

English | [简体中文](../../zh/inplace/inplace.md)

---

`ccc.inplace` provides a series of "in-place" containers whose storage lives on the stack with compile-time fixed capacity. Unlike traditional dynamic-memory containers (e.g. `std::vector`), inplace containers involve no heap allocation, making them suitable for performance-sensitive scenarios or environments where dynamic memory is prohibited.

## Features

- Stack-allocated storage, no heap allocation, cache-friendly.
- Compile-time fixed capacity; exceeding capacity throws an exception (or can be handled gracefully via `try_*` interfaces).
- Interface style follows C++ standard library containers, supporting iterators, range operations, and STL algorithms.
- Compatible with C++11 and later.

## Current Components

| Component | Module Name                                       | Description |
|-----------|---------------------------------------------------|-------------|
| `inplace_vector<T, N>` | [ccc.inplace.inplace_vector](./inplace_vector.md) | A lower-version implementation of C++26 `std::inplace_vector` |
| `inplace_map<Key, T, N>` | [ccc.inplace.inplace_map](./inplace_map.md) | Fixed-capacity ordered associative container |

## Quick Start

Include the header:

```cpp
#include "ccc/inplace.hh"  // include all inplace components
```

Or include only inplace_vector:

```cpp
#include "ccc/inplace/inplace_vector.hh"
```

Or include only inplace_map:

```cpp
#include "ccc/inplace/inplace_map.hh"
```

When using C++20 modules:

```cpp
import ccc.inplace;                  // import all inplace components
import ccc.inplace.inplace_vector;   // import inplace_vector only
import ccc.inplace.inplace_map;      // import inplace_map only
```

