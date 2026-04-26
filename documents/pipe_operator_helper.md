# ccc.pipe_operator_helper

`pipe_operator_helper` is a small C++ utility library that enables a pipe-style invocation syntax and pipe-style
type-casts.

Instead of writing traditional calls like:

- `f1(x)`
- `f2(x, y)`
- `static_cast<int>(a)`

you can write:

- `x | f1()`
- `x | f2(y)`
- `a > static_cast_to<int>()`

This can make some call-chains and transformations read left-to-right in a more natural way.

## Features

- Pipe-style function invocation helpers (supporting free functions and templates).
- Pipe-style type casts: `static_cast_to`, `dynamic_cast_to`, `const_cast_to`, `reinterpret_cast_to`, `bit_cast_to`, and
  helpers for `move_to` and `forward_to`.
- Designed to be usable as a single header (or as a module when built that way).
- Works with references, pointers, value types and many cast scenarios (see tests for details).

## Quick usage

- Include the header:

  `#include "pipe_operator_helper.hpp"`

  or, when using C++ modules (if configured):

  `import pipe_operator_helper;`

- Function piping:
    - Define a normal function (for example, `int add(int a, int b)`) and provide a helper overload that returns a
      `pipe_tag` wrapping a callable that takes the left-hand argument.

    ```C++
    constexpr int add(const int a, const int b) noexcept { return a + b; }

    constexpr auto add(const int b) noexcept
    {
        return pipe_operator_helper::pipe_tag{[=](const int a) noexcept { return add(a, b); }};
    }
    ```

    - Then you can use both `add(x, y)` and `x | add(y)`.
    - The same pattern works for references, templates, `constexpr`, and `consteval` helpers (see
      `test/pipe_operators_tests/1arg.cpp` and `test/pipe_operators_tests/2args.cpp`).

- Type-cast operators:
    - Instead of `static_cast<T>(v)` use `v > pipe_operator_helper::static_cast_to<T>()` or
      `v > pipe_operator_helper::cast_to<T>()` (`cast_to` has the same effect with `static_cast_to`).
    - Similarly, there are `dynamic_cast_to<T>()`, `const_cast_to<T>()`, `reinterpret_cast_to<T>()` and
      `bit_cast_to<T>()` utilities.
    - `std::move` and `std::forward` has type-cast operators too:
        - `v > pipe_operator_helper::move_to()` has same effect with `std::move(v)`.
        - `v > pipe_operator_helper::forward_to<T>()` has same effect with `std::forward<T>(v)`.

## Notes & tips

- The library exposes both header-only and module-only usage. If your toolchain supports C++20 modules and the project
  is configured to build the module(`-DCPP_PIPE_OPERATOR_HELPER_USE_MODULES=ON`), you can import `pipe_operator_helper`
  instead of including the header.
- For up-to-date usage patterns, check:
  - `test/pipe_operators_tests/1arg.cpp`
  - `test/pipe_operators_tests/2args.cpp`
  - `test/type_cast_test.cpp`

## Special Thanks

* The implementation of v1.x.x was inspired by [wzxzhuxi/cpp-functional-programming](https://github.com/wzxzhuxi/cpp-functional-programming/tree/main/06-composition).
