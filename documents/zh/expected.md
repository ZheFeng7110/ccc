# ccc.expected

[English](../en/expected.md) | 简体中文

---

`ccc::expected<T, E, Criterion>` 是对 `std::expected` 的 C++14 实现与扩展。ccc::expected 不仅能够实现"正常值"与"错误值"，还可以通过自定义 Criterion 类型，从而扩展出"正常值附带额外 Criterion 信息"的语义。

例如，当字符串处理缓冲区不够，导致数据被截断时，这是"有值"的，但可以附带自定义 Criterion 信息，以表示"字符串有值但被截断"。

当忽略第三个模板参数时，使用默认值 `default_criterion`，其行为与普通的 `std::expected` 类似。

## 特性

- C++14 兼容的 expected/单子错误处理实现。
- 可选的第三个模板参数 `Criterion`，用于扩展语义。
- 单子操作（需要 C++17 及以上）：`and_then`、`or_else`、`transform`、`transform_error`。
- 使用默认 criterion 时，与 `std::expected` 基本兼容。
- 支持引用、指针、值类型以及多种场景。

## 快速使用

- 引入头文件：

  `#include "ccc/expected.hh"`

  或在使用 C++ 模块时：

  `import ccc.expected;`

### 使用默认 criterion 的基本用法

```cpp
ccc::expected<int, std::string> result = 42;
if (result.has_value()) {
    int value = *result;
    // 使用 value
}
```

### 使用自定义 criterion

```cpp
// 用于截断字符串的自定义 criterion
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

// 与 expected 一起使用
ccc::expected<std::string, std::string, truncate_criterion> process_string() {
    // ... 处理并检测截断
    return ccc::expected<std::string, std::string, truncate_criterion>(
        std::move(result), truncate_criterion{.was_truncated = true});
}
```

## 核心类型

- `ccc::expected<T, E, Criterion = default_criterion>`: 主 expected 类型
- `ccc::unexpected<E>`: 错误值包装器
- `ccc::bad_expected_access<E>`: 访问 unexpected 值时抛出的异常
- `ccc::default_criterion`: 简单的布尔语义默认 criterion

## 单子操作

所有单子操作在 C++17 及更高版本中可用：

- `and_then`: 链接返回 expected 的操作
- `or_else`: 通过返回 expected 处理错误
- `transform`: 转换值
- `transform_error`: 转换错误

## 注意事项与提示

- Criterion 类型必须满足特定要求：
  - 具有嵌套的 `value_type` 类型定义
  - 实现 `has_value()` 方法（noexcept）
  - 提供 `default_error_value` 静态成员
  - 默认构造函数、能够通过 `default_error_value` 静态成员构造的构造函数

- 完整的用法示例请参阅：
  - `test/expected_test/expected_test.cc`
