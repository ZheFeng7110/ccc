# tassert（tag_assert）设计文档

- 日期：2026-10-24
- 状态：设计提案，供后续实现与评审使用
- 最低标准：C++20
- 公共入口：`ccc::tassert<Tag>(...)`
- tag 类型：`ccc::tassert_tag`
- 头文件：`ccc/tassert.hh`
- 模块：`ccc.tassert`

## 1. 目标与设计结论

提供以字符串 tag 分类的断言库，通过常量表达式配置决定各个 tag 是否启用。断言调用不使用宏；宏仅用于选择配置头文件和格式化后端。支持头文件与 C++20 模块两种引入方式。

断言入口为带 tag 非类型模板参数的函数模板：

```cpp
#include "ccc/tassert.hh"

inline constexpr ccc::tassert_tag liba{"lib.modulea"};

void example(int a, int b) {
    ccc::tassert<liba>(1 == 1);
    ccc::tassert<liba>(a == b)
        .message("a did not equal b (a={}, b={})", a, b);
}
```

公共名称分别为断言函数模板 `ccc::tassert`、tag 类型 `ccc::tassert_tag` 和配置命名空间 `ccc::tassert_config`。模块名称为 `ccc.tassert`。

需要区分三种保证：

| 写法 | 禁用时的行为 | 编译期裁剪保证 |
| --- | --- | --- |
| `ccc::tassert<liba>(expr)` | 不进行断言处理，但 `expr` 仍按函数实参规则求值 | tag 状态在函数模板中编译期可知，断言处理分支可被裁剪 |
| `ccc::tassert<liba>([&] { return expr; })` | 不调用谓词，因而不求值其内部的 `expr` | 不依赖优化即可保证不调用；谓词本身通常仍需合法 |
| `if constexpr (liba.enabled()) { ... }` 或 `if_enabled<liba>(...)` | 整个受保护断言及消息实参均不执行 | 由 `if constexpr` 明确选择分支 |

**如果“被禁用的断言不执行”包含条件表达式、消息参数的全部副作用，应使用第三种写法；仅需延迟条件时使用第二种。** 普通函数无法在接收到实参后取消已经发生的实参求值。

第一版范围包括：精确 tag 匹配、同步失败报告、函数指针形式的可替换失败处理器、失败后退出进程、惰性谓词、编译期分支辅助函数及可选格式化。tag 开关由编译期配置决定。

## 2. 配置与 tag 语义

### 2.1 用户配置头文件

```cpp
// project_assert_config.hh
#pragma once
#ifndef PROJECT_ASSERT_CONFIG_HH
#define PROJECT_ASSERT_CONFIG_HH

#include <string_view>

namespace ccc::tassert_config {
inline constexpr std::string_view ignore_tags[] = {
    "lib.modulea",
    "libb.util",
};
}

#endif
```

通过构建选项定义 `CCC_TASSERT_CONFIG_HEADER`，其展开结果必须是可用于 `#include` 的头文件名。例如：

```text
-DCCC_TASSERT_CONFIG_HEADER='"project_assert_config.hh"'
```

也可在首次包含库头文件之前定义该宏。配置头文件在全局命名空间中包含，由它自行声明 `ccc::tassert_config::ignore_tags`，不能在库内部的 namespace 块内包含。

规则如下：

1. 未定义 `CCC_TASSERT_CONFIG_HEADER`：使用空忽略列表，所有 tag 启用。
2. 定义了该宏：配置头文件必须在 `ccc::tassert_config` 中提供名为 `ignore_tags` 的可常量求值序列。首版支持 `inline constexpr std::string_view` 的内建数组及 `inline constexpr std::array<std::string_view, N>`。
3. 配置变量必须写成 `inline constexpr`。这是最终用户文档中的强制要求，用于保证头文件被多个翻译单元包含时不会产生 ODR 问题。显式空配置使用 `inline constexpr std::array<std::string_view, 0> ignore_tags{};`，不使用非标准零长度内建数组。
4. 缺少配置文件、缺少变量、类型不支持或内容不能常量求值，均给出编译错误，不静默退回全启用。
5. 配置文件只声明配置数据，不反向包含 `ccc/tassert.hh` 或导入 `ccc.tassert`。
6. 不受 `NDEBUG` 影响；是否禁用完全由 tag 配置决定。

