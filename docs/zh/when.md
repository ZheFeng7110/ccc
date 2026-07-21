# ccc.when

[English](../en/when.md) | 简体中文

---

`ccc::when` 是一个仿照 **Kotlin when 表达式** 设计的通用化 `switch` 替代方案。与传统 `switch` 语句只能匹配整型常量表达式不同，`when` 可以对任何支持 `operator==` 的类型进行条件匹配，并且支持直接返回值，使代码更加简洁和安全。

## 特性

- 需要 C++17 及以上版本。
- 比 `switch` 更通用：case 值不限于整型常量表达式，任何支持 `operator==` 且不抛异常的类型均可使用。
- 参数成对出现，形式为 `(case_value, handler)`。
- 所有 handler 的返回类型必须一致；支持返回任意可复制/可移动的类型。
- 未显式写出 `default_tag` 且无匹配时，返回默认构造的结果值。
- 可同时通过头文件 (`#include`) 或 C++20 模块 (`import`) 使用。

## 快速使用

引入头文件：

```cpp
#include "ccc/when.hh"
```

使用 C++20 模块时：

```cpp
import ccc.when;
```

## 基本用法

### 替代 switch 语句

传统 `switch`：

```cpp
switch (value) {
case 1: doSomething1(); break;
case 2: doSomething2(); break;
default: doDefault(); break;
}
```

等效的 `ccc::when`：

```cpp
ccc::when(value,
    1, [&] { doSomething1(); },
    2, [&] { doSomething2(); },
    ccc::default_tag, [&] { doDefault(); }
);
```

### 返回值（仿照 Kotlin when 表达式）

```cpp
int r = ccc::when(value,
    1, [] { return -1; },
    2, [] { return -2; },
    ccc::default_tag, [] { return 0; }
);
```

当 `value` 等于 `2` 时，`r` 等于 `-2`。

### 非整型匹配

`when` 表达式支持任何支持 `operator==` 且不抛异常的类型：

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

## 核心规则

- 参数必须**成对出现**：`(case_value, handler)` 或 `(ccc::default_tag, handler)`。
- 如果使用了 `ccc::default_tag`，它必须是**最后一对参数**。
- 所有 handler 必须是**无参数可调用对象**（如 lambda），且返回类型必须相同。
- case 值必须支持与输入值进行 `operator==` 比较，并且该比较必须是 `noexcept` 的。
- 如果没有匹配任何 case，也没有提供 `default_tag`，则返回**默认构造的返回值**（例如 `int{}`）。

## 注意事项与提示

- `ccc::when` 是 `constexpr` 的，可以在编译期上下文中使用（要求 handler 也是 `constexpr`）。
- `when` 表达式的 `noexcept` 性取决于所有 handler 是否都是 `noexcept` 的；若任一 handler 可能抛出，则整个表达式不是 `noexcept`。

## 完整示例

更多用法请参阅测试文件：
- `test/when_test.cc`
