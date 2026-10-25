# tassert Test Guide

This directory tests the header and module interfaces of `ccc::tassert`, configuration propagation, failure handling, formatting backends, and installed use. Each subdirectory is an independent project described by [projects.json](projects.json).

## Running the tests

Run the CMake matrix with:

```sh
python3 test/tassert/run_projects.py --tool cmake --fail-fast
```

Run the mcpp matrix with:

```sh
python3 test/tassert/run_projects.py --tool mcpp --fail-fast
```

The driver gives every variant an independent build directory and module cache. For mcpp it runs `mcpp build -p test`, `mcpp run -p test`, and the generated binary directly. This verifies configuration headers, module BMIs, compiler macros, exit codes, and output without allowing one scenario to affect another. Successful scenarios first determine the platform-specific `EXIT_FAILURE`; negative scenarios only need to fail compilation and are checked against compiler diagnostics.

## Shared infrastructure

- [api.hh](api.hh) provides the `active` and `disabled` tags and selects headers, `ccc.tassert`, or the aggregate module.
- [config.hh](config.hh) provides configurations containing duplicate, empty, and embedded-null tags.
- [format_support.hh](format_support.hh) provides counting and throwing formatters for `std::format` and fmt.
- [project.cmake](project.cmake) centralizes standards, backends, configuration headers, module mode, warnings, and threads.
- [fmt.cmake](fmt.cmake) integrates fmt for scenarios that select that backend.
- [run_projects.py](run_projects.py) expands variants, builds projects, runs checks, matches compilation errors, and verifies module configuration rebuilds.

The CMake matrix contains 76 scenarios (56 runtime and 20 compilation-negative scenarios). The mcpp matrix contains 64 scenarios because installed consumers are CMake-only.

## Runtime scenarios

“Default” means the base project configuration; “module” imports `ccc.tassert`; “aggregate” imports `ccc`; and “mixed” uses headers and modules together.

| Project | Coverage |
| --- | --- |
| `basic` | Tags, compile-time enablement, result lifetime, lazy predicates, `if_enabled`, failure termination, `NDEBUG`, and automatic backends. |
| `ignore_tags` | Configuration headers, exact tag matching, duplicates, empty strings, embedded nulls, and empty configurations. |
| `lazy_condition` / `constexpr_branch` | Evaluation order, disabled callbacks, `if constexpr`, and compile-time branch removal. |
| `default_failure` / `handler` / `handler_concurrency` | Default and custom handlers, source locations, concurrency, recursion protection, and exit behavior. |
| `format_std` / `format_fmt` / `formatter_failure` | Standard and fmt formatting, modules, custom formatters, temporary arguments, compile-time checks, and formatting-failure fallback. |
| `exceptions` | Propagation and ordering of predicate, message, and formatting exceptions. |
| `multiple_translation_units` | Shared handler state, configuration, and failure behavior across translation units. |
| `modules` | Direct, aggregate, and mixed imports; backend selection; configuration propagation; and BMI rebuilds. |
| `install_consumer` | Installed headers and modules, `find_package(ccc)`, consumer configuration, and installed dependencies. |

## Compilation-negative scenarios

The projects under `compile_errors` must fail compilation. They cover standards below C++20, missing or invalid configuration, invalid backend selection, unavailable formatting backends, copy/move restrictions, invalid predicates, format type and index errors, runtime format strings, lvalue messages, and unsupported standard/fmt types. The driver checks semantic keywords in compiler diagnostics and does not run these programs.

## Extending the tests

1. Add an independent project with `CMakeLists.txt`, `main.cc`, and an mcpp manifest unless it is CMake-only.
2. Register its standards, backend, configuration, interface, variants, and assertions in `projects.json`.
3. Set `compile_error` for compilation negatives and match keywords from actual diagnostics.
4. Use `rebuild_config` when testing module configuration replacement in one build tree.
5. Check both tool execution and the generated binary.
6. Use `--fail-fast` for diagnosis, then run the full matrix for regression coverage.
