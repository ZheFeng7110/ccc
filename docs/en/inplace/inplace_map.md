# ccc.inplace.inplace_map

English | [简体中文](../../zh/inplace/inplace_map.md)

---

`ccc::inplace_map<Key, T, N>` is an ordered associative container with compile-time fixed capacity, built on top of two `ccc::inplace_vector`s that separately store keys and values. All elements reside on the stack with no heap allocation whatsoever.

## Features

- Fully stack-allocated fixed-capacity ordered storage; keys are kept in strict weak ordering according to `Compare`.
- Compatible with C++11 (selected member functions are `constexpr` under C++17 / C++20).
- Interface style follows `std::map`, supporting standard operations such as `insert`, `emplace`, `try_emplace`, `insert_or_assign`, `operator[]`, `find`, `lower_bound`, etc.
- Full iterator support (forward/reverse/const), works seamlessly with STL algorithms.
- Supports `<=>` (C++20) and traditional comparison operators (pre-C++20).
- Provides `keys()` and `values()` observers for direct access to the underlying key/value containers.
- Provides non-member `swap`, supporting ADL.

## Quick Start

Include the header:

```cpp
#include "ccc/inplace/inplace_map.hh"
```

When using C++20 modules:

```cpp
import ccc.inplace.inplace_map;
```

### Basic Usage

```cpp
ccc::inplace_map<int, std::string, 10> m;
m.insert({1, "one"});
m[2] = "two";
m.emplace(3, "three");

for (const auto& [k, v] : m) {
    // iterate in key order: (1, "one"), (2, "two"), (3, "three")
}
```

### Handling Capacity Overflow

```cpp
ccc::inplace_map<int, std::string, 3> m;
m[1] = "a";
m[2] = "b";
m[3] = "c";

// operator[] throws std::bad_alloc when full
try {
    m[4] = "d";
} catch (const std::bad_alloc&) {
    // capacity exhausted
}

// insert also throws std::bad_alloc when full
try {
    m.insert({5, "e"});
} catch (const std::bad_alloc&) {
    // capacity exhausted
}
```

## Template Parameters

```cpp
template<typename Key,
         typename T,
         std::size_t N,
         typename Compare = std::less<Key>,
         typename KeyContainer = ccc::inplace_vector<Key, N>,
         typename MappedContainer = ccc::inplace_vector<T, N>>
class inplace_map;
```

| Parameter | Description |
|-----------|-------------|
| `Key` | Key type |
| `T` | Mapped value type |
| `N` | Compile-time fixed capacity, must be greater than 0 |
| `Compare` | Key comparison function object, defaults to `std::less<Key>` |
| `KeyContainer` | Underlying key container, defaults to `ccc::inplace_vector<Key, N>` |
| `MappedContainer` | Underlying value container, defaults to `ccc::inplace_vector<T, N>` |

## Member Types

| Type | Definition |
|------|------------|
| `key_type` | `Key` |
| `mapped_type` | `T` |
| `value_type` | `std::pair<const Key, T>` |
| `key_compare` | `Compare` |
| `reference` | `std::pair<const Key&, T&>` |
| `const_reference` | `std::pair<const Key&, const T&>` |
| `size_type` | `std::size_t` |
| `difference_type` | `std::ptrdiff_t` |
| `iterator` | Random-access iterator (accesses key-value pairs by index) |
| `const_iterator` | Const iterator |
| `reverse_iterator` | Reverse iterator |
| `const_reverse_iterator` | Const reverse iterator |
| `key_container_type` | `KeyContainer` |
| `mapped_container_type` | `MappedContainer` |
| `value_compare` | Function object that compares `value_type` by key |

## Capacity Operations

### `capacity` / `max_size`

```cpp
static constexpr size_type capacity() noexcept;
static constexpr size_type max_size() noexcept;
```

Returns the compile-time capacity `N`. Both always return the same value.

## Size and State

| Member Function | Description |
|-----------------|-------------|
| `size() const noexcept` | Current number of elements |
| `empty() const noexcept` | Whether the container is empty |
| `full() const noexcept` | Whether the container is full (`size() == capacity()`) |

## Element Access

### `at`

```cpp
mapped_type&       at(const key_type& key);
const mapped_type& at(const key_type& key) const;
```

Returns a reference to the value mapped to the given key. Throws `std::out_of_range` if the key does not exist.

### `operator[]`

```cpp
mapped_type& operator[](const key_type& key);
mapped_type& operator[](key_type&& key);
```

Searches for the given key; if found, returns the corresponding value reference. Otherwise inserts the key at the sorted position with a default-constructed value and returns its reference. **Throws `std::bad_alloc` if the container is full**.

## Iterators

| Member Function | Description |
|-----------------|-------------|
| `begin()` / `end()` | Forward iterators (in key order) |
| `cbegin()` / `cend()` | Const forward iterators |
| `rbegin()` / `rend()` | Reverse iterators |
| `crbegin()` / `crend()` | Const reverse iterators |

Dereferencing an iterator yields `std::pair<const Key&, T&>` (or const variant). Iterators satisfy random-access iterator requirements.

## Construction and Destruction

### Default Constructor

```cpp
constexpr inplace_map();
constexpr explicit inplace_map(const key_compare& comp);
```

Constructs an empty container. An explicit comparator may be provided.

### Range Constructor

```cpp
template<typename InputIt>
inplace_map(InputIt first, InputIt last);
template<typename InputIt>
inplace_map(InputIt first, InputIt last, const key_compare& comp);
```