### 2.2 匹配规则

- 按字符串完整内容、长度、大小写精确匹配，不按地址比较。
- `lib.modulea` 不匹配 `lib.modulea.child`，点号仅是普通字符。
- 不支持通配符、正则、前缀及反向启用规则。
- 重复项无额外影响；未在忽略列表中的 tag 始终启用。
- 字符串按字节比较，不进行 Unicode 归一化。
- 空 tag 合法，可由空字符串规则禁用。显式带长度的 `string_view` 按实际长度处理，包括内嵌空字符。

### 2.3 tag 对象

接口轮廓如下；断言返回类型在后文说明：

```cpp
namespace ccc {
class tassert_tag {
public:
    consteval explicit tassert_tag(std::string_view name);
    constexpr std::string_view name() const noexcept;
    constexpr bool enabled() const noexcept;
};
}
```

公共 tag 类型为 `ccc::tassert_tag`。

构造函数必须常量求值，完成字符串匹配并保存 `name_`、`enabled_`。运行时不查询配置列表，也不初始化全局注册表。匹配采用简单线性扫描，成本发生在编译期，无需引入哈希碰撞问题。

名称为非拥有的 `string_view`，底层字符必须具有足够长的生命周期；推荐字符串字面量或静态存储期字符数组。`consteval` 不能代替这一生命周期约束。

tag 对象必须统一使用 `inline constexpr`，无论定义在单个源文件还是公共头文件中：

```cpp
inline constexpr ccc::tassert_tag liba{"lib.modulea"};
```

tag 对象具有静态存储期，其引用用作 `ccc::tassert<Tag>` 的非类型模板参数。

## 3. 断言 API 与求值规则

### 3.1 基本调用与位置

公开以下语义重载，每个重载的最后一个参数都提供默认值：

```cpp
std::source_location location = std::source_location::current()
```

- `tassert<Tag>(bool condition, location)`
- `tassert<Tag>(Predicate&& predicate, location)`

谓词重载通过约束限定为无参可调用、结果可显式转换为 `bool` 的类型，避免无约束转发重载抢占普通条件调用。所有重载返回内部的临时断言结果对象，允许后接 `.message(format, args...)`。`Tag` 为 `const tassert_tag&` 非类型模板参数，并在函数体内通过 `if constexpr` 选择启用或禁用路径。

位置必须由最外层公开调用的默认参数捕获，然后逐层传递。不能在内部辅助函数或析构函数里重新调用 `current()`，否则会得到库实现位置。允许显式传入位置，便于用户编写包装函数。

结果：

- tag 禁用：返回无动作结果，不执行谓词。
- tag 启用且条件成立：返回无动作结果。
- tag 启用且条件失败：返回待报告结果，于 `.message(...)` 或完整表达式结束时报告并终止。

非宏接口不能自动获取条件源码，例如 `a == b` 的文本；默认诊断不声称包含原表达式。用户可通过消息补充上下文。

### 3.2 惰性条件

```cpp
ccc::tassert<liba>([&] { return expensive_check(); });
```

只在 tag 启用时调用谓词一次，并仅转换一次返回值。谓词抛出的异常按正常函数调用传播，不将其伪装为返回 `false`，因此谓词入口不能无条件标记为 `noexcept`。

创建 lambda 及其捕获初始化仍会发生：

```cpp
ccc::tassert<liba>([value = expensive_value()] { return value > 0; });
```

上例不会跳过 `expensive_value()`。需要完全跳过这部分工作时，应将整个调用放入编译期分支。

### 3.3 严格编译期分支

最直接的接口是公开 `enabled()`：

```cpp
if constexpr (liba.enabled()) {
    ccc::tassert<liba>(expensive_check())
        .message("state={}", expensive_state());
}
```

另外提供模板辅助函数：

```cpp
namespace ccc {
template <const tassert_tag& Tag, class F>
constexpr void if_enabled(F&& body);
}
```

