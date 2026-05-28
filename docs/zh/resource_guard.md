# ccc.resource_guard

[English](../en/resource_guard.md) | 简体中文

---

`ccc.resource_guard` 提供了一套轻量级的资源管理工具，基于 RAII 模式确保资源在作用域退出时被正确释放。核心组件包括 `defer`（延迟执行）、`try_finally`（try-finally 模式）和 `with`（with 语句模式）。

## 特性

- C++11 及以上版本兼容。
- 零堆分配，仅依赖栈上存储。
- `defer` 可在析构函数中执行任意可调用对象，模拟 Go 语言的 `defer` 或 Python 的 `finally`。
- `try_finally` 提供结构化的 try-finally 控制流，无论是否抛出异常，finally 块都会执行。
- `with` 为具有 `with_start()` / `with_end()` 接口的资源类型提供简洁的作用域管理模式。

## 快速使用

引入头文件：

```cpp
#include "ccc/resource_guard.hh"
```

使用 C++20 模块时：

```cpp
import ccc.resource_guard;
```

## 核心组件

### `ccc::defer<Func>`

`defer` 接受一个可调用对象，并将其执行推迟到 `defer` 实例析构时。无论作用域以何种方式退出（正常返回或异常抛出），该可调用对象都会被调用。

```cpp
{
    int* p = new int(42);
    ccc::defer df{[&] { delete p; p = nullptr; }};
    // 使用 p...
    // 离开作用域时自动 delete，即使中途抛出异常也能正确释放
}

{
    FILE* f = fopen("data.txt", "r");
    ccc::defer df{[&] { if (f) fclose(f); }};
    // 使用 f...
    // 作用域退出时自动 fclose
}
```

注意事项：
- `defer` 不可复制、不可移动，确保每个实例的唯一所有权。
- 被延迟的可调用对象必须是 `noexcept` 的（或确保不会抛出），因为 C++ 析构函数中抛出异常会导致 `std::terminate`。
- C++20 及以上版本中析构函数标记为 `constexpr`，支持编译期使用。

### `ccc::try_finally`

`try_finally` 提供显式的 try-finally 控制流。第一个参数是 try 块，第二个参数是 finally 块，finally 块在 try 块完成后（正常或异常）必定执行。

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
        // try 块：执行可能抛出异常的操作
        risky_operation(f);
    },
    [&] {
        // finally 块：无论是否异常都会执行
        f.close();
    }
);
```

注意：`try_finally` 不会捕获或抑制异常——如果 try 块抛出异常，异常会继续向外传播，但 finally 块会在此之前执行。

### `ccc::with`

`with` 为实现了 `with_start()` 和 `with_end()` 方法的资源类型提供简洁的作用域管理。进入 with 块时自动调用 `with_start()`，退出时（无论正常或异常）自动调用 `with_end()`。

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
    // 进入时，自动调用 `l.with_start()`，此处 l 已被锁定
    return do_work();
});  // 退出时，自动调用 `l.with_end()`，此处 l 已被解锁
```

`with` 的返回值是传入函数的返回值，支持任意可复制或可移动的类型。

## 完整示例

更多用法请参阅测试文件：
- `test/resource_guard_test.cc`

