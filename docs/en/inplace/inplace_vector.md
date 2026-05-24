# ccc.inplace.inplace_vector

English | [简体中文](../../zh/inplace/inplace_vector.md)

---

`ccc::inplace_vector<T, N>` is a lower-version implementation of C++26 `std::inplace_vector`, providing a contiguous container with compile-time fixed capacity. Elements are stored in an internal stack-allocated buffer with no heap allocation whatsoever.

## Features

- Fully stack-allocated fixed-capacity contiguous storage, offering the same memory layout advantages as `std::array` combined with `std::vector`-like dynamic sizing.
- Compatible with C++11 (selected member functions are `constexpr` under C++17 / C++20).
- Rich element manipulation interfaces: `push_back`, `emplace_back`, `insert`, `emplace`, `erase`, `resize`, and more.
- Provides `try_*` and `unchecked_*` variants for exception-free capacity checking and high-performance paths.
- Full iterator support (forward/reverse/const), works seamlessly with STL algorithms.
- Supports `<=>` (C++20) and traditional comparison operators (pre-C++20).
- Provides free functions `ccc::erase`, `ccc::erase_if`, and `ccc::swap`.

## Quick Start

Include the header:

```cpp
#include "ccc/inplace/inplace_vector.hh"
```

When using C++20 modules:

```cpp
import ccc.inplace.inplace_vector;
```

### Basic Usage

```cpp
ccc::inplace_vector<int, 10> v;
v.push_back(1);
v.push_back(2);
v.push_back(3);

for (int x : v) {
    // use x
}

// Works with STL algorithms
std::sort(v.begin(), v.end());
```

### Handling Capacity Overflow

```cpp
ccc::inplace_vector<int, 5> v;

// push_back throws std::bad_alloc when full
try {
    for (int i = 0; i < 10; ++i) v.push_back(i);
} catch (const std::bad_alloc&) {
    // capacity exhausted
}

// try_push_back returns nullptr when full, no exception
auto* p = v.try_push_back(42);
if (p) {
    // insertion succeeded
}

// unchecked_push_back skips the capacity check; caller must guarantee not full
v.unchecked_push_back(99);
```

## Template Parameters

```cpp
template<typename T, std::size_t N>
class inplace_vector;
```

| Parameter | Description |
|-----------|-------------|
| `T` | Element type |
| `N` | Compile-time fixed capacity, must be greater than 0 |

## Member Types

| Type | Definition |
|------|------------|
| `value_type` | `T` |
| `size_type` | `std::size_t` |
| `difference_type` | `std::ptrdiff_t` |
| `reference` | `T&` |
| `const_reference` | `const T&` |
| `pointer` | `T*` |
| `const_pointer` | `const T*` |
| `iterator` | `T*` (random-access iterator) |
| `const_iterator` | `const T*` |
| `reverse_iterator` | `std::reverse_iterator<iterator>` |
| `const_reverse_iterator` | `std::reverse_iterator<const_iterator>` |

## Capacity Operations

### `capacity` / `max_size`

```cpp
static constexpr size_type capacity() noexcept;
static constexpr size_type max_size() noexcept;
```

Returns the compile-time capacity `N`. Both always return the same value.

### `reserve`

```cpp
static constexpr void reserve(size_type new_cap);
```

Throws `std::bad_alloc` if `new_cap > N`; otherwise does nothing. Provided for compatibility with generic code.

### `shrink_to_fit`

```cpp
static constexpr void shrink_to_fit() noexcept;
```

No-op. Capacity is fixed and cannot be reduced.

## Size and State

| Member Function | Description |
|-----------------|-------------|
| `size() const noexcept` | Current number of elements |
| `empty() const noexcept` | Whether the container is empty |
| `full() const noexcept` | Whether the container is full (`size() == capacity()`) |

## Element Access

| Member Function | Description |
|-----------------|-------------|
| `operator[](i)` | Unchecked random access; out-of-bounds triggers an assertion in debug mode |
| `at(i)` | Bounds-checked random access; throws `std::out_of_range` on out-of-bounds |
| `front()` | Reference to the first element; asserts on empty |
| `back()` | Reference to the last element; asserts on empty |
| `data()` | Pointer to the underlying storage |

## Iterators

| Member Function | Description |
|-----------------|-------------|
| `begin()` / `end()` | Forward iterators |
| `cbegin()` / `cend()` | Const forward iterators |
| `rbegin()` / `rend()` | Reverse iterators |
| `crbegin()` / `crend()` | Const reverse iterators |

Since storage is contiguous, iterators are raw pointers satisfying random-access iterator requirements.

## Construction and Destruction

### Default Constructor

```cpp
constexpr inplace_vector() noexcept;
```

Constructs an empty container.

### Copy / Move Constructors

```cpp
inplace_vector(const inplace_vector& other);
inplace_vector(inplace_vector&& other) noexcept(/* ... */);
```

Deep-copies or moves elements. The move constructor's noexcept condition depends on whether `T`'s move constructor is `noexcept`.

### initializer_list Constructor

```cpp
inplace_vector(std::initializer_list<T> ilist);
```

Throws `std::bad_alloc` if `ilist.size() > N`.

### Destructor

```cpp
~inplace_vector();
```

Destroys all elements.

## Assignment