实现语义：仅在 `if constexpr (Tag.enabled())` 的真分支内以转发方式调用 `body` 一次。不要在函数签名、约束或无条件异常规格中提前要求禁用分支的 `F` 可调用。`Tag` 引用的对象必须满足非类型模板参数要求，使用命名空间作用域的 `inline constexpr tassert_tag`。

```cpp
ccc::if_enabled<liba>([&] {
    ccc::tassert<liba>(expensive_check())
        .message("state={}", expensive_state());
});
```

该辅助函数消除的是回调调用分支；传入回调时的捕获初始化仍遵循正常实参求值规则。示例使用无初始化副作用的引用捕获。

编译期“不执行”不等于“任意无效代码均可写在其中”。非模板上下文的 `if constexpr` 仍检查语法和类型；普通 lambda 的函数体也通常在定义时检查。只有位于模板依赖上下文中的相关实例化才可能被丢弃。第一版不承诺禁用后允许无效语法、未声明名称或任意不可格式化类型。

### 3.4 优化承诺

`consteval` 构造保证配置在编译期决定，惰性谓词保证禁用时不被调用，`if constexpr` 保证分支选择。`inline` 不保证机器码内联或零指令开销。

对于 `ccc::tassert<liba>(a == b)` 写法，只承诺断言处理分支在编译期关闭；优化器可能消除无副作用的条件计算和临时对象，但库不依赖这一点实现正确性。禁用 tag 也不能避免先行实参求值带来的异常或未定义行为。若必须保证条件本身不执行，使用谓词写法或 `if constexpr`/`if_enabled`。

## 4. 链式消息与格式化

### 4.1 链式消息接口

消息格式串和格式参数在同一次 `.message(...)` 模板调用中出现：

```cpp
ccc::tassert<liba>(a == b)
    .message("a did not equal b (a={}, b={})", a, b);
```

建议接口轮廓如下：

```cpp
template <class... Args>
void message(std::format_string<Args...> format, Args&&... args) &&;
```

在 fmt 后端使用等价的 `fmt::format_string<Args...>`。格式字符串必须是编译期可检查的字符串；参数类型、字段索引和格式说明符按所选后端的规则进行编译期检查。多余参数等细节遵循后端语义。

约定：

- 只有断言失败时执行格式化及输出；成功或禁用时不创建格式化结果字符串。
- `.message("...", a(), b())` 的实参仍会在进入成员函数前求值，即使断言成功或禁用；需要跳过这些副作用时使用第 3.3 节的编译期分支。仅将条件改成谓词并不能延迟消息实参。
- `.message` 仅允许对右值断言结果调用，并在报告后返回 `void`，不返回悬空的临时对象引用。
- 未调用 `.message` 的失败结果在临时对象析构时报告基本信息。
- `.message` 的格式参数不保存到完整表达式之外；格式化同步完成。

### 4.2 编译期关闭与格式检查的边界

`Tag` 是 `ccc::tassert<Tag>` 的非类型模板参数，因此实现可以使用 `if constexpr (Tag.enabled())` 完全移除断言处理、失败报告和格式化代码。它不能阻止普通条件实参 `a == b` 在函数调用之前求值，这是 C++ 函数调用语义的限制：

```cpp
// 禁用 tag 时仍可能求值 a == b，但不会执行断言处理和 message 格式化。
ccc::tassert<liba>(a == b).message("a={}, b={}", a, b);

// 禁用 tag 时连 expensive_check() 也不会调用。
ccc::tassert<liba>([&] { return expensive_check(); })
    .message("check failed");
```

即便 tag 已禁用，`.message` 的模板参数仍须满足格式化后端的类型和格式约束；“关闭”表示运行时语义及生成的断言处理被裁剪，不表示允许无效格式字符串或不可格式化类型存在于源代码中。

### 4.3 标准版本与后端策略

严格说，`std::format` 属于 C++20，当前标准库提供的编译期格式串检查还受缺陷修正与实现进度影响，并不是只要打开 C++23 就一定可用。本项目按需求采用以下支持策略：

| 环境 | 基本断言 | 格式化 `.message(...)` |
| --- | --- | --- |
| C++20，无 fmt | 支持 | 不支持 |
| C++20，显式选择并提供 fmt | 支持 | fmt 后端 |
| C++23 及以上，标准库格式化可用 | 支持 | 默认标准后端 |
| C++23 及以上，显式选择 fmt | 支持 | fmt 后端 |
| C++23 及以上，标准库格式化不可用、未选择 fmt | 支持 | 不支持 |

