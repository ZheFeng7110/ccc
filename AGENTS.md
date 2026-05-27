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
- 配置宏位于 `include/ccc/detail/config.hh`。使用 `CCC_MODULE_INTERFACE_UNIT` 控制模块专属行为，使用 `CCC_MODULE_EXPORT` / `CCC_MODULE_EXPORT_BEGIN` / `CCC_MODULE_EXPORT_END` 标记导出符号。
- 头文件放在 `include/ccc/` 下，使用 `#pragma once` + `#ifndef`/`#define` 双重包含保护。
- 模块接口单元（`.ccm`）放在 `modules/` 目录下。
- 模块名与头文件路径对应：`include/ccc/foo/bar.hh` 对应 `export module ccc.foo.bar;`
- 标准库头文件的 `#include` 放在头文件中 `#ifndef CCC_MODULE_INTERFACE_UNIT` 守卫之外；在 `.ccm` 文件中则放在全局模块片段（`module;` 之后）。
- 组件间存在依赖时，`.ccm` 文件需 `import` 被依赖的模块（若被依赖模块也是对外 API 的一部分，则使用 `export import`）。

### 命名空间与模块导出

- 面向用户的功能定义在 `ccc` 命名空间内，并通过模块 `CCC_MODULE_EXPORT` / `CCC_MODULE_EXPORT_BEGIN` / `CCC_MODULE_EXPORT_END` 导出。
- 内部实现细节定义在 `ccc::detail` 命名空间内，**不**通过模块导出（不在 `.ccm` 文件的 `export` 块内）。

### 函数约束：SFINAE / Concepts

当函数或模板需要按条件参与重载决议时，遵循 `.agents/instructions/constraint-for-functions.md` 中定义的双分支模式：

- 使用 `__cpp_concepts` 作为功能检测宏。
- C++20 分支：`requires(...)` 或 `concept`。
- C++20 之前的分支：SFINAE（`enable_if_t`、`void_t + decltype`）。
- 两个分支的约束条件在逻辑上必须等价。

### 单元测试

- 测试框架：Google Test（gtest），通过 CPM 自动拉取。
- 所有测试放在 `test/` 目录下，每个组件使用独立子目录（如 `test/expected_test/`、`test/inplace_vector_test/`）。
- 测试文件使用 `.cc` 扩展名。
- 测试的 `CMakeLists.txt` 使用 `globSources` 函数根据配置的 C++ 标准（`CCC_TEST_CPP_STANDARD`）条件性地包含测试。
- 预编译头：`test/test_pch.hh`。
- 测试时需覆盖**使用模块**和**不使用模块**两种情形，统一使用 C++20（`-DCCC_TEST_CPP_STANDARD=20`）。

  **不使用模块（头文件模式）：**
  ```sh
  cmake -S . -B build -DCCC_BUILD_TESTS=ON -DCCC_TEST_CPP_STANDARD=20
  cmake --build build
  ctest --test-dir build
  ```

  **使用模块：**
  ```sh
  cmake -S . -B build_modules -DCCC_BUILD_TESTS=ON -DCCC_TEST_CPP_STANDARD=20 -DCCC_USE_CPP_MODULES=ON
  cmake --build build_modules
  ctest --test-dir build_modules
  ```

### 构建系统

- CMake 3.28 及以上。
- 根目录 `CMakeLists.txt` 定义 `ccc` 库目标（头文件模式为 INTERFACE，模块模式为 STATIC/SHARED）。
- 构建目录（`cmake-build-*`、`build_*`、`build-*`）已加入 `.gitignore`，不应手动修改其中的文件。
- 启用测试：`-DCCC_BUILD_TESTS=ON`；启用模块构建：`-DCCC_USE_CPP_MODULES=ON`。

### 代码风格

- 根目录 `.clang-format` 使用 Google 风格并做了定制（4 空格缩进、120 列宽限制、禁用参数装箱等）。
- 头文件包含保护模式：`#pragma once` + `#ifndef`/`#define`/`#endif`。
- 分支与循环语句（`if`、`else`、`switch`、`for`、`while`、`do`-`while` 等）即使内部只有一行也必须加上大括号。
- 类的 `private` / `protected` 成员（成员变量、成员函数、嵌套类型等）的命名均以下划线结尾（例如 `int value_;`、`void helper_();`），`public` 成员不使用下划线结尾。

### 补充说明

项目级的新约束或编码规范应添加为 `.agents/instructions/` 下的 markdown 文件。Skill 文件统一存放在 `.agents/skills/` 下。
