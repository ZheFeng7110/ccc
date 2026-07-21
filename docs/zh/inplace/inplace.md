# ccc.inplace

[English](../../en/inplace/inplace.md) | 简体中文

---

`ccc.inplace` 提供一系列"就地"（in-place）容器，其内存在栈上分配、容量在编译期固定。与传统动态内存容器（如 `std::vector`）不同，inplace 容器不涉及堆分配，适合对性能敏感或禁止动态内存的场景。

## 特性

- 栈上存储，无堆分配，缓存友好。
- 编译期固定容量，超出容量时抛出异常（或通过 `try_*` 系列接口优雅处理）。
- 接口风格贴合 C++ 标准库容器，支持迭代器、范围操作、STL 算法。
- 兼容 C++11 及以上版本。

## 当前组件

| 组件 | 模块名                                               | 说明 |
|------|---------------------------------------------------|------|
| `inplace_vector<T, N>` | [ccc.inplace.inplace_vector](./inplace_vector.md) | C++26 `std::inplace_vector` 的低版本实现 |
| `inplace_map<Key, T, N>` | [ccc.inplace.inplace_map](./inplace_map.md) | 容量固定的有序关联容器 |

## 快速使用

引入头文件：

```cpp
#include "ccc/inplace.hh"  // 引入全部 inplace 组件
```

或仅引入 inplace_vector：

```cpp
#include "ccc/inplace/inplace_vector.hh"
```

或仅引入 inplace_map：

```cpp
#include "ccc/inplace/inplace_map.hh"
```

使用 C++20 模块时：

```cpp
import ccc.inplace;                  // 引入全部 inplace 组件
import ccc.inplace.inplace_vector;   // 仅引入 inplace_vector
import ccc.inplace.inplace_map;      // 仅引入 inplace_map
```

