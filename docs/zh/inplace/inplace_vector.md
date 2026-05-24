# ccc.inplace.inplace_vector

[English](../en/inplace/inplace_vector.md) | 简体中文

---

`ccc::inplace_vector<T, N>` 是 C++26 `std::inplace_vector` 的低版本实现，提供容量在编译期固定的连续容器。元素存储在容器内部的栈上缓冲区中，不涉及任何堆分配。

## 特性

- 完全在栈上的固定容量连续存储，与 `std::array` 相同的内存布局优势，同时提供类似 `std::vector` 的动态大小。
- 兼容 C++11（部分成员函数在 C++17 / C++20 下为 `constexpr`）。
- 丰富的元素操作接口：`push_back`、`emplace_back`、`insert`、`emplace`、`erase`、`resize` 等。
- 提供 `try_*` 和 `unchecked_*` 系列接口，支持无异常的容量检查与高性能路径。
- 完全支持迭代器（正向/反向/const），可与 STL 算法无缝配合。
- 支持 `<=>`（C++20）和传统比较运算符（C++20 之前）。
- 提供自由函数 `ccc::erase` / `ccc::erase_if` 及 `ccc::swap`。

## 快速使用

引入头文件：

```cpp
#include "ccc/inplace/inplace_vector.hh"
```

使用 C++20 模块时：

```cpp
import ccc.inplace.inplace_vector;
```

### 基本用法

```cpp
ccc::inplace_vector<int, 10> v;
v.push_back(1);
v.push_back(2);
v.push_back(3);

for (int x : v) {
    // 使用 x
}

// 与 STL 算法配合
std::sort(v.begin(), v.end());
```

### 响应容量超限

```cpp
ccc::inplace_vector<int, 5> v;

// push_back 在满时抛出 std::bad_alloc
try {
    for (int i = 0; i < 10; ++i) v.push_back(i);
} catch (const std::bad_alloc&) {
    // 容量已满
}

// try_push_back 在满时返回 nullptr，不抛异常
auto* p = v.try_push_back(42);
if (p) {
    // 插入成功
}

// unchecked_push_back 不检查容量，调用者需自行保证未满
v.unchecked_push_back(99);
```

## 模板参数

```cpp
template<typename T, std::size_t N>
class inplace_vector;
```

| 参数 | 说明 |
|------|------|
| `T` | 元素类型 |
| `N` | 编译期固定容量，必须大于 0 |

## 成员类型

| 类型 | 定义 |
|------|------|
| `value_type` | `T` |
| `size_type` | `std::size_t` |
| `difference_type` | `std::ptrdiff_t` |
| `reference` | `T&` |
| `const_reference` | `const T&` |
| `pointer` | `T*` |
| `const_pointer` | `const T*` |
| `iterator` | `T*`（随机访问迭代器） |
| `const_iterator` | `const T*` |
| `reverse_iterator` | `std::reverse_iterator<iterator>` |
| `const_reverse_iterator` | `std::reverse_iterator<const_iterator>` |

## 容量操作

### `capacity` / `max_size`

```cpp
static constexpr size_type capacity() noexcept;
static constexpr size_type max_size() noexcept;
```

返回编译期容量 `N`。两者始终返回相同的值。

### `reserve`

```cpp
static constexpr void reserve(size_type new_cap);
```

若 `new_cap > N`，抛出 `std::bad_alloc`；否则无操作。提供此接口以兼容泛型代码。

### `shrink_to_fit`

```cpp
static constexpr void shrink_to_fit() noexcept;
```

无操作。因为容量固定，无法收缩。

## 大小与状态

| 成员函数 | 说明 |
|----------|------|
| `size() const noexcept` | 当前元素个数 |
| `empty() const noexcept` | 是否为空 |
| `full() const noexcept` | 是否已满（`size() == capacity()`） |

## 元素访问

| 成员函数 | 说明 |
|----------|------|
| `operator[](i)` | 不检查边界的随机访问，越界时通过断言报告（debug 模式下） |
| `at(i)` | 带边界检查的随机访问，越界时抛出 `std::out_of_range` |
| `front()` | 首元素引用，空时断言失败 |
| `back()` | 末元素引用，空时断言失败 |
| `data()` | 指向底层存储的指针 |

## 迭代器

| 成员函数 | 说明 |
|----------|------|
| `begin()` / `end()` | 正向迭代器 |
| `cbegin()` / `cend()` | const 正向迭代器 |
| `rbegin()` / `rend()` | 反向迭代器 |
| `crbegin()` / `crend()` | const 反向迭代器 |

由于存储是连续的，迭代器即为裸指针，满足随机访问迭代器要求。

## 构造与析构

### 默认构造

```cpp
constexpr inplace_vector() noexcept;
```

构造空容器。

### 拷贝 / 移动构造

```cpp
inplace_vector(const inplace_vector& other);
inplace_vector(inplace_vector&& other) noexcept(/* ... */);
```

深拷贝或移动元素。移动构造的 noexcept 条件取决于 `T` 的移动构造函数是否 `noexcept`。

### initializer_list 构造

```cpp
inplace_vector(std::initializer_list<T> ilist);
```

若 `ilist.size() > N`，抛出 `std::bad_alloc`。

### 析构

```cpp
~inplace_vector();
```

销毁所有元素。

## 赋值