建议提供构建宏 `CCC_TASSERT_FORMAT_BACKEND`：

- `0`：无格式化后端，提供无消息的基本断言能力。
- `1`：标准后端，要求 C++23 及以上，且相关库功能实际可用。
- `2`：fmt 后端，要求显式配置 fmt 依赖。
- 未定义：C++23 及以上检测到可用标准格式化时选择 `1`，否则选择 `0`。

不因 `__has_include(<fmt/format.h>)` 为真就自动选择 fmt，避免机器环境或 include path 偶然改变依赖及内联定义。显式选择不可用后端应在编译期报错。标准后端检测需要头文件可用性、功能测试宏和构建验证，不只检查 `__cplusplus`。

无格式化后端时，包含 `ccc/tassert.hh` 和使用无消息断言应正常编译；只有使用 `.message(...)` 时给出明确的编译期诊断。C++20 即使标准库提供 `<format>`，首版也按上述策略只开放 fmt 格式化；C++23 标准后端则使用 `std::format_string<Args...>`。

fmt 为可选外部依赖，仅选择 fmt 后端的构建需要集成。CMake 通过 CPM.cmake 获取固定版本的 fmt，mcpp 在 `[dependencies]` 中直接声明 fmt，具体见第 7.1 节。优先使用 `fmt::fmt`，也可由构建系统一致选择 `fmt::fmt-header-only`。后端应分别验证自定义 formatter；两套后端的类型扩展不自动互通。

## 5. 临时结果对象与失败处理

### 5.1 生命周期模型

在 `ccc::detail` 中定义结果对象及报告工具，不作为模块公共符号导出。结果类型携带 tag 的编译期启用状态，以便裁剪 `.message(...)` 内的格式化和处理器调用路径。

结果对象保存待失败状态、tag 名称及最初捕获的位置。其状态机为：

1. 禁用或条件为真：无动作状态，析构和 `.message(...)` 均不报告。
2. 条件为假：待报告状态。
3. 失败结果上调用 `.message(...)`：先标记为已处理，再同步格式化并分派到失败处理器。
4. 失败结果未调用 `.message(...)`：在临时对象析构时标记为已处理，并分派基本失败信息到处理器。

要求：

- 析构函数为 `noexcept`，成功路径不分配内存。
- 删除复制及移动操作；各重载直接返回 prvalue，依靠 C++17 起的保证复制消除构造结果，避免多个对象重复报告。
- `.message(...)` 限定为右值调用，并返回 `void`；不返回悬空的临时对象引用。
- 不对结果类型添加 `[[nodiscard]]`，因为不接 `.message(...)` 的独立表达式是正常用法。
- 不保存格式参数引用或 `format_args` 到完整表达式之外。

断言结果是“立即使用”的临时对象，不是可保存的任务。删除复制和移动仍不能阻止 `auto pending = ccc::tassert<liba>(...);` 的保证复制消除初始化，因此文档必须明确禁止依赖这种延迟报告方式。

格式化生成的消息由失败分派路径持有，生命周期覆盖处理器调用；消息与格式参数不需要由结果析构函数延迟访问。

### 5.2 失败处理器、输出与退出

公开以下接口：

```cpp
namespace ccc {
struct tassert_failure_info {
    std::string_view tag;
    std::source_location location;
    std::string_view message;  // 已格式化的消息；未提供时为空。
};

using tassert_handler = void (*)(const tassert_failure_info& info) noexcept;

tassert_handler tassert_set_handler(tassert_handler handler) noexcept;
tassert_handler tassert_get_handler() noexcept;
}
```

处理器是普通函数指针，不使用拥有状态的函数包装器。用户可传入普通函数或可转换为该指针类型的无捕获 `noexcept` lambda。

接口约定：

