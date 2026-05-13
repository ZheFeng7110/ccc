# ccc.expected

English | [简体中文](../zh/expected.md)

---

`ccc::expected<T, E, Criterion>` is a C++14 implementation and extension of `std::expected`. It not only provides "expected value" and "error value" semantics, but also supports custom `Criterion` types to extend "expected value with additional criterion information" semantics.

For example, when a string processing buffer is not large enough and data is truncated, there is a "value", but custom criterion information can be attached to indicate "the string has a value but was truncated".

When ignoring the third template parameter, the default `default_criterion` is used, and its behavior is similar to a standard `std::expected`.

## Features

- C++14 compatible implementation of expected/monadic error handling.
- Optional third template parameter `Criterion` for extended semantics.
- Monadic operations (requires C++17 and above): `and_then`, `or_else`, `transform`, `transform_error`.
- Basic compatibility with `std::expected` semantics when using the default criterion.
- Works with references, pointers, value types and many scenarios.

## Quick Usage

- Include the header:

  `#include "ccc/expected.hh"`

  or, when using C++ modules:

  `import ccc.expected;`

### Basic usage with default criterion

```cpp
ccc::expected<int, std::string> result = 42;
if (result.has_value()) {
    int value = *result;
    // use value
}
```

### Using custom criterion

```cpp
// Custom criterion for truncated string
struct truncate_criterion {
    using value_type = bool;
    static constexpr value_type default_error_value = false;

    bool was_truncated = false;

    constexpr bool has_value() const noexcept { return true; }
    constexpr bool operator==(const truncate_criterion& other) const noexcept {
        return was_truncated == other.was_truncated;
    }
};
static_assert(ccc::is_criterion_v<truncate_criterion>, "must satisfy criterion requirements");

// Use with expected
ccc::expected<std::string, std::string, truncate_criterion> process_string() {
    // ... process and detect truncation
    return ccc::expected<std::string, std::string, truncate_criterion>(
        std::move(result), truncate_criterion{.was_truncated = true});
}
```

## Core Types

- `ccc::expected<T, E, Criterion = default_criterion>`: The main expected type
- `ccc::unexpected<E>`: Wrapper for error values
- `ccc::bad_expected_access<E>`: Exception thrown when accessing value of unexpected
- `ccc::default_criterion`: Default criterion with simple boolean semantics

## Monadic Operations

All monadic operations are available in C++17 and later:

- `and_then`: Chain operations that return expected
- `or_else`: Handle errors by returning expected
- `transform`: Transform the value
- `transform_error`: Transform the error

## Notes & Tips

- Criterion types must satisfy specific requirements:
  - Have a nested `value_type` typedef
  - Implement `has_value()` method (noexcept)
  - Provide `default_error_value` static member
  - Default constructible, and constructible via the `default_error_value` static member

- For complete usage examples, see:
  - `test/expected_test/expected_test.cc`
