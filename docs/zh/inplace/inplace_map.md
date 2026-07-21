# ccc.inplace.inplace_map

[English](../../en/inplace/inplace_map.md) | 简体中文

---

`ccc::inplace_map<Key, T, N>` 是一个容量在编译期固定的有序关联容器，基于两个 `ccc::inplace_vector` 分别存储键和值。所有元素存放在栈上，不涉及任何堆分配。

## 特性

- 完全在栈上的固定容量有序存储，键按 `Compare` 严格弱序排列。
- 兼容 C++11（部分成员函数在 C++17 / C++20 下为 `constexpr`）。
- 接口风格贴合 `std::map`，支持 `insert`、`emplace`、`try_emplace`、`insert_or_assign`、`operator[]`、`find`、`lower_bound` 等标准操作。
- 提供完整的迭代器（正向/反向/const），可与 STL 算法无缝配合。
- 支持 `<=>`（C++20）和传统比较运算符（C++20 之前）。
- 提供 `keys()` 和 `values()` 观察器，可直接访问底层键/值容器。
- 提供非成员 `swap`，支持 ADL。

## 快速使用

引入头文件：

```cpp
#include "ccc/inplace/inplace_map.hh"
```

使用 C++20 模块时：

```cpp
import ccc.inplace.inplace_map;
```

### 基本用法

```cpp
ccc::inplace_map<int, std::string, 10> m;
m.insert({1, "one"});
m[2] = "two";
m.emplace(3, "three");

for (const auto& [k, v] : m) {
    // 按 key 有序遍历: (1, "one"), (2, "two"), (3, "three")
}
```

### 响应容量超限

```cpp
ccc::inplace_map<int, std::string, 3> m;
m[1] = "a";
m[2] = "b";
m[3] = "c";

// operator[] 在满时抛出 std::bad_alloc
try {
    m[4] = "d";
} catch (const std::bad_alloc&) {
    // 容量已满
}

// insert 在满时同样抛出 std::bad_alloc
try {
    m.insert({5, "e"});
} catch (const std::bad_alloc&) {
    // 容量已满
}
```

## 模板参数

```cpp
template<typename Key,
         typename T,
         std::size_t N,
         typename Compare = std::less<Key>,
         typename KeyContainer = ccc::inplace_vector<Key, N>,
         typename MappedContainer = ccc::inplace_vector<T, N>>
class inplace_map;
```

| 参数 | 说明 |
|------|------|
| `Key` | 键类型 |
| `T` | 映射值类型 |
| `N` | 编译期固定容量，必须大于 0 |
| `Compare` | 键的比较函数对象，默认为 `std::less<Key>` |
| `KeyContainer` | 键的底层容器，默认为 `ccc::inplace_vector<Key, N>` |
| `MappedContainer` | 值的底层容器，默认为 `ccc::inplace_vector<T, N>` |

## 成员类型

| 类型 | 定义 |
|------|------|
| `key_type` | `Key` |
| `mapped_type` | `T` |
| `value_type` | `std::pair<const Key, T>` |
| `key_compare` | `Compare` |
| `reference` | `std::pair<const Key&, T&>` |
| `const_reference` | `std::pair<const Key&, const T&>` |
| `size_type` | `std::size_t` |
| `difference_type` | `std::ptrdiff_t` |
| `iterator` | 随机访问迭代器（按索引访问键值对） |
| `const_iterator` | const 迭代器 |
| `reverse_iterator` | 反向迭代器 |
| `const_reverse_iterator` | const 反向迭代器 |
| `key_container_type` | `KeyContainer` |
| `mapped_container_type` | `MappedContainer` |
| `value_compare` | 按 `key_compare` 比较 `value_type` 的函数对象 |

## 容量操作

### `capacity` / `max_size`

```cpp
static constexpr size_type capacity() noexcept;
static constexpr size_type max_size() noexcept;
```

返回编译期容量 `N`。两者始终返回相同的值。

## 大小与状态

| 成员函数 | 说明 |
|----------|------|
| `size() const noexcept` | 当前元素个数 |
| `empty() const noexcept` | 是否为空 |
| `full() const noexcept` | 是否已满（`size() == capacity()`） |

## 元素访问

### `at`

```cpp
mapped_type&       at(const key_type& key);
const mapped_type& at(const key_type& key) const;
```

返回指定键对应的值引用。若键不存在，抛出 `std::out_of_range`。

### `operator[]`

```cpp
mapped_type& operator[](const key_type& key);
mapped_type& operator[](key_type&& key);
```

查找指定键，若存在则返回对应值引用；否则在有序位置插入该键并以值默认构造的形式创建新元素，返回其值引用。**若容器已满，抛出 `std::bad_alloc`**。

## 迭代器

| 成员函数 | 说明 |
|----------|------|
| `begin()` / `end()` | 正向迭代器（按 key 有序） |
| `cbegin()` / `cend()` | const 正向迭代器 |
| `rbegin()` / `rend()` | 反向迭代器 |
| `crbegin()` / `crend()` | const 反向迭代器 |

迭代器解引用后产生 `std::pair<const Key&, T&>`（或 const 版本），满足随机访问迭代器要求。

## 构造与析构

### 默认构造

```cpp
constexpr inplace_map();
constexpr explicit inplace_map(const key_compare& comp);
```

构造空容器。可显式传入比较器。

### 范围构造

```cpp
template<typename InputIt>
inplace_map(InputIt first, InputIt last);
template<typename InputIt>
inplace_map(InputIt first, InputIt last, const key_compare& comp);
```

从范围插入元素构建容器。若元素数量超过 `N`，抛出 `std::bad_alloc`。

### initializer_list 构造

