# tag 断言（C++20）

头文件为 `ccc/tassert.hh`，模块为 `ccc.tassert`，聚合模块 `ccc` 也导出这些接口。`ccc/all.hh` 在 C++20 及以上包含本组件。实现保持 header-only，要求开启异常处理。

```cpp
#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag request{"network.request"};

void send(bool connected) {
    ccc::tassert<request>(connected);
}
```

tag 必须统一声明为命名空间作用域的 **`inline constexpr ccc::tassert_tag`**，以引用形式作为非类型模板参数。构造函数为 `consteval`；`name()` 和 `enabled()` 为 `constexpr`。名称是不拥有字符的 `std::string_view`，应使用字符串字面量或生命周期足够长的静态字符数组。

## 配置

在首次包含前统一定义 `CCC_TASSERT_CONFIG_HEADER`，展开为带引号的配置头文件名。配置头只声明数据，不反向包含或导入 tassert：

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

**配置变量必须使用 `inline constexpr`，即使只在单个源文件中定义。** 支持 `std::string_view` 内建数组及 `std::array<std::string_view, N>`。显式空配置应包含 `<array>` 并声明 `inline constexpr std::array<std::string_view, 0> ignore_tags{};`。缺失文件、缺失变量、不支持的类型和不能常量求值的内容会导致编译失败；编译器无法普遍检查变量是否写了 `inline`。

未选择配置头时所有 tag 启用，`NDEBUG` 不影响行为。匹配按完整长度和字节内容区分大小写，包括显式长度中的内嵌空字符。空 tag 合法；重复项无额外影响；点号没有特殊含义。

## 求值规则与消息

```cpp
ccc::tassert<request>([&] { return expensive_check(); });
if constexpr (request.enabled()) {
    ccc::tassert<request>(expensive_check()).message("state={}", expensive_state());
}
ccc::if_enabled<request>([&] {
    ccc::tassert<request>(expensive_check()).message("state={}", expensive_state());
});
```

普通 bool 实参在 tag 禁用时仍求值。谓词只在启用时被转发调用一次，返回值只显式转换为 bool 一次，谓词异常正常传播。创建 lambda 的捕获初始化仍会执行。`if_enabled` 仅在启用时转发调用回调一次，不提前要求禁用回调可调用。

`.message(format, args...)` 使用后端的编译期格式串检查，仅允许右值调用，返回 `void`，仅失败时格式化。消息实参在进入成员函数前求值，成功或禁用时也不例外；要跳过这些副作用，请保护整个调用。禁用不意味着可以使用无效格式串或不可格式化类型；普通非模板 `if constexpr` 和 lambda 定义仍受正常语法、类型检查约束。

结果对象用于立即使用，禁止复制和移动。**不要保存结果或依赖延迟报告**；保证复制消除仍允许把它初始化到变量中。条件失败后，在 `.message` 中或完整表达式结束时报告。若消息实参先抛出异常，待失败结果在栈展开时报告基本失败信息；成功和禁用结果保留原异常传播。

位置由公开调用的默认 `std::source_location::current()` 捕获。包装函数应接收默认位置并显式传入；诊断不自动获取条件源码。编译期分支保证跳过工作，`inline` 不承诺零指令开销。

## 格式化后端

同一程序及模块统一使用 `CCC_TASSERT_FORMAT_BACKEND`：

| 值 | 行为 |
| --- | --- |
| 未定义 | C++23 及以上且标准格式化实际可用时选标准后端，否则无后端 |
| `0` | 基本断言；调用 `.message` 时明确编译报错 |
| `1` | 要求 C++23 及可用的、支持编译期格式检查的 `std::format` |
| `2` | 显式依赖 fmt，支持 C++20 及以上 |

C++20 不自动启用标准格式化，发现 fmt 头文件也不会自动选择 fmt。显式选择不可用后端会编译失败。自定义 formatter 和格式检查细节遵循后端规则，std 与 fmt 的扩展彼此独立。

## 失败处理器

```cpp
void report(const ccc::tassert_failure_info& info) noexcept {
    // info.tag、info.location、info.message 仅在调用期间有效。
    std::exit(42);
}
auto previous = ccc::tassert_set_handler(report);
ccc::tassert_set_handler(previous);
```