- 初始处理器为库提供的默认处理器。
- `tassert_set_handler(handler)` 安装处理器并返回前一个有效处理器，便于保存和恢复。
- `tassert_set_handler(nullptr)` 恢复默认处理器；`tassert_get_handler()` 始终返回非空的当前有效处理器。
- 每次失败在分派时读取处理器一次并同步调用，安装操作影响此后读取到新指针的失败分派。
- 同一程序、相同配置的多个翻译单元以及头文件/模块消费方共享处理器状态；内部以具有统一实体身份的 inline 原子函数指针存储，实现并发读写。
- 原子读写只保证指针替换安全。处理器可能被多个线程并发调用，其使用的外部状态由用户同步；替换处理器不会等待旧处理器执行结束。
- 失败信息及其中的视图仅在处理器调用期间有效，需要留存时由处理器复制数据。

默认处理器将诊断信息输出到标准错误流，尽力刷新后调用 `std::exit(EXIT_FAILURE)`。诊断至少包含：

```text
[ccc.tassert] assertion failed
tag: lib.modulea
location: /path/to/source.cc:42:9
function: void example(int, int)
message: a did not equal b (a=1, b=2)
```

函数名、列号及其具体呈现取决于 `std::source_location` 的实现，测试不应假定函数签名的跨编译器格式相同。

失败处理器用于定制诊断和退出方式，约定处理后退出进程；若自定义处理器返回，库调用 `std::exit(EXIT_FAILURE)` 兜底，保证业务代码不会在失败断言之后继续运行。处理器的函数指针类型带 `noexcept`，不得让异常逸出。

`std::exit` 使用正常退出流程，执行已注册的 `atexit` 回调以及标准规定的静态/线程存储期清理，但不展开当前调用栈，不析构当前栈帧中的自动对象。默认失败不能依赖局部 RAII 对象清理资源。多个线程同时报告时应尽量将单条报告连续写出，不承诺退出前所有线程都完成日志输出。

格式化或分配异常发生时，以静态兜底消息构造失败信息并调用当前处理器；默认输出本身出现问题时，尽力使用不分配内存的输出路径后执行失败退出。处理器内再次触发断言时，以线程局部重入标记检测并直接走最小输出与失败退出路径，避免递归分派。静态/线程对象清理和 `atexit` 回调应避免再次触发断言，尤其不能依赖递归调用 `std::exit` 的行为。首版按正常启用异常处理的构建实现，`-fno-exceptions` 的兼容性需另行设计和验证。

若 `.message(...)` 的实参求值在进入成员函数前抛出异常，已构造的失败结果会在栈展开时析构，向处理器提供基本失败信息；无动作结果不影响异常传播。文档与场景项目应覆盖这一顺序，避免把 RAII 延迟报告误认为在 `tassert` 返回时已经完成失败处理。

## 6. 配置一致性、ODR 与模块

### 6.1 同一构建配置

`ignore_tags`、语言模式和格式化后端共同构成 tassert 的构建配置。一个程序中使用 tassert 的各翻译单元及预编译模块应保持一致。不能让同名模块或同一套内联定义在不同翻译单元拥有不同配置。

用户配置必须统一写成 `inline constexpr`：

```cpp
namespace ccc::tassert_config {
inline constexpr std::string_view ignore_tags[] = {
    "lib.modulea",
    "libb.util",
};
}
```

这是最终用户文档中的强制要求，不能省略 `inline`。配置头文件通常会被多个翻译单元包含；`inline constexpr` 使数组成为可跨翻译单元共享的同一实体，避免裸 `constexpr` 数组的内部链接导致 ODR 问题。实现直接在常量求值阶段读取这一共享配置实体。

空配置也必须遵循这一规则：`inline constexpr std::array<std::string_view, 0> ignore_tags{};`。整个程序、所有翻译单元和预编译模块仍必须使用相同的配置内容；`inline` 解决实体定义的 ODR 问题，不允许混用不同忽略列表。

### 6.2 头文件模式

头文件无条件包含实际需要的标准库及项目头文件，采用仓库的 `#pragma once` 加 include guard 模式。格式化后端仅根据统一构建配置选择依赖。

`CCC_TASSERT_CONFIG_HEADER` 必须在首次包含 tassert 前设置；同一翻译单元中修改宏再包含不会生成第二套配置。

### 6.3 模块模式

遵循现有“全局模块段包含头文件，选择性 using 导出”的模式：

