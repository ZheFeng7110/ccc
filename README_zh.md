# ccc

[English](./README.md) | 简体中文

---

ccc 是一个 C++ 实用功能库，提供一系列现代 C++ 工具，包括管道操作符、单子 expected 等更多功能。

## 特性

- 头文件模式和模块模式双重支持，灵活适配不同场景。
- 支持 C++14 及以上版本（部分功能需要 C++17 或 C++20）。

## 快速开始

### 引入库

```cpp
#include "ccc/all.hh"
```

或者，引入特定组件：

```cpp
#include "ccc/expected.hh"
#include "ccc/pipe_operator_helper.hh"
```

使用 C++20 模块（如果已配置）：

```cpp
import ccc;  // 引入所有
import ccc.expected;  // 仅引入 expected 组件
```

## 文档

- `ccc.utility` - 在低版本 C++ 中实现较高版本的 C++ STL 功能的实用工具
- [ccc.pipe_operator_helper](docs/zh/pipe_operator_helper.md) - 管道风格函数调用辅助工具
- [ccc.expected](docs/zh/expected.md) - 支持可选 Criterion 的单子 expected
- [ccc.inplace](docs/zh/inplace/inplace.md) - 栈上分配、编译期固定容量的“就地”容器
- [ccc.resource_guard](docs/zh/resource_guard.md) - 基于 RAII 的资源管理工具，支持 defer、try_finally、with 模式

## 构建

本项目使用 CMake。构建方式：

```bash
cmake -S . -B build
cmake --build build
```

如需模块支持，配置时使用：

```bash
cmake -DCPP_PIPE_OPERATOR_HELPER_USE_MODULES=ON ..
```

## 测试

运行测试：

```bash
cmake -S . -B build -DCCC_BUILD_TESTS=ON
cmake --build build
cd build && ctest .
```

## 许可证

本项目采用 MIT 许可证。