`tassert_handler` 是带 `noexcept` 的普通函数指针。初始处理器向 stderr 输出 tag、调用处文件/行/列、函数和可选消息，尽力刷新后 `std::exit(EXIT_FAILURE)`。`tassert_get_handler()` 始终返回有效非空指针；设置处理器返回上一个有效指针，传入 `nullptr` 恢复默认。自定义处理器若返回，库以失败退出兜底。

同配置的多个翻译单元及头文件/模块消费方共享 inline 原子指针。替换操作可并发执行，但不等待旧处理器结束；处理器可能并发调用，外部状态需自行同步。失败分派仅读取一次指针并同步调用。失败信息及消息视图均为借用，需要留存时自行复制。

格式化或分配异常会改用静态兜底消息。处理器重入时输出最小诊断并 `std::_Exit(EXIT_FAILURE)`，避免递归分派及重复退出清理。默认输出不分配内存，尽量连续输出单条报告；不承诺所有并发报告均在退出前完成。`std::exit` 执行已注册退出清理，不展开当前栈中的自动对象；静态/线程对象清理和 `atexit` 回调应避免再次断言。本版不支持关闭异常处理的构建。

## 构建、模块与安装

```sh
cmake -S . -B build -DCCC_USE_CPP_MODULES=ON -DCMAKE_CXX_STANDARD=23 \
  -DCCC_TASSERT_FORMAT_BACKEND=1 \
  -DCCC_TASSERT_CONFIG_HEADER=/absolute/path/project_assert_config.hh
```

宏配置通过 `ccc::ccc` 传播到模块接口及消费方。后端 `2` 使用 `find_package(fmt 11.2.0 CONFIG REQUIRED)` 查找 **>=11.2.0** 的 fmt，链接 `fmt::fmt`。`test/CMakeLists.txt` 在缺少兼容包时通过 CPM **0.43.2** 获取测试依赖；后端 `1` 还执行配置阶段的标准格式化编译、链接检查。其他组件仍可按原最低语言版本使用，显式请求标准格式化后端时目标要求 C++23。

`CCC_INSTALL=ON` 安装头文件，模块模式还安装模块源码及库。配置头及其依赖由用户提供。安装目标记录配置头文件名；消费方应在编译模块接口和使用代码前，将配置头所在目录加入安装目标的接口包含目录，并保证其内容与提供方构建一致。fmt 构建的包配置会查找已安装的 fmt。模块消费方使用兼容配置重新构建已安装的接口源码；BMI、对象和缓存均属于具体配置及工具链。不同配置请安装到不同前缀。

**用户配置必须参与模块接口编译。** 消费方在 `import` 前定义宏不能改变已构建模块。所有翻译单元及模块必须统一忽略列表、语言模式和后端；配置内容变化必须触发相关产物重建。

mcpp 通过库的显式 `tassert-fmt` feature 声明 `fmtlib.fmt = ">=11.2.0"`，传播后端 `2`。场景项目直接声明 `fmtlib.fmt` 依赖并选择该库 feature。根应用通过 `build.dialect_cxxflags` 设置 `-DFMT_ATTACH_TO_GLOBAL_MODULE=1`，使 fmt 模块和头文件提供一致实体；该全构建设置也将配置宏传播至依赖模块。配置头宏的引号转义参见各项目的 `mcpp.toml`。

## 验证

使用 Python 3.11 及以上逐项目构建、运行：

```sh
python3 test/tassert/run_projects.py --cmake-arg=-DCMAKE_CXX_COMPILER=clang++
python3 test/tassert/run_projects.py --tool mcpp
```

`projects.json` 列出各场景的语言版本、后端、引入方式、参数、预期退出状态和输出，以及预期编译失败类别。可用 `--project 'modules'`、`--mode mixed`、`--build-root <路径>`、重复的 `--cmake-arg=...` 或 `--mcpp-arg=--offline` 选择和配置运行。每个变体使用独立构建目录并保存日志。mcpp 运行器为变体建立独立工作区，执行 `mcpp build -p test`、`mcpp run -p test`，直接运行真实程序复核结果并检查工具的退出状态传播。工具链不支持的组合会报告失败，应显式选择适用项目；安装场景使用 CMake。