```cpp
module;

#include "ccc/tassert.hh"

export module ccc.tassert;

export namespace ccc {
using ::ccc::tassert;
using ::ccc::tassert_tag;
using ::ccc::if_enabled;
using ::ccc::tassert_failure_info;
using ::ccc::tassert_handler;
using ::ccc::tassert_set_handler;
using ::ccc::tassert_get_handler;
}
```

公共 API 定义在 `ccc` 中，入口为 `ccc::tassert`，tag 类型为 `ccc::tassert_tag`；内部类型、配置解析和报告设施定义在 `ccc::detail`，不写入 export 块。内部结果类型可经公开函数的返回类型供调用者使用，其必要成员定义必须保持可达；这不等于导出供用户按名称依赖的内部 API，应以实际 import 编译测试验证。

**用户配置必须参与模块接口单元的编译。** 在消费方源文件中、`import ccc.tassert;` 之前定义 `CCC_TASSERT_CONFIG_HEADER`，不会改变已经构建好的模块配置。使用定制配置时必须用该配置重新构建模块及其依赖产物。

BMI、对象文件及模块缓存要按配置头内容、后端和工具链设置区分；修改配置头必须触发重建。安装的默认配置模块不能被宣传为支持消费方任意宏覆盖。可安装模块源码供消费方按配置重建，或明确交付的是配置固定的构建产物。

使用同一配置时，应验证独立头文件消费方、独立模块消费方及不同翻译单元混合消费的一致性；不承诺任意 include/import 顺序组合都自然可用。

## 7. 与现有仓库的集成规划

以下是后续实现时的预期变更，本设计文档不实施这些变更。

| 文件或目录 | 计划职责 |
| --- | --- |
| `include/ccc/tassert.hh` | 公共 tag、断言重载及分支辅助函数 |
| `include/ccc/detail/tassert_config.hh` | 用户配置加载、编译期匹配及后端选择 |
| `include/ccc/detail/tassert_report.hh` | 内部结果对象、格式化适配、处理器状态和失败分派 |
| `modules/ccc.tassert.ccm` | 选择性导出公共 API |
| `modules/ccc.ccm` | 增加 `export import ccc.tassert;` |
| `include/ccc/all.hh` | 仅 C++20 及以上包含 tassert，不抬高其他组件的最低标准 |
| `CMakeLists.txt` | 注册模块文件、配置宏及可选 fmt 目标依赖 |
| `cmake/get_cpm.cmake` | 从指定发布链接下载并纳入仓库的 CPM 引导文件 |
| `test/tassert/` | 场景项目目录、项目路径清单及逐项目编译运行入口 |
| `test/tassert/<场景>/CMakeLists.txt` | 独立场景的标准版本、配置、后端和依赖设置 |
| `test/tassert/<场景>/mcpp.toml` | 独立场景的项目、ccc 路径依赖及可选 fmt 依赖 |
| `test/CMakeLists.txt`、`test/mcpp.toml` | 划定场景项目边界，避免被现有测试源文件递归收集 |
| `docs/zh/tassert.md`、`docs/en/tassert.md` | 后续公共使用文档 |

根目录 `mcpp.toml` 已使用 `modules/**/*.ccm`，新模块文件会被源文件规则覆盖；仍需验证聚合模块导出及后端依赖配置。

库保持 header-only 的实现方式，除现有模块构建产物和可选 fmt 外不增加必须链接的自有运行库。CMake 中配置宏、配置头搜索路径及后端依赖应传播到相关目标：头文件目标用 `INTERFACE`，模块目标根据实现与消费需求使用 `PUBLIC`。不能只给测试可执行文件定义宏而漏掉模块编译。

不同忽略列表或后端的测试应构建为独立程序；模块模式下分别构建对应的库/BMI 变体，避免在同一个程序中引入配置冲突。

所有实现遵循仓库的 4 空格缩进、120 列宽、分支与循环加大括号，以及私有/受保护成员名称以下划线结尾的约定。

### 7.1 fmt 依赖集成

CMake 使用 CPM.cmake。实现时从以下链接下载 `get_cpm.cmake`，将文件保存到仓库的 `cmake/get_cpm.cmake`：