Constructs the container by inserting elements from a range. Throws `std::bad_alloc` if the number of elements exceeds `N`.

### initializer_list Constructor

```cpp
inplace_map(std::initializer_list<value_type> ilist);
inplace_map(std::initializer_list<value_type> ilist, const key_compare& comp);
```

Constructs the container from an initializer list. Throws `std::bad_alloc` if the number of elements exceeds `N`.

### Copy / Move Constructors

```cpp
inplace_map(const inplace_map& other);
inplace_map(inplace_map&& other) noexcept(/* ... */);
```

Deep-copies or moves the key and value containers. The move constructor's noexcept condition depends on whether the underlying containers' move constructors are `noexcept`.

### Destructor

```cpp
~inplace_map();
```

Default destructor; destroys all elements.

## Assignment

```cpp
inplace_map& operator=(const inplace_map& other);
inplace_map& operator=(inplace_map&& other) noexcept(/* ... */);
inplace_map& operator=(std::initializer_list<value_type> ilist);
```

All provide self-assignment protection. Initializer-list assignment first `clear()`s then `insert`s.

## Modifiers

### `clear`

```cpp
void clear() noexcept;
```

Removes all key-value pairs.

### `insert`

```cpp
std::pair<iterator, bool> insert(const value_type& value);
std::pair<iterator, bool> insert(value_type&& value);
iterator insert(const_iterator hint, const value_type& value);
iterator insert(const_iterator hint, value_type&& value);
template<typename InputIt> void insert(InputIt first, InputIt last);
void insert(std::initializer_list<value_type> ilist);
```

Inserts a key-value pair. If the key already exists, does nothing and returns `false`; otherwise inserts and returns `true`. The `hint` variants can optimize position lookup when the hint is correct. Throws `std::bad_alloc` when full.

### `insert_or_assign`

```cpp
std::pair<iterator, bool> insert_or_assign(const key_type& key, mapped_type&& value);
std::pair<iterator, bool> insert_or_assign(key_type&& key, mapped_type&& value);
```

If the key already exists, replaces its value and returns `{it, false}`; otherwise inserts a new key-value pair and returns `{it, true}`. Throws `std::bad_alloc` when full.

### `emplace`

```cpp
template<typename... Args>
std::pair<iterator, bool> emplace(key_type&& key, Args&&... args);
template<typename... Args>
iterator emplace_hint(const_iterator hint, key_type&& key, Args&&... args);
```

Constructs the value in-place. The signature is `(key_type&& key, Args&&... args)`; the value is constructed via `MappedContainer::emplace`. Does nothing if the key already exists.

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

If the key already exists, does nothing; otherwise constructs and inserts in-place. Throws `std::bad_alloc` when full.

### `erase`

```cpp
size_type erase(const key_type& key);
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

- Erase by key: returns the number of elements removed (0 or 1).
- Erase by position / range: returns the iterator following the last removed element.

### `swap`

```cpp
void swap(inplace_map& other) noexcept(/* ... */);
```

Swaps the contents of two containers. Template parameters must match exactly.

## Lookup Operations

| Member Function | Description |
|-----------------|-------------|
| `find(const key_type& key)` | Finds the key, returns the corresponding iterator; returns `end()` if not found |
| `contains(const key_type& key) const` | Checks whether the specified key exists |
| `count(const key_type& key) const` | Returns the number of elements with the given key (0 or 1) |
| `lower_bound(const key_type& key)` | Returns iterator to the first element not less than `key` |
| `upper_bound(const key_type& key)` | Returns iterator to the first element greater than `key` |
| `equal_range(const key_type& key)` | Returns `{lower_bound, upper_bound}` range |

## Observers

| Member Function | Description |
|-----------------|-------------|
| `key_comp() const` | Returns a copy of the key comparator |
| `value_comp() const` | Returns a copy of the value comparator (compares by key) |
| `keys() const noexcept` | Returns a const reference to the underlying key container |
| `values() const noexcept` | Returns a const reference to the underlying value container |

## Free Functions

### `ccc::swap`

```cpp
template<typename Key, typename T, std::size_t N, typename Compare,
         typename KeyContainer, typename MappedContainer>
void swap(inplace_map<Key, T, N, Compare, KeyContainer, MappedContainer>& a,
          inplace_map<Key, T, N, Compare, KeyContainer, MappedContainer>& b) noexcept(noexcept(a.swap(b)));
```

Non-member `swap`, supports ADL.

## Comparison Operators

**C++20 and later**: provides `operator==` and `operator<=>` (three-way comparison), based on `std::lexicographical_compare_three_way`.

**Pre-C++20**: provides all six comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`), based on `std::lexicographical_compare`.

Comparison is performed lexicographically over key-value pairs.

## Notes

- `inplace_map` has a compile-time fixed capacity that cannot grow dynamically. Most mutating operations throw `std::bad_alloc` on overflow; `at` throws `std::out_of_range` when the key is not found.
- `operator[]` default-constructs a value and inserts it when the key is absent; throws `std::bad_alloc` if the container is full.
- Keys must satisfy the strict weak ordering requirement of `Compare`; otherwise behavior is undefined.
- Because the underlying storage consists of two parallel `inplace_vector`s, iterators may be invalidated by insert/erase operations, consistent with `std::vector` semantics.
- `inplace_map` does not guarantee node stability (all elements are stored in a contiguous stack buffer).
- The destructor of value type `T` and key type `Key` must be accessible and non-throwing.

See `test/inplace/inplace_map_test.cc` for complete usage examples.
