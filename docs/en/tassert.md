# Tagged assertions (C++20)

Include `ccc/tassert.hh` or import `ccc.tassert` (also exported by `ccc`). `ccc/all.hh` includes it in C++20 and later. The implementation is header-only; enable exceptions when building it.

```cpp
#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag network{"network.request"};

void send(bool connected) {
    ccc::tassert<network>(connected);
}
```

A tag must be a namespace-scope `inline constexpr ccc::tassert_tag`. Its constructor is `consteval`; `name()` and `enabled()` are `constexpr`. The name is a non-owning `std::string_view`: use literals or static character arrays with sufficient lifetime. Tags are reference non-type template arguments.

## Configuration

Define `CCC_TASSERT_CONFIG_HEADER` to a quoted include filename before the first include, consistently throughout the program:

```cpp
// project_assert_config.hh: configuration data only
#pragma once
#ifndef PROJECT_ASSERT_CONFIG_HH
#define PROJECT_ASSERT_CONFIG_HH
#include <string_view>
namespace ccc::tassert_config {
inline constexpr std::string_view ignore_tags[] = {"network.request"};
}
#endif
```

The **`inline constexpr` declaration is required**, including in a single source file. Supported sequences are built-in arrays of `std::string_view` and `std::array<std::string_view, N>`. For an explicit empty list, use `inline constexpr std::array<std::string_view, 0> ignore_tags{};` and include `<array>`. The configuration header must not include/import tassert. Missing files, missing variables, unsupported types, and nonconstant data produce compilation errors; the compiler cannot generally diagnose a missing `inline` specifier.

With no configuration header, all tags are enabled. `NDEBUG` has no effect. Matching compares complete, case-sensitive byte strings, including embedded NULs when a length is supplied. Empty tags are valid; duplicate entries have no additional effect. Dots have no special meaning.

## Evaluation and messages

```cpp
ccc::tassert<network>([&] { return check_connection(); });

if constexpr (network.enabled()) {
    ccc::tassert<network>(check_connection()).message("state={}", read_state());
}
ccc::if_enabled<network>([&] {
    ccc::tassert<network>(check_connection()).message("state={}", read_state());
});
```

The ordinary bool argument is evaluated even for a disabled tag. The predicate overload calls its forwarded callable once and explicitly converts its result to bool once, only when enabled. Predicate exceptions propagate. Lambda capture initializers still run when constructing the argument. `if_enabled` calls its body once when enabled, forwarding its value category, and imposes no callability requirement on a disabled body.

`.message(format, args...)` accepts a compile-time checked format string, is restricted to rvalues, and returns `void`. Formatting happens only on failure. Message arguments are evaluated before the member function even on success or for disabled tags; use the surrounding compile-time branch to skip those side effects. Disabled assertions still require valid predicates and valid message formats/types. Ordinary non-template `if constexpr` and lambda definitions still undergo normal syntax/type checking.

The result is an immediately used temporary. Copying and moving are deleted. **Do not save it in a variable or depend on delayed reporting**: guaranteed copy elision still permits such initialization. A false condition becomes pending until `.message` runs or the full expression ends. If a message argument throws first, the pending result reports basic failure during stack unwinding. Successful/disabled results preserve the exception.

The default `std::source_location::current()` is captured at the public call, including predicate calls. Wrappers should accept a default source location and pass it explicitly. Diagnostics do not automatically contain the condition's source text. Compile-time branching guarantees skipping work; `inline` does not promise zero instructions.

## Formatting backends

`CCC_TASSERT_FORMAT_BACKEND` selects the backend consistently across all translation units and modules:

| Value | Behavior |
| --- | --- |
| omitted | C++23 or later with available standard formatting: standard; otherwise none |
| `0` | Basic assertions only; `.message` produces a clear compilation error |
| `1` | Requires C++23 and available `std::format` with compile-time format checking |
| `2` | Explicit fmt dependency; supports C++20 and later |

C++20 never automatically uses `<format>`; fmt is never selected just because its headers exist. Explicitly selecting an unavailable backend fails compilation. Formatting uses the chosen backend's formatter customization and format checking rules. Standard and fmt custom formatters are separate extensions.

## Failure handlers

