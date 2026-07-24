# AGENTS.md

## 项目概述

ccc 是一个跨平台的 C++ 实用功能库，提供一系列现代 C++ 工具（管道操作符、单子 expected、资源守卫、inplace 容器等）。所有组件同时支持传统头文件引入和 C++20 模块引入。

## 核心约束

### C++ 标准兼容

- 最低要求：C++11（`__cplusplus >= 201103L`）。不考虑也不引用 C++11 之前的写法。
- 部分功能可能需要 C++17 或 C++20；通过 `__cplusplus` 或标准功能检测宏进行条件编译。
- 禁止使用平台相关 API、编译器扩展或操作系统相关代码。所有行为必须符合 C++ 标准且跨平台一致。

### 文件扩展名

- C++ 源文件：`.cc`
- C++ 头文件：`.hh`
- C++ 模块接口单元：`.ccm`

### 头文件 + 模块双支持

每个库组件必须同时支持 `#include` 和 `import` 两种引入方式。具体模式参见 skill 文件 `.agents/skills/cpp-lib-headers-modules-both-supports/SKILL.md`。

速查要点：
- 头文件放在 `include/ccc/` 下，使用 `#pragma once` + `#ifndef`/`#define` 双重包含保护。头文件为纯 C++ 代码，不含 `export` 关键字或模块相关宏，所有 `#include`（标准库和项目依赖）均为无条件。
- 模块接口单元（`.ccm`）放在 `modules/` 目录下。
- 模块名与头文件路径对应：`include/ccc/foo/bar.hh` 对应 `export module ccc.foo.bar;`
- `.ccm` 文件中，所有 `#include`（标准库和项目头文件）均放在全局模块片段（`module;` 之后、`export module` 之前）。
- `.ccm` 文件使用 `export namespace ccc { using ::ccc::xxx; }` 选择性导出符号。不需要 `import` 其他项目模块——依赖由头文件的 `#include` 隐式提供。

### 命名空间与模块导出

- 面向用户的功能定义在 `ccc` 命名空间内，通过在 `.ccm` 文件中使用 `export namespace ccc { using ::ccc::xxx; }` 导出。
- 内部实现细节定义在 `ccc::detail` 命名空间内，**不**通过模块导出（不在 `.ccm` 文件的 `export` 块内）。

### 函数约束：SFINAE / Concepts

当函数或模板需要按条件参与重载决议时，遵循 `.agents/instructions/constraint-for-functions.md` 中定义的双分支模式：

- 使用 `__cpp_concepts` 作为功能检测宏。
- C++20 分支：`requires(...)` 或 `concept`。
- C++20 之前的分支：SFINAE（`enable_if_t`、`void_t + decltype`）。
- 两个分支的约束条件在逻辑上必须等价。

### 单元测试

- 测试框架：Catch2 v2.13.10（单头文件）。
- 所有测试放在 `test/` 目录下。
- 测试文件使用 `.cc` 扩展名。
- 测试的 `CMakeLists.txt` 使用 `globSources` 函数根据配置的 C++ 标准（`CCC_TEST_CPP_STANDARD`）条件性地包含测试。
- 预编译头：`test/test_pch.hh`。
- 测试通过运行脚本 `scripts/Run-UnitTest.ps1`。
- 除非得到用户提示，否则不运行 `scripts/Run-InstallTest.ps1`

### 构建系统

- CMake 3.28 及以上。
- 根目录 `CMakeLists.txt` 定义 `ccc` 库目标（头文件模式为 INTERFACE，模块模式为 STATIC/SHARED）。
- 构建目录（`cmake-build-*`、`build_*`、`build-*`）已加入 `.gitignore`，不应手动修改其中的文件。
- 启用测试：`-DCCC_BUILD_TESTS=ON`；启用模块构建：`-DCCC_USE_CPP_MODULES=ON`。

### mcpp 构建与测试

除 CMake 外，项目也支持使用 [mcpp](https://github.com/mcpp-community/mcpp) 以 C++ 模块方式构建和运行测试。相关配置位于仓库根目录：

- `mcpp.toml` — 工作空间与 `ccc` 库配置
- `build.mcpp` — 构建脚本，将 `modules/*.ccm` 生成到 `generated/modules/*.cppm`
- `test/mcpp.toml` — `ccc-test` 测试包配置

mcpp 会在首次构建时自动安装默认工具链，无需在 CI 中手写安装流程。用法参考 mcpp 仓库的 [`.agents/skills/mcpp-usage/SKILL.md`](https://github.com/mcpp-community/mcpp/blob/main/.agents/skills/mcpp-usage/SKILL.md) 和 [`docs/`](https://github.com/mcpp-community/mcpp/tree/main/docs) 目录。

本地运行测试：

```sh
mcpp build -p test
mcpp run -p test
```

### 代码风格

- 根目录 `.clang-format` 使用 Google 风格并做了定制（4 空格缩进、120 列宽限制、禁用参数装箱等）。
- 头文件包含保护模式：`#pragma once` + `#ifndef`/`#define`/`#endif`。
- 分支与循环语句（`if`、`else`、`switch`、`for`、`while`、`do`-`while` 等）即使内部只有一行也必须加上大括号。
- 类的 `private` / `protected` 成员（成员变量、成员函数、嵌套类型等）的命名均以下划线结尾（例如 `int value_;`、`void helper_();`），`public` 成员不使用下划线结尾。

### 补充说明

项目级的新约束或编码规范应添加为 `.agents/instructions/` 下的 markdown 文件。Skill 文件统一存放在 `.agents/skills/` 下。

### Git 提交规范

- 提交信息统一使用英文，清晰描述变更内容和原因。
- 提交信息第一行为一句简洁的总结，基于最近一次提交与当前变更之间的差异概括。
- 若变更较复杂，可在第一行后空一行，以 `- ` 开头的条目补充原因、内容和影响；简单变更可省略。
- 提交信息每行首字母大写，不以句号结尾，不使用 Markdown 语法。
- 最后一行标注 AI 辅助信息，格式为
  `Assisted-by: AGENT_NAME:MODEL_VERSION[, AGENT_NAME2:MODEL_VERSION2, ...] [TOOL1] [TOOL2]...`
  - AGENT_NAME：你使用的 AI 工具、框架或智能体的名称（例如 Claude, Copilot, Codex 等）。
  - MODEL_VERSION：具体调用的模型版本（例如 claude-3-opus, gpt-4 等）。
  - `[TOOL1]` `[TOOL2]`（可选）：搭配使用的专业代码分析工具（例如 coccinelle, sparse, smatch, clang-tidy 等）。
  - 若 MODEL_VERSION 包含空格，需用英文双引号将其包裹，例如 "K2.7 Code"。
  - 若包含多个 AGENT_NAME:MODEL_VERSION，则用逗号分隔。
  - 示例：
    - `Assisted-by: Codex:ChatGPT-4.5`
    - `Assisted-by: OpenCode:deepseek-v4-pro clang-tidy`
    - `Assisted-by: Claude:claude-3-opus coccinelle sparse`
    - `Assisted-by: Kimi Code:"K2.7 Code", OpenCode:GLM-5.1 clang-tidy`
- 多行提交信息使用多个 `-m` 参数分行，不要用 `\n` 内嵌换行。
