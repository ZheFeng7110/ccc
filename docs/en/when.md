# ccc.when

English | [简体中文](../zh/when.md)

---

`ccc::when` is a generalized `switch` replacement inspired by **Kotlin's when expression**. Unlike traditional `switch` statements, which are limited to integral constant expressions, `when` can match against any type that supports `operator==`, and it supports returning values directly, making the code more concise and safer.

## Features

- Requires C++17 or later.
- More general than `switch`: case values are not limited to integral constant expressions; any type supporting `operator==` without throwing exceptions can be used.
- Arguments appear in pairs: `(case_value, handler)`.
- All handlers must return the same type; supports returning any copyable or movable type.
- When no case matches and no `default_tag` is provided, a default-constructed return value is returned.
- Available via both header inclusion (`#include`) and C++20 modules (`import`).

## Quick Usage

Include the header:

```cpp
#include "ccc/when.hh"
```

When using C++20 modules:

```cpp
import ccc.when;
```

## Basic Usage

### Replacing switch

Traditional `switch`:

```cpp
switch (value) {
case 1: doSomething1(); break;
case 2: doSomething2(); break;
default: doDefault(); break;
}
```

Equivalent `ccc::when`:

```cpp
ccc::when(value,
    1, [&] { doSomething1(); },
    2, [&] { doSomething2(); },
    ccc::default_tag, [&] { doDefault(); }
);
```

### Returning values (inspired by Kotlin when expressions)

```cpp
int r = ccc::when(value,
    1, [] { return -1; },
    2, [] { return -2; },
    ccc::default_tag, [] { return 0; }
);
```

When `value` equals `2`, `r` is `-2`.

### Non-integral matching

`when` supports any type that supports `operator==` without throwing exceptions:

```cpp
struct point {
    int x = 0, y = 0;
    constexpr bool operator==(const point& other) const noexcept {
        return x == other.x && y == other.y;
    }
};

const point p{2, 3};
const int result = ccc::when(p,
    point{0, 0}, [] { return 0; },
    point{2, 3}, [] { return 1; },
    ccc::default_tag, [] { return -1; }
);
// result == 1
```

## Core Rules

- Arguments must appear in **pairs**: `(case_value, handler)` or `(ccc::default_tag, handler)`.
- If `ccc::default_tag` is used, it must be the **last pair**.
- All handlers must be **zero-argument callables** (e.g., lambdas) and must return the **same type**.
- Case values must support `operator==` with the input value, and the comparison must be `noexcept`.
- If no case matches and no `default_tag` is provided, a **default-constructed return value** (e.g., `int{}`) is returned.

## Notes and Tips

- `ccc::when` is `constexpr`, so it can be used in compile-time contexts (provided the handlers are also `constexpr`).
- The `noexcept` status of the `when` expression depends on whether all handlers are `noexcept`; if any handler may throw, the entire expression is not `noexcept`.

## Complete Examples

For more usage examples, see the test file:
- `test/when_test.cc`