```cpp
inplace_map(std::initializer_list<value_type> ilist);
inplace_map(std::initializer_list<value_type> ilist, const key_compare& comp);
```

从初始化列表构建容器。若元素数量超过 `N`，抛出 `std::bad_alloc`。

### 拷贝 / 移动构造

```cpp
inplace_map(const inplace_map& other);
inplace_map(inplace_map&& other) noexcept(/* ... */);
```

深拷贝或移动键和值容器。移动构造的 noexcept 条件取决于底层容器的移动构造函数是否 `noexcept`。

### 析构

```cpp
~inplace_map();
```

默认析构，销毁所有元素。

## 赋值

```cpp
inplace_map& operator=(const inplace_map& other);
inplace_map& operator=(inplace_map&& other) noexcept(/* ... */);
inplace_map& operator=(std::initializer_list<value_type> ilist);
```

均提供自赋值保护。初始化列表赋值会先 `clear()` 再 `insert`。

## 修改操作

### `clear`

```cpp
void clear() noexcept;
```

清空所有键值对。

### `insert`

```cpp
std::pair<iterator, bool> insert(const value_type& value);
std::pair<iterator, bool> insert(value_type&& value);
iterator insert(const_iterator hint, const value_type& value);
iterator insert(const_iterator hint, value_type&& value);
template<typename InputIt> void insert(InputIt first, InputIt last);
void insert(std::initializer_list<value_type> ilist);
```

插入键值对。若键已存在，则不做任何修改并返回 `false`；否则插入并返回 `true`。
`hint` 版本在 hint 正确时可优化插入位置判断。满时抛出 `std::bad_alloc`。

### `insert_or_assign`

```cpp
std::pair<iterator, bool> insert_or_assign(const key_type& key, mapped_type&& value);
std::pair<iterator, bool> insert_or_assign(key_type&& key, mapped_type&& value);
```

若键已存在，替换其值并返回 `{it, false}`；否则插入新键值对并返回 `{it, true}`。满时抛出 `std::bad_alloc`。

### `emplace`

```cpp
template<typename... Args>
std::pair<iterator, bool> emplace(key_type&& key, Args&&... args);
template<typename... Args>
iterator emplace_hint(const_iterator hint, key_type&& key, Args&&... args);
```

原地构造值。`emplace` 的签名为 `(key_type&& key, Args&&... args)`，值通过 `MappedContainer::emplace` 原地构造。若键已存在则不做修改。

### `try_emplace`

```cpp
template<typename... Args>
std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args);
template<typename... Args>
std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args);
template<typename... Args>
iterator try_emplace(const_iterator hint, const key_type& key, Args&&... args);
template<typename... Args>
iterator try_emplace(const_iterator hint, key_type&& key, Args&&... args);
```

若键已存在，不做任何操作；否则原地构造插入。满时抛出 `std::bad_alloc`。

### `erase`

```cpp
size_type erase(const key_type& key);
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

- 按键删除：返回删除的数量（0 或 1）。
- 按位置/范围删除：返回被删除元素之后的位置。

### `swap`

```cpp
void swap(inplace_map& other) noexcept(/* ... */);
```

交换两个容器的内容。要求模板参数完全相同。

## 查找操作

| 成员函数 | 说明 |
|----------|------|
| `find(const key_type& key)` | 查找键，返回对应迭代器；未找到返回 `end()` |
| `contains(const key_type& key) const` | 判断是否包含指定键 |
| `count(const key_type& key) const` | 返回键的数量（0 或 1） |
| `lower_bound(const key_type& key)` | 返回首个不小于 `key` 的迭代器 |
| `upper_bound(const key_type& key)` | 返回首个大于 `key` 的迭代器 |
| `equal_range(const key_type& key)` | 返回 `{lower_bound, upper_bound}` 区间 |

## 观察器

| 成员函数 | 说明 |
|----------|------|
| `key_comp() const` | 返回键比较器副本 |
| `value_comp() const` | 返回值比较器副本（按 key 比较） |
| `keys() const noexcept` | 返回底层键容器的 const 引用 |
| `values() const noexcept` | 返回底层值容器的 const 引用 |

## 自由函数

### `ccc::swap`

```cpp
template<typename Key, typename T, std::size_t N, typename Compare,
         typename KeyContainer, typename MappedContainer>
void swap(inplace_map<Key, T, N, Compare, KeyContainer, MappedContainer>& a,
          inplace_map<Key, T, N, Compare, KeyContainer, MappedContainer>& b) noexcept(noexcept(a.swap(b)));
```

非成员 `swap`，支持 ADL。

## 比较运算符

**C++20 及以上**：提供 `operator==` 和 `operator<=>`（三路比较），基于 `std::lexicographical_compare_three_way`。

**C++20 之前**：提供全套六种比较运算符（`==`、`!=`、`<`、`>`、`<=`、`>=`），基于 `std::lexicographical_compare`。

比较基于键值对的字典序。

## 注意事项

- `inplace_map` 的容量在编译期固定，不可动态增长。超出容量时大部分修改操作抛出 `std::bad_alloc`，`at` 在键不存在时抛出 `std::out_of_range`。
- `operator[]` 在键不存在时会默认构造值并插入，若容器已满则抛出 `std::bad_alloc`。
- 键必须满足 `Compare` 的严格弱序要求，否则行为未定义。
- 由于底层由两个并行的 `inplace_vector` 维护，插入/删除后迭代器可能失效，语义与 `std::vector` 一致。
- `inplace_map` 不保证节点稳定性（所有元素存储在连续的栈缓冲区中）。
- 值类型 `T` 和键类型 `Key` 的析构函数必须可访问且不抛出异常。

完整的用法示例请参阅 `test/inplace/inplace_map_test.cc`。
