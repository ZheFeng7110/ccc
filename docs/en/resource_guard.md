# ccc.resource_guard

English | [简体中文](../zh/resource_guard.md)

---

`ccc.resource_guard` provides a lightweight set of resource management utilities based on RAII, ensuring resources are properly released when leaving scope. The core components include `defer` (deferred execution), `try_finally` (try-finally pattern), and `with` (with-statement pattern).

## Features

- Compatible with C++11 and later.
- Zero heap allocation, stack-based storage only.
- `defer` executes an arbitrary callable in its destructor, emulating Go's `defer` or Python's `finally`.
- `try_finally` provides structured try-finally control flow where the finally block always executes regardless of exceptions.
- `with` offers concise scope-based management for resource types that implement `with_start()` / `with_end()` interfaces.

## Quick Usage

Include the header:

```cpp
#include "ccc/resource_guard.hh"
```

When using C++20 modules:

```cpp
import ccc.resource_guard;
```

## Core Components

### `ccc::defer<Func>`

`defer` accepts a callable and defers its execution to when the `defer` instance is destroyed. The callable is invoked regardless of how the scope is exited (normal return or exception).

```cpp
{
    int* p = new int(42);
    ccc::defer df{[&] { delete p; p = nullptr; }};
    // use p...
    // p is automatically deleted on scope exit, even if an exception is thrown
}

{
    FILE* f = fopen("data.txt", "r");
    ccc::defer df{[&] { if (f) fclose(f); }};
    // use f...
    // fclose is called automatically on scope exit
}
```

Notes:
- `defer` is non-copyable and non-movable, ensuring unique ownership per instance.
- The deferred callable must be `noexcept` (or guaranteed not to throw), because throwing from a C++ destructor causes `std::terminate`.
- In C++20 and later, the destructor is marked `constexpr`, enabling compile-time usage.

### `ccc::try_finally`

`try_finally` provides explicit try-finally control flow. The first argument is the try block, and the second is the finally block. The finally block always executes after the try block completes (normally or via exception).

```cpp
struct File {
    bool on = false;
    void open()  { on = true; }
    void close() noexcept { on = false; }
};

File f;
f.open();

ccc::try_finally(
    [&] {
        // try block: perform operations that may throw
        risky_operation(f);
    },
    [&] {
        // finally block: always executed
        f.close();
    }
);
```

Note: `try_finally` does not catch or suppress exceptions — if the try block throws, the exception propagates outward, but the finally block executes before that.

### `ccc::with`

`with` provides concise scope-based management for resource types that implement `with_start()` and `with_end()` methods. `with_start()` is called automatically on entry, and `with_end()` is called on exit (normal or exceptional).

```cpp
struct Lock {
    bool is_locked = false;
    void lock()   noexcept { is_locked = true; }
    void unlock() noexcept { is_locked = false; }

    void with_start() noexcept { lock(); }
    void with_end()   noexcept { unlock(); }
};

Lock l;
const auto result = ccc::with(l, [&] {
    // l.with_start() is called automatically on entry; l is locked here
    return do_work();
});  // l.with_end() is called automatically on exit; l is unlocked here
```

`with` returns the value of the passed function, supporting any copyable or movable type.

## Complete Examples

For more usage examples, see the test file:
- `test/resource_guard_test.cc`