```cpp
inplace_vector& operator=(const inplace_vector& other);
inplace_vector& operator=(inplace_vector&& other) noexcept(/* ... */);
inplace_vector& operator=(std::initializer_list<T> ilist);
```

均提供自赋值保护。

## 修改操作

### `assign`

```cpp
void assign(size_type count, const T& value);
template<typename InputIt> void assign(InputIt first, InputIt last);
void assign(std::initializer_list<T> ilist);
template<typename Range> void assign_range(Range&& range);
```

替换当前内容。`assign_range` 接受任意范围类型。若新元素数量超过 `N`，抛出 `std::bad_alloc`。

### `push_back` / `try_push_back` / `unchecked_push_back`

```cpp
reference       push_back(const T& value);
reference       push_back(T&& value);
pointer         try_push_back(const T& value);
pointer         try_push_back(T&& value);
pointer         unchecked_push_back(const T& value);
pointer         unchecked_push_back(T&& value);
```

| 变体 | 满时的行为 |
|------|------------|
| `push_back` | 抛出 `std::bad_alloc` |
| `try_push_back` | 返回 `nullptr` |
| `unchecked_push_back` | 未定义行为（调用者保证未满） |

### `emplace_back` / `try_emplace_back` / `unchecked_emplace_back`

```cpp
template<typename... Args> reference emplace_back(Args&&... args);
template<typename... Args> pointer   try_emplace_back(Args&&... args);
template<typename... Args> pointer   unchecked_emplace_back(Args&&... args);
```

原地构造元素于末尾，行为类似 `push_back` 系列。

### `pop_back`

```cpp
void pop_back() noexcept;
```

移除末尾元素。若容器为空则无操作。

### `insert`（单个元素）

```cpp
iterator insert(const_iterator pos, const T& value);
iterator insert(const_iterator pos, T&& value);
```

在 `pos` 前插入元素，满时抛出 `std::bad_alloc`。

### `insert`（n 个副本）

```cpp
iterator insert(const_iterator pos, size_type count, const T& value);
```

若 `count == 0` 则为无操作。满时抛出 `std::bad_alloc`。

### `insert`（范围）

```cpp
template<typename InputIt> iterator insert(const_iterator pos, InputIt first, InputIt last);
iterator insert(const_iterator pos, std::initializer_list<T> ilist);
template<typename Range> iterator insert_range(const_iterator pos, Range&& range);
```

从范围插入元素。`insert_range` 接受任意范围类型。满时抛出 `std::bad_alloc`。

### `emplace`

```cpp
template<typename... Args> iterator emplace(const_iterator pos, Args&&... args);
```

在 `pos` 前原地构造元素。满时抛出 `std::bad_alloc`。

### `append_range` / `try_append_range`

```cpp
template<typename Range> void append_range(Range&& range);
template<typename Range> bool try_append_range(Range&& range);
```

在末尾追加一个范围内的所有元素。`try_append_range` 在容量不足时返回 `false` 而非抛出异常。

### `erase`

```cpp
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

移除指定位置或范围内的元素，返回被移除元素之后的位置。

### `clear`

```cpp
void clear() noexcept;
```

销毁所有元素，将 `size()` 置零。

### `resize`

```cpp
void resize(size_type new_size);
void resize(size_type new_size, const T& value);
```

调整大小。若 `new_size > N`，抛出 `std::out_of_range`。

- 扩大：新元素进行默认构造（或使用 `value` 拷贝）。
- 缩小：销毁多余元素。

### `swap`

```cpp
void swap(inplace_vector& other) noexcept(/* ... */);
```

交换两个容器的内容。要求两个容器的 `T` 和 `N` 完全相同。

## 自由函数

### `ccc::swap`

```cpp
template<typename T, std::size_t N>
void swap(inplace_vector<T, N>& a, inplace_vector<T, N>& b) noexcept(noexcept(a.swap(b)));
```

非成员 `swap`，支持 ADL。

### `ccc::erase`

```cpp
template<typename T, std::size_t N, typename U>
typename inplace_vector<T, N>::size_type erase(inplace_vector<T, N>& c, const U& value);
```

移除所有等于 `value` 的元素，返回移除的数量。

### `ccc::erase_if`

```cpp
template<typename T, std::size_t N, typename Pred>
typename inplace_vector<T, N>::size_type erase_if(inplace_vector<T, N>& c, Pred pred);
```

移除所有满足谓词 `pred` 的元素，返回移除的数量。

## 比较运算符

**C++20 及以上**：提供 `operator==` 和 `operator<=>`（三路比较），基于 `std::lexicographical_compare_three_way`。

**C++20 之前**：提供全套六种比较运算符（`==`、`!=`、`<`、`>`、`<=`、`>=`），基于 `std::lexicographical_compare`。

## 注意事项

- `inplace_vector` 的容量在编译期固定，不可动态增长。超出容量时大部分修改操作抛出 `std::bad_alloc`，`resize` 抛出 `std::out_of_range`。
- `try_*` 系列接口提供无异常的错误处理路径，适合禁用异常的环境。
- `unchecked_*` 接口跳过容量检查，调用者须自行保证未满，否则行为未定义。
- `inplace_vector` 的迭代器为裸指针，在插入/删除操作后可能失效，语义与 `std::vector` 一致。
- 由于使用 `alignas(T) unsigned char` 作为底层存储，元素类型 `T` 的析构函数必须可访问且不抛出异常。

完整的用法示例请参阅 `test/inplace_vector_test/inplace_vector_test.cc`。

