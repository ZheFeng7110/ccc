---
name: cpp-lib-headers-modules-both-supports
description: Design and implement a C++ library that supports both traditional header inclusion and C++20 modules simultaneously. Use when building a C++ library that needs to be consumable via `#include` (headers) and `import` (modules) from the same source, with patterns for config macros, pure-header libraries, component dependencies, and libraries with source files.
---

# C++ Library: Headers & Modules

指导如何设计一个 C++ 库，使其能同时通过头文件 (`#include`) 和 C++20 模块 (`import`) 两种方式被使用。

## 配置头文件

将配置头文件放在 `include/project_name/detail/config.hpp` 路径下，定义以下宏：

```cpp
#ifdef PROJECT_NAME_MODULE_INTERFACE_UNIT

#define PROJECT_NAME_MODULE_EXPORT       export
#define PROJECT_NAME_MODULE_EXPORT_BEGIN export {
#define PROJECT_NAME_MODULE_EXPORT_END   }

#else

#define PROJECT_NAME_MODULE_EXPORT
#define PROJECT_NAME_MODULE_EXPORT_BEGIN
#define PROJECT_NAME_MODULE_EXPORT_END

#endif
```

该头文件必须在每一个库头文件中包含，以确保在模块单元中正确导出符号，在非模块单元中忽略 export。

在头文件中使用宏标记导出：

```cpp
#include "project_name/detail/config.hpp"

PROJECT_NAME_MODULE_EXPORT
namespace project_name { /* ... */ }

PROJECT_NAME_MODULE_EXPORT_BEGIN
/* 需要导出的类、函数或变量 */
PROJECT_NAME_MODULE_EXPORT_END
```

## 纯头文件库

如果库的实现是纯头文件的（例如全是模板），按以下模式设计。

### 头文件 (`lib.hpp`)

```cpp
#pragma once
#ifndef LIB_HPP
#define LIB_HPP

#include "project_name/detail/config.hpp"

#ifndef PROJECT_NAME_MODULE_INTERFACE_UNIT
// 在此处包含需要的 C/C++ 标准库头文件
#endif

/* 库的实现，用 PROJECT_NAME_MODULE_EXPORT 标记导出 */

#endif // !LIB_HPP
```

### 模块接口单元 (`lib.cppm`)

```cpp
module;

// 在此处包含需要的 C/C++ 标准库头文件

#define PROJECT_NAME_MODULE_INTERFACE_UNIT

export module lib;  // 模块名称与头文件名称相同

#ifdef __clang__
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include "lib.hpp"
```

头文件嵌套路径与模块名称的对应关系：头文件 `lib/component.hpp` 对应模块名 `lib.component`。

### 组件依赖模式

如果 `lib/a.hpp` 依赖 `lib/b.hpp`，且两者都提供模块：

#### `lib/b.hpp` 及对应模块
按上述纯头文件模式设计。

#### `lib/a.hpp`

```cpp
#pragma once
#ifndef LIB_A_HPP
#define LIB_A_HPP

#include "project_name/detail/config.hpp"

#ifndef PROJECT_NAME_MODULE_INTERFACE_UNIT
// 在此处包含需要的 C/C++ 标准库头文件
#include "lib/b.hpp"  // 依赖的头文件
#endif

/* 库的实现 */

#endif // !LIB_A_HPP
```

#### `lib.a.cppm`

```cpp
module;

// 在此处包含需要的 C/C++ 标准库头文件

#define PROJECT_NAME_MODULE_INTERFACE_UNIT

export module lib.a;

import lib.b;  // 导入依赖的模块（可以视情况 `export import`）

#ifdef __clang__
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include "lib/a.hpp"
```

## 存在源文件的库

如果库的实现存在 `.cpp` 源文件，对外提供头文件：

- **头文件**：参考上述纯头文件模式设计
- **源文件**作为模块实现单元：

```cpp
// lib.cpp

#ifdef PROJECT_NAME_USE_MODULES
module;

// 在此处包含需要的 C/C++ 标准库头文件

module lib;  // 与模块接口单元名称相同

// 在此处 import 依赖的其他模块（如果有）
#else
#include "lib.hpp"  // 包含自己对应的头文件

// 在此处包含需要的 C/C++ 标准库头文件和其他依赖
#endif

// 源文件实现...
```
