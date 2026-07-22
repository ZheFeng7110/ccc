# ccc.pipe_operator_helper

English | [简体中文](../zh/pipe_operator_helper.md)

---

`pipe_operator_helper` is a small C++ utility library that enables a pipe-style invocation syntax and pipe-style type-casts.

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

- Pipe-style function invocation helpers (supporting function objects and templates).
- Pipe-style type casts: `static_cast_to`, `dynamic_cast_to`, `const_cast_to`, `reinterpret_cast_to`, `bit_cast_to`, and
  helpers for `move_to` and `forward_to`.
- Designed to be usable as a single header (or as a module when built that way).
- Works with references, pointers, value types and many cast scenarios (see tests for details).

## Quick Usage

- Include the header:

  `#include "ccc/pipe_operator_helper.hpp"`

  or, when using C++ modules (if configured):

  `import ccc.pipe_operator_helper;`

- Function piping:
    - Define a callable type derived from `pipe_operator_helper::pipe_operator<Derived, Arity>` (where `Arity` is the
      number of arguments the function takes), implement the actual call as `operator()`, and re-expose the base
      class's partial-application `operator()` through a `using` declaration. Then create an `inline constexpr`
      instance.

    ```C++
    struct add_t : pipe_operator_helper::pipe_operator<add_t, 2> {
        using pipe_operator_helper::pipe_operator<add_t, 2>::operator();

        constexpr int operator()(const int a, const int b) const noexcept { return a + b; }
    };
    inline constexpr add_t add{};
    ```

    - Then you can use both `add(x, y)` and `x | add(y)`.
    - The left-hand argument's type is deduced at the pipe site, so templates no longer need explicit template
      arguments (e.g. `x | add(y)` instead of `x | add<T>(y)`).
    - The same pattern works for references, templates, `constexpr`, and `consteval` helpers (see
      `test/pipe_operator_helper_test/pipe_operators_tests/1arg.cc` and
      `test/pipe_operator_helper_test/pipe_operators_tests/2args.cc`).

- Type-cast operators:
    - Instead of `static_cast<T>(v)` use `v > pipe_operator_helper::static_cast_to<T>()` or
      `v > pipe_operator_helper::cast_to<T>()` (`cast_to` has the same effect with `static_cast_to`).
    - Similarly, there are `dynamic_cast_to<T>()`, `const_cast_to<T>()`, `reinterpret_cast_to<T>()` and
      `bit_cast_to<T>()` utilities.
    - `std::move` and `std::forward` has type-cast operators too:
        - `v > pipe_operator_helper::move_to()` has same effect with `std::move(v)`.
        - `v > pipe_operator_helper::forward_to<T>()` has same effect with `std::forward<T>(v)`.

## Type Traits

- `is_pipe_operator<T>` and `is_pipe_operator_v<T>`: compile-time type traits that check whether a type `T` is (derived from) a pipe operator type. Returns `true` for any type that inherits from `pipe_operator_base` (including `pipe_operator<Derived, Arity>` and user-defined types).

## Notes & Tips

- The library exposes both header-only and module-only usage. If your toolchain supports C++20 modules and the project
  is configured to build the module (`-DCCC_USE_CPP_MODULES=ON`), you can import `ccc.pipe_operator_helper`
  instead of including the header.
- If the pipeable callable conflicts with an existing function, you can place the instances in a `pp` namespace for
  disambiguation. See `test/pipe_operator_helper_test/pipe_operators_tests/wrap_already_exists.cc`.
- For up-to-date usage patterns, check:
  - `test/pipe_operator_helper_test/pipe_operators_tests/1arg.cc`
  - `test/pipe_operator_helper_test/pipe_operators_tests/2args.cc`
  - `test/pipe_operator_helper_test/pipe_operators_tests/wrap_already_exists.cc`
  - `test/pipe_operator_helper_test/type_cast_test.cc`

## Special Thanks

* The implementation in [the legacy repository](https://github.com/ZheFeng7110/cpp_pipe_operator_helper) of `v1.x.x` was inspired by [wzxzhuxi/cpp-functional-programming](https://github.com/wzxzhuxi/cpp-functional-programming/tree/main/06-composition).
