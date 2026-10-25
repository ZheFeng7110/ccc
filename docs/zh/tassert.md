# ccc.tassert

[English](../en/tassert.md) | 简体中文

---

`ccc::tassert` 提供按 tag 分组的断言，可在编译期按名称禁用指定分组。需要 C++20 及以上版本，并开启异常处理；`NDEBUG` 不影响断言行为。

## 基本用法

```cpp
#include "ccc/tassert.hh"

inline constexpr ccc::tassert_tag request{"network.request"};

void send(bool connected) {
    ccc::tassert<request>(connected);
}
```

使用模块时，将头文件替换为：

```cpp
import ccc.tassert;
```

也可通过 `ccc/all.hh`（C++20 及以上）或聚合模块 `ccc` 引入。

- tag 在命名空间作用域声明为 **`inline constexpr ccc::tassert_tag`**，可放在公共头文件中供多处使用。
- 名称使用字符串字面量或生命周期足够长的静态字符数组。`name()` 返回名称，`enabled()` 可在编译期查询是否启用。
- 默认启用所有 tag。启用的断言条件为假时，默认向 stderr 输出 tag、调用位置、函数及可选消息，然后以失败状态退出程序。
- 直接调用断言或紧接着调用 `.message(...)`，不要保存断言的返回结果。

## 禁用指定 tag

创建配置头文件，例如 `project_assert_config.hh`：

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

通过构建系统统一定义宏 `CCC_TASSERT_CONFIG_HEADER`，使其展开为带引号的配置头文件名，例如 `"project_assert_config.hh"`。该定义应同时应用于所有使用 tassert 的翻译单元和相关模块接口的编译。

此时名为 `network.request` 的 tag 被禁用。名称匹配区分大小写，必须完整相同；点号只是名称的一部分，不表示层级或通配符。

配置头不要包含或导入 tassert。`ignore_tags` 必须使用 **`inline constexpr`**，支持 `std::string_view` 内建数组和 `std::array<std::string_view, N>`。如需空列表，包含 `<array>` 并声明：

```cpp
inline constexpr std::array<std::string_view, 0> ignore_tags{};
```

整个程序应使用同一配置；具体配置方式见[构建与集成](#构建与集成)。

## 跳过禁用断言的计算

普通实参即使在 tag 禁用时也会求值。将开销较大的检查放进 lambda，才会在禁用时跳过：

```cpp
ccc::tassert<request>(expensive_check());  // 禁用时仍计算
ccc::tassert<request>([&] { return expensive_check(); });  // 仅启用时计算
```

lambda 在启用时调用一次，返回值须能显式转换为 `bool`；检查抛出的异常正常传播。lambda 的捕获初始化仍会执行，因此应把需要跳过的工作放在函数体中。

## 添加失败消息

选择可用的格式化后端后，可直接追加 `.message(...)`：

```cpp
ccc::tassert<request>(connected).message("request {} is disconnected", request_id);
```

消息只在断言失败时格式化，但**消息实参在成功或禁用时仍会求值**。若要在禁用时跳过整个调用，使用 `ccc::if_enabled`：

```cpp
ccc::if_enabled<request>([&] {
    ccc::tassert<request>(expensive_check()).message("state={}", expensive_state());
});
```

也可用 `if constexpr (request.enabled()) { ... }`。禁用 tag 不会取消正常的语法、类型和格式串检查。

### 选择格式化后端

通过 `CCC_TASSERT_FORMAT_BACKEND` 选择：

| 值 | 行为 |
| --- | --- |
| 未定义 | C++23 及以上且标准格式化可用时使用 `std::format`，否则仅支持基本断言 |
| `0` | 仅支持基本断言，不能调用 `.message(...)` |
| `1` | 使用 `std::format`，要求 C++23 及支持编译期格式检查的标准库 |
| `2` | 使用 fmt，支持 C++20 及以上；需提供 fmt **>=11.2.0** |

fmt 必须显式选择，不会因已安装而自动启用。选择不可用的后端会导致编译失败。格式串在编译期检查，自定义类型的格式化遵循所选后端的规则。

## 自定义失败处理

处理器接收 tag、调用位置和消息，必须为 `noexcept` 函数：

```cpp
#include <cstdlib>

void report(const ccc::tassert_failure_info& info) noexcept {
    // 在这里记录 info.tag、info.location 和 info.message。
    std::exit(42);
}

void configure_assertions() {
    auto previous = ccc::tassert_set_handler(report);
    // ...
    ccc::tassert_set_handler(previous);  // 恢复先前处理器
}
```

`tassert_get_handler()` 获取当前处理器，`tassert_set_handler(nullptr)` 恢复默认处理器。自定义处理器若返回，程序仍以失败状态退出，不能用它忽略失败并继续运行。

处理器由整个程序共享，可能被多个线程同时调用；共享状态需自行同步。失败信息只在调用期间有效，需要留存时应复制。不要在处理器或退出清理中再次触发断言。默认的 `std::exit` 不会展开当前调用栈，不能依赖局部对象的析构来清理资源。

调用位置默认记录在 `tassert` 调用处，而非自动记录条件源码。封装断言时，可让包装函数接收默认值为 `std::source_location::current()` 的位置参数，并将其作为 `tassert` 的第二个实参传入。

## 构建与集成

### CMake

例如，启用模块、标准格式化后端和自定义配置：

```sh
cmake -S . -B build -DCCC_USE_CPP_MODULES=ON -DCMAKE_CXX_STANDARD=23 \
  -DCCC_TASSERT_FORMAT_BACKEND=1 \
  -DCCC_TASSERT_CONFIG_HEADER=path/to/project_assert_config.hh
cmake --build build
```

链接 `ccc::ccc` 即可获得相应配置。使用 fmt 时将后端改为 `2`，并确保 CMake 能找到 fmt >=11.2.0。

### 模块与安装

**配置必须在编译模块接口时生效。** 在 `import` 前定义宏不能改变已构建模块。所有翻译单元和模块必须统一忽略列表、语言模式和格式化后端；修改配置后应重新构建相关产物。

使用 `CCC_INSTALL=ON` 安装时，配置头及其依赖由用户提供。消费方应将配置头所在目录加入安装目标的接口包含目录，并保持配置内容与库构建时一致；使用 fmt 的包还需安装 fmt。模块消费方须以兼容工具链和配置重新构建已安装的模块接口源码。不同配置请安装到不同前缀。

### mcpp

使用 fmt 时，为 ccc 选择 `tassert-fmt` feature，并声明 `fmtlib.fmt = ">=11.2.0"` 依赖。在根应用的 `build.dialect_cxxflags` 中设置 `-DFMT_ATTACH_TO_GLOBAL_MODULE=1`，并通过该全构建设置传递配置宏。可参考 [fmt 配置示例](../../test/tassert/format_fmt/mcpp.toml)和[忽略列表配置示例](../../test/tassert/ignore_tags/mcpp.toml)。