```text
https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.43.2/get_cpm.cmake
```

使用处直接 `include` 仓库中的该文件，再调用 `CPMAddPackage` 集成 fmt。仅选择 fmt 后端时执行依赖集成；fmt 首个验证版本固定为 `11.2.0`。以下为根目录 `CMakeLists.txt` 中的调用示意：

```cmake
include("${CMAKE_CURRENT_LIST_DIR}/cmake/get_cpm.cmake")

CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    VERSION 11.2.0
    GIT_TAG 11.2.0
    OPTIONS "FMT_TEST OFF" "FMT_DOC OFF" "FMT_INSTALL ON"
)
```

将 `fmt::fmt` 与 `CCC_TASSERT_FORMAT_BACKEND=2` 按第 7 节的可见性要求传递给 ccc 及消费目标。多个场景可复用 CPM 源码缓存，各项目的构建目录和模块产物保持独立；模块配置必须在 ccc 模块接口编译之前确定。安装导出还需处理 fmt 的依赖查找，不能让消费目标依赖只在构建目录存在的目标。

mcpp 场景项目直接在其 `mcpp.toml` 的 `[dependencies]` 中声明依赖，例如在 `test/tassert/<场景>/` 下：

```toml
[dependencies]
ccc = { path = "../../.." }
fmt = "11.2.0"
```

依赖 fmt 的场景同时设置后端为 `2`，并确保设置作用于依赖模块的构建。场景项目形成独立的依赖构建上下文；若构建工具的依赖配置传播或缓存隔离机制不足，需在后续构建集成中补足，而不能仅定义消费源文件的宏。基础及标准后端场景只声明所需依赖。

## 8. 场景项目测试设计与验收标准

### 8.1 组织方式

tassert 采用独立应用场景项目测试，不加入 Catch2 单元测试可执行文件。`test/tassert/` 中维护项目路径清单，每个路径对应可独立配置和构建的项目，包含入口源文件、所需用户配置头、`CMakeLists.txt` 与 `mcpp.toml`。

建议目录布局：

```text
test/tassert/
    projects.json
    run_projects.py
    basic/
    ignore_tags/
    lazy_condition/
    constexpr_branch/
    default_failure/
    handler/
    handler_concurrency/
    format_std/
    format_fmt/
    formatter_failure/
    exceptions/
    multiple_translation_units/
    modules/
    install_consumer/
    compile_errors/
```

`projects.json` 显式列出项目相对路径、构建方式、C++ 标准、后端、引入方式，以及每次运行的参数、预期退出状态和 stdout/stderr 匹配规则。一个项目可有多个运行案例。编译失败案例列出具体负例子项目路径及预期诊断类别，由运行器判定编译结果。

运行器逐个项目执行配置、编译与运行，为每个项目/工具链/配置创建独立构建目录；汇总结果并在任一案例不符合预期时以非零状态结束。应用程序通过普通控制流、计数器、退出状态及输出标记验证行为，不能用待测 tassert 自身作为唯一的测试成功判据。

现有 `test/mcpp.toml` 的 `**/*.cc` 源文件规则和现有 CMake 源文件收集需要明确排除这些独立场景，场景配置和 fmt 依赖均由各项目自己的构建描述管理。

### 8.2 项目清单与覆盖场景

