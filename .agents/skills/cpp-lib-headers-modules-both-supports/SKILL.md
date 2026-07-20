---
name: cpp-lib-headers-modules-both-supports
description: Design and implement a C++ library that supports both traditional header inclusion and C++20 modules simultaneously. Use when building a C++ library that needs to be consumable via `#include` (headers) and `import` (modules) from the same source, with patterns for config macros, pure-header libraries, component dependencies, and libraries with source files.
---

# C++ Library: Headers & Modules (Implicit Include Pattern)

指导如何设计一个 C++ 库，使其能同时通过头文件 (`#include`) 和 C++20 模块 (`import`) 两种方式被使用。

核心思路：**头文件为纯 C++ 代码（无 `export` 关键字），无条件包含所有依赖，`.cppm` 中使用 `export namespace { using }` 选择性导出符号。**

## 配置头文件

将配置头文件放在 `include/project_name/detail/config.hpp` 路径下，定义 `PROJECT_NAME_USE_MODULES` 宏：

```cpp
#pragma once
#ifndef PROJECT_NAME_DETAIL_CONFIG_HPP
#define PROJECT_NAME_DETAIL_CONFIG_HPP

#ifndef PROJECT_NAME_USE_MODULES
#define PROJECT_NAME_USE_MODULES 0
#endif

#endif  // !PROJECT_NAME_DETAIL_CONFIG_HPP
```

本模式仅需 `PROJECT_NAME_USE_MODULES` 一个配置宏，头文件代码无需 `export` 等模块关键字。

## 纯头文件库

### 头文件 (`lib.hpp`)

```cpp
#pragma once
#ifndef LIB_HPP
#define LIB_HPP

#include <cstddef>                        // 标准库头文件：无条件包含
#include "lib/b.hpp"                      // 项目模块依赖：无条件包含（隐式）

namespace lib::detail {
    // 不导出的实现细节
}

namespace lib {
    int func(int a) { return a; }         // 无需 export 宏
    struct Type {};
    struct UnExportedType {};             // 不导出
}

#endif  // !LIB_HPP
```

### 模块接口单元 (`lib.cppm`)

```cpp
module;

#include "lib.hpp"                        // 头文件在全局模块段

export module lib;

// 无需 import 项目模块，依赖已由头文件隐式包含

export namespace lib {                    // 选择性导出
    using ::lib::func;
    using ::lib::Type;
    // UnExportedType 不列出，不导出
}
```

## 组件依赖模式

如果 `lib/a.hpp` 依赖 `lib/b.hpp`，且两者都提供模块：

### `lib/a.hpp`

```cpp
#pragma once
#ifndef LIB_A_HPP
#define LIB_A_HPP

#include <cstddef>
#include "lib/b.hpp"                      // 无条件包含（隐式）

namespace lib {
    void funcA();
}

#endif
```

### `lib.a.cppm`

```cpp
module;

#include "lib/a.hpp"

export module lib.a;

// 无需 import lib.b，依赖已由头文件隐式包含

export namespace lib {
    using ::lib::funcA;
}
```

## 存在源文件的库

**头文件**：与纯头文件库模式相同。

**源文件（`lib.cpp`）**：使用 `PROJECT_NAME_USE_MODULES` 进行条件编译：

```cpp
#if (defined(PROJECT_NAME_USE_MODULES) && PROJECT_NAME_USE_MODULES)
module;
#include <cstddef>                        // 全局模块段包含标准库头文件
module lib;                               // 模块实现单元
import lib.b;                             // import 替代 #include
#else
#include "lib.hpp"                        // 头文件模式
#endif

// 实现代码...
```

## 隐式包含规则

在 `.hpp` 文件中，所有 `#include` 均无条件包含。
`.cppm` 文件不 `import` 项目模块，项目依赖完全由头文件的 `#include` 隐式提供。
