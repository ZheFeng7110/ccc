# ccc.tassert

English | [简体中文](../zh/tassert.md)

---

`ccc::tassert` groups assertions by tag and lets you disable selected groups by name at compile time. It requires C++20 or later with exceptions enabled. `NDEBUG` does not affect assertion behavior.

## Basic usage

```cpp
#include "ccc/tassert.hh"

inline constexpr ccc::tassert_tag request{"network.request"};

void send(bool connected) {
    ccc::tassert<request>(connected);
}
```

For modules, replace the include with:

```cpp
import ccc.tassert;
```

You can also use `ccc/all.hh` (C++20 and later) or the aggregate module `ccc`.

- Declare each tag at namespace scope as **`inline constexpr ccc::tassert_tag`**. Put shared tags in a common header.
- Use a string literal or a static character array with sufficient lifetime for the name. `name()` returns the name; `enabled()` lets you query its status at compile time.
- All tags are enabled by default. When an enabled assertion fails, the default handler writes the tag, call location, function, and optional message to stderr, then exits with a failure status.
- Call the assertion directly or immediately append `.message(...)`; do not store its result.

## Disabling tags

Create a configuration header, for example `project_assert_config.hh`:

```cpp
#pragma once
#ifndef PROJECT_ASSERT_CONFIG_HH
#define PROJECT_ASSERT_CONFIG_HH

#include <string_view>

namespace ccc::tassert_config {
inline constexpr std::string_view ignore_tags[] = {"network.request"};
}

#endif
```

Define the `CCC_TASSERT_CONFIG_HEADER` macro consistently through your build system, expanding to a quoted configuration header filename such as `"project_assert_config.hh"`. Apply this definition when compiling every translation unit that uses tassert and all relevant module interfaces.

This disables tags named `network.request`. Matching is case-sensitive and requires the full name; dots are ordinary characters, not hierarchy separators or wildcards.

The configuration header must not include or import tassert. Declare `ignore_tags` as **`inline constexpr`**, using a built-in array of `std::string_view` or `std::array<std::string_view, N>`. For an empty list, include `<array>` and declare:

```cpp
inline constexpr std::array<std::string_view, 0> ignore_tags{};
```

Use the same configuration throughout the program. See [Build integration](#build-integration) for setup instructions.

## Skipping work for disabled assertions

Ordinary arguments are evaluated even when a tag is disabled. Put expensive checks inside a lambda to skip them when disabled:

```cpp
ccc::tassert<request>(expensive_check());  // Evaluated even when disabled
ccc::tassert<request>([&] { return expensive_check(); });  // Evaluated only when enabled
```

When enabled, the lambda is called once and its result must be explicitly convertible to `bool`. Exceptions from the check propagate normally. Lambda capture initializers still run, so put work you want to skip inside the body.

## Adding failure messages

With a formatting backend selected, append `.message(...)` directly:

```cpp
ccc::tassert<request>(connected).message("request {} is disconnected", request_id);
```

Formatting happens only on failure, but **message arguments are evaluated even on success or when disabled**. To skip the entire call when disabled, use `ccc::if_enabled`:

```cpp
ccc::if_enabled<request>([&] {
    ccc::tassert<request>(expensive_check()).message("state={}", expensive_state());
});
```

Alternatively, use `if constexpr (request.enabled()) { ... }`. Disabling a tag does not remove normal syntax, type, or format-string checking.

### Choosing a formatting backend

Set `CCC_TASSERT_FORMAT_BACKEND` to select a backend:

| Value | Behavior |
| --- | --- |
| omitted | Use `std::format` in C++23 or later when available; otherwise basic assertions only |
| `0` | Basic assertions only; `.message(...)` is unavailable |
| `1` | Use `std::format`; requires C++23 and a standard library with compile-time format checking |
| `2` | Use fmt; supports C++20 and later and requires fmt **>=11.2.0** |

fmt must be selected explicitly; installing it does not enable it automatically. Selecting an unavailable backend fails compilation. Format strings are checked at compile time, and custom type formatting follows the selected backend's rules.

## Custom failure handling

A handler receives the tag, call location, and message, and must be a `noexcept` function:

```cpp
#include <cstdlib>

void report(const ccc::tassert_failure_info& info) noexcept {
    // Log info.tag, info.location, and info.message here.
    std::exit(42);
}

void configure_assertions() {
    auto previous = ccc::tassert_set_handler(report);
    // ...
    ccc::tassert_set_handler(previous);  // Restore the previous handler
}
```

`tassert_get_handler()` retrieves the current handler; `tassert_set_handler(nullptr)` restores the default. If a custom handler returns, the program still exits with a failure status. A handler cannot ignore a failure and resume execution.

The handler is shared throughout the program and may be called from multiple threads concurrently; synchronize shared state yourself. Failure information is valid only during the call, so copy anything you need to keep. Do not trigger assertions from handlers or exit cleanup. The default `std::exit` does not unwind the current call stack, so do not rely on local destructors for cleanup.

The reported location defaults to the `tassert` call site; diagnostics do not automatically include the condition's source text. When wrapping an assertion, accept a location parameter defaulted to `std::source_location::current()` and pass it as the second argument to `tassert`.

## Build integration

### CMake

For example, enable modules, standard formatting, and a custom configuration:

```sh
cmake -S . -B build -DCCC_USE_CPP_MODULES=ON -DCMAKE_CXX_STANDARD=23 \
  -DCCC_TASSERT_FORMAT_BACKEND=1 \
  -DCCC_TASSERT_CONFIG_HEADER=path/to/project_assert_config.hh
cmake --build build
```

Link `ccc::ccc` to inherit these settings. For fmt, select backend `2` and ensure CMake can find fmt >=11.2.0.

### Modules and installation

**Configuration must apply when compiling the module interface.** Defining macros before importing an already built module cannot change its settings. All translation units and modules must agree on the ignore list, language mode, and formatting backend. Rebuild relevant artifacts after changing configuration.

When installing with `CCC_INSTALL=ON`, supply the configuration header and its dependencies yourself. Add its directory to the installed target's interface include directories and keep its contents consistent with the library build. Packages using fmt also require an installed fmt package. Module consumers must rebuild the installed interface sources with a compatible toolchain and configuration. Install different configurations to separate prefixes.

### mcpp

For fmt, select ccc's `tassert-fmt` feature and declare a `fmtlib.fmt = ">=11.2.0"` dependency. Set `-DFMT_ATTACH_TO_GLOBAL_MODULE=1` in the root application's `build.dialect_cxxflags`, and use this build-wide setting to pass configuration macros too. See the [fmt configuration example](../../test/tassert/format_fmt/mcpp.toml) and [ignore-list configuration example](../../test/tassert/ignore_tags/mcpp.toml).