| 项目路径（相对于 `test/tassert/`） | 应用场景及预期 |
| --- | --- |
| `basic/` | 默认启用、条件成功、无格式化依赖、`NDEBUG` 下保持相同语义；运行成功 |
| `ignore_tags/` | 用户 `inline constexpr ignore_tags`、空列表、精确匹配、大小写、重复规则和内容相同但地址不同的字符串；`static_assert` 验证启用状态，运行验证禁用失败条件不调用处理器 |
| `lazy_condition/` | 启用谓词恰好调用一次，禁用谓词零次；普通条件仍求值，捕获初始化与消息实参按正常规则求值 |
| `constexpr_branch/` | `if constexpr` 与 `if_enabled` 跳过整个断言、消息实参及捕获初始化中的副作用 |
| `default_failure/` | 裸失败及带消息失败，验证 stderr 中 tag、调用处位置、消息，以及失败退出；`atexit` 标记执行、当前栈局部析构标记未执行 |
| `handler/` | 设置、获取、替换、返回旧指针、传空恢复默认、捕获完整失败信息；自定义处理器用约定状态码退出；返回型处理器验证库失败退出兜底，禁用与成功时调用次数为零 |
| `handler_concurrency/` | 多线程安装/读取有效指针，分派获得完整指针；各终止性案例在独立运行中验证，重入案例验证不会递归调用用户处理器 |
| `format_std/` | C++23 标准后端，零参数消息、格式说明符、自定义 formatter 和临时格式参数；分别运行成功与失败案例 |
| `format_fmt/` | C++20 及 C++23 的 fmt 后端；分别用 CPM 和 mcpp 获取依赖并运行，验证格式化及依赖传递 |
| `formatter_failure/` | 自定义 formatter 抛出异常，当前处理器收到兜底消息并退出；成功和禁用时 formatter 不被调用 |
| `exceptions/` | 谓词异常正常传播；消息实参异常下，失败结果分派基本信息，成功/禁用结果保留异常传播 |
| `multiple_translation_units/` | 公共头中的配置和 tag、跨源文件一致性；一个翻译单元设置处理器，另一个读取并在失败时调用 |
| `modules/` | `import ccc.tassert`、`import ccc`、模块配置重建、不同翻译单元头文件/模块混合消费，以及共享处理器状态 |
| `install_consumer/` | 安装后消费、默认与定制配置构建产物、fmt 依赖和模块源文件/产物可用性 |
| `compile_errors/` | 多个独立负例子项目覆盖不合法配置、低语言版本、缺失后端、格式错误、不可格式化类型及结果复制移动；预期编译失败 |

配置的 `inline` 要求通过用户文档和示例审阅验证；C++20 没有直接反射变量是否声明为 `inline` 的通用机制，不能承诺所有缺少 `inline` 的配置都自动给出诊断。负例子只纳入实现实际可以稳定诊断的错误。

### 8.3 退出与输出的判定

运行器将每个可执行程序作为子进程启动，分别捕获 stdout、stderr 和退出状态。默认断言失败按 `EXIT_FAILURE` 对应的失败状态判定，不按信号崩溃判定，也不硬编码其数值在所有平台上都等于 `1`。自定义处理器案例可用专用退出码区分处理器路径与默认路径。

诊断至少验证 tag、消息及调用处文件/行号；函数签名格式与路径分隔符按平台容许变化。验证消息实参、formatter、处理器和析构各阶段的顺序时使用明确输出标记，发生退出的案例单独运行。

编译失败案例只检查相关诊断类别和关键文本，不依赖编译器完整报错字面相同。运行设置超时，将意外挂起、异常终止或缺失预期输出报告为失败。

### 8.4 构建矩阵

- 头文件：C++20 无 fmt、C++20 + fmt、C++23 标准后端、C++23 + fmt。
- 模块：上述受工具链支持的配置，并验证 `import ccc.tassert` 和聚合 `import ccc`。
- GCC、Clang、MSVC 的受支持组合；分别验证标准库格式化的真实可用性。
- 独立多翻译单元配置、模块重建、安装后消费及可选依赖传递。

CMake 逐项目执行以下流程；`<项目>` 为清单中的项目目录，`<构建目录>` 为该案例独立的输出目录：

```sh
cmake -S test/tassert/<项目> -B <构建目录>
cmake --build <构建目录>
```

随后由运行器启动该项目的程序并核对预期结果；预期编译失败项目在编译阶段完成判定。fmt 场景通过第 7.1 节的 CPM 配置集成依赖。

mcpp 的场景项目各自使用独立工作区，测试包名统一为 `test`。运行器以项目目录为工作目录，按仓库约定逐项执行：

```sh
mcpp build -p test
mcpp run -p test
```

运行器捕获 `mcpp run` 启动程序的结果，并验证其对目标退出状态的传播；必要时由运行器直接执行构建好的目标以获取真实状态。完整验收以清单中所有适用项目逐个编译、运行或按预期编译失败为准，覆盖编译期开关、求值规则、失败处理器、默认退出、格式检查及两种引入方式的一致性。