```cpp
inplace_vector& operator=(const inplace_vector& other);
inplace_vector& operator=(inplace_vector&& other) noexcept(/* ... */);
inplace_vector& operator=(std::initializer_list<T> ilist);
```

All provide self-assignment protection.

## Modifiers

### `assign`

```cpp
void assign(size_type count, const T& value);
template<typename InputIt> void assign(InputIt first, InputIt last);
void assign(std::initializer_list<T> ilist);
template<typename Range> void assign_range(Range&& range);
```

Replaces the current contents. `assign_range` accepts any range type. Throws `std::bad_alloc` if the new element count exceeds `N`.

### `push_back` / `try_push_back` / `unchecked_push_back`

```cpp
reference       push_back(const T& value);
reference       push_back(T&& value);
pointer         try_push_back(const T& value);
pointer         try_push_back(T&& value);
pointer         unchecked_push_back(const T& value);
pointer         unchecked_push_back(T&& value);
```

| Variant | Behavior when full |
|---------|--------------------|
| `push_back` | Throws `std::bad_alloc` |
| `try_push_back` | Returns `nullptr` |
| `unchecked_push_back` | Undefined behavior (caller guarantees not full) |

### `emplace_back` / `try_emplace_back` / `unchecked_emplace_back`

```cpp
template<typename... Args> reference emplace_back(Args&&... args);
template<typename... Args> pointer   try_emplace_back(Args&&... args);
template<typename... Args> pointer   unchecked_emplace_back(Args&&... args);
```

Constructs an element in-place at the end. Behavior mirrors the `push_back` family.

### `pop_back`

```cpp
void pop_back() noexcept;
```

Removes the last element. No-op if the container is empty.

### `insert` (Single Element)

```cpp
iterator insert(const_iterator pos, const T& value);
iterator insert(const_iterator pos, T&& value);
```

Inserts an element before `pos`. Throws `std::bad_alloc` when full.

### `insert` (n Copies)

```cpp
iterator insert(const_iterator pos, size_type count, const T& value);
```

No-op when `count == 0`. Throws `std::bad_alloc` when full.

### `insert` (Range)

```cpp
template<typename InputIt> iterator insert(const_iterator pos, InputIt first, InputIt last);
iterator insert(const_iterator pos, std::initializer_list<T> ilist);
template<typename Range> iterator insert_range(const_iterator pos, Range&& range);
```

Inserts elements from a range. `insert_range` accepts any range type. Throws `std::bad_alloc` when full.

### `emplace`

```cpp
template<typename... Args> iterator emplace(const_iterator pos, Args&&... args);
```

Constructs an element in-place before `pos`. Throws `std::bad_alloc` when full.

### `append_range` / `try_append_range`

```cpp
template<typename Range> void append_range(Range&& range);
template<typename Range> bool try_append_range(Range&& range);
```

Appends all elements from a range to the end. `try_append_range` returns `false` on insufficient capacity instead of throwing.

### `erase`

```cpp
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

Removes elements at the specified position or range. Returns the position after the removed elements.

### `clear`

```cpp
void clear() noexcept;
```

Destroys all elements and sets `size()` to zero.

### `resize`

```cpp
void resize(size_type new_size);
void resize(size_type new_size, const T& value);
```

Resizes the container. Throws `std::out_of_range` if `new_size > N`.

- Enlarging: new elements are default-constructed (or copy-constructed from `value`).
- Shrinking: excess elements are destroyed.

### `swap`

```cpp
void swap(inplace_vector& other) noexcept(/* ... */);
```

Swaps the contents of two containers. Both must have identical `T` and `N`.

## Free Functions

### `ccc::swap`

```cpp
template<typename T, std::size_t N>
void swap(inplace_vector<T, N>& a, inplace_vector<T, N>& b) noexcept(noexcept(a.swap(b)));
```

Non-member `swap`, supports ADL.

### `ccc::erase`

```cpp
template<typename T, std::size_t N, typename U>
typename inplace_vector<T, N>::size_type erase(inplace_vector<T, N>& c, const U& value);
```

Removes all elements equal to `value`. Returns the number of removed elements.

### `ccc::erase_if`

```cpp
template<typename T, std::size_t N, typename Pred>
typename inplace_vector<T, N>::size_type erase_if(inplace_vector<T, N>& c, Pred pred);
```

Removes all elements satisfying the predicate `pred`. Returns the number of removed elements.

## Comparison Operators

**C++20 and later**: provides `operator==` and `operator<=>` (three-way comparison), based on `std::lexicographical_compare_three_way`.

**Pre-C++20**: provides all six comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`), based on `std::lexicographical_compare`.

## Notes

- `inplace_vector` has a compile-time fixed capacity that cannot grow dynamically. Most mutating operations throw `std::bad_alloc` on overflow; `resize` throws `std::out_of_range`.
- The `try_*` family provides exception-free error handling, suitable for environments where exceptions are disabled.
- `unchecked_*` interfaces skip capacity checks; the caller must guarantee the container is not full, otherwise behavior is undefined.
- `inplace_vector` iterators are raw pointers and may be invalidated by insert/erase operations, consistent with `std::vector` semantics.
- The underlying storage uses `alignas(T) unsigned char`, so element type `T` must have an accessible, non-throwing destructor.

See `test/inplace_vector_test/inplace_vector_test.cc` for complete usage examples.