```cpp
void handler(const ccc::tassert_failure_info& info) noexcept {
    // info.tag, info.location, info.message are valid only during this call.
    std::exit(42);
}
auto previous = ccc::tassert_set_handler(handler);
ccc::tassert_set_handler(previous);
```

`tassert_handler` is a `noexcept` function pointer. The initial handler writes tag, file/line/column, function, and optional message to stderr, flushes, then calls `std::exit(EXIT_FAILURE)`. `tassert_get_handler()` always returns an effective non-null pointer. Setting a handler returns the previous effective pointer; setting `nullptr` restores the default. If a custom handler returns, the library exits with `EXIT_FAILURE`.

The inline atomic pointer is shared across translation units and header/module consumers built with the same configuration. Replacement is safe concurrently and does not wait for an old handler to finish. Handlers can run concurrently and must synchronize their own state. Each failure reads the pointer once, then calls it synchronously. Copy information if it must outlive that call; views and formatted messages are borrowed.

Formatting/allocation exceptions dispatch a static fallback message. Recursive assertion handling writes a minimal diagnostic and calls `std::_Exit(EXIT_FAILURE)` to avoid recursive dispatch and exit cleanup. Default output avoids allocation and serializes individual reports; simultaneous failures need not all finish logging. `std::exit` executes registered exit cleanup but does not unwind automatic objects in the current stack. Static/thread cleanup and `atexit` callbacks must avoid asserting again. Exception-disabled builds are outside the supported configuration.

## Builds, modules and installation

CMake options:

```sh
cmake -S . -B build -DCCC_USE_CPP_MODULES=ON -DCMAKE_CXX_STANDARD=23 \
  -DCCC_TASSERT_FORMAT_BACKEND=1 \
  -DCCC_TASSERT_CONFIG_HEADER=/absolute/path/project_assert_config.hh
```

The backend and configuration propagate through `ccc::ccc` to both module compilation and consumers. Backend `2` uses `find_package(fmt 11.2.0 CONFIG REQUIRED)` and links `fmt::fmt`, accepting fmt versions **>=11.2.0**. The test dependency setup in `test/CMakeLists.txt` uses CPM **0.43.2** to obtain fmt when a compatible package is unavailable. Standard backend selection includes a configure-time compile/link check. Backend selection alone does not raise the minimum standard of unrelated components, except when explicitly requesting the standard formatting backend.

`CCC_INSTALL=ON` installs headers and, for module builds, module sources and the library. The user supplies the configuration header and its dependencies. The installed target records its filename; add the directory containing that header to the installed target's interface include directories before compiling module interfaces or consumers. The header contents must match the provider build. The package config finds fmt when required, using the installed fmt CMake package. Module consumers rebuild the installed interface sources with compatible settings; module caches, objects, and BMIs belong to a specific configuration/toolchain.

**Configure the module interface build itself.** Defining a macro before importing an already built module cannot change its configuration. All translation units and prebuilt modules must agree on ignore tags, language mode, and backend. Changing configuration contents must rebuild all relevant artifacts. Install a separate prefix for each configuration.

For mcpp, `tassert-fmt` is an explicit library feature that declares `fmtlib.fmt = ">=11.2.0"` and propagates backend `2`. Scenario manifests declare `fmtlib.fmt` directly and select this library feature. Set `-DFMT_ATTACH_TO_GLOBAL_MODULE=1` in the root application's `build.dialect_cxxflags` so fmt's module provides the same entities as its headers. This graph-wide setting also applies configuration macros to dependency modules. See those manifests for the escaped quoting required for configuration include filenames.

## Verification

`test/tassert/projects.json` lists independent applications, language modes, backends, import modes, expected compile errors and subprocess results. Run them with Python 3.11 or later:

```sh
python3 test/tassert/run_projects.py --cmake-arg=-DCMAKE_CXX_COMPILER=clang++
python3 test/tassert/run_projects.py --tool mcpp
```

Use `--project 'modules'`, `--mode mixed`, `--build-root <path>`, repeated `--cmake-arg=...`, or `--mcpp-arg=--offline` to select/configure a run. Each variant has independent builds and logs. The mcpp runner stages a separate workspace per variant, executes `mcpp build -p test` and `mcpp run -p test`, checks the real executable too, and verifies exit-status propagation. Unsupported compiler/library configurations are reported as failures; select applicable projects explicitly. Installation scenarios use CMake.
