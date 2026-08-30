# ccc.hash

English | [简体中文](../zh/hash.md)

---

`ccc.hash` provides a compile-time-capable hash facility: the `ccc::hash` function object, modeled on the `std::hash` policy, together with `ccc::hash_combine` for building hashes of composite values. It covers basic types (arithmetic types, enumerations, pointers) as well as common standard library types (strings, pairs, tuples, optionals, variants, smart pointers, vectors, bitsets, error codes, and more).

## Features

- Compatible with C++11 and later; all hash computations are `constexpr` in C++20 and later, usable in compile-time contexts (e.g. `static_assert`, `constexpr` variables).
- Follows the `std::hash` policy: unsupported types are excluded from overload resolution (the primary template is disabled), and specializations are `noexcept` whenever their components are.
- Strings are hashed with MurmurHash3 (x86 32-bit variant), a fast general-purpose non-cryptographic hash.
- `hash_combine` adopts the classic Boost scheme (`seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2)`), with both binary and variadic forms.
- Composite specializations (pair/tuple/optional/variant/vector) are only enabled when all element types are hashable.
- Both `#include` and C++20 module (`import ccc.hash`) usage are supported.

## Quick Usage

Include the header:

```cpp
#include "ccc/hash.hh"
```

When using C++20 modules:

```cpp
import ccc.hash;
```

## Core Components

### `ccc::hash<T>`

A stateless function object whose `operator()` returns the hash of a value as `std::size_t`.

Enabled specializations:

- Basic types: all cv-unqualified arithmetic types, enumerations (hashed as their underlying type), object pointers, and `std::nullptr_t` (hashes to `0`).
- Strings: `std::basic_string`, and `std::basic_string_view` since C++17 — hashed byte-wise with MurmurHash3, independent of allocator/traits.
- Pairs and tuples: `std::pair<T1, T2>` and `std::tuple<Ts...>`, enabled when every element type is hashable; the empty tuple hashes to `0`.
- Optionals and variants (C++17): `std::optional<T>` (empty state uses a fixed seed distinct from any engaged value), `std::variant<Ts...>` (mixes the alternative index, so the same value in different alternatives hashes differently), and `std::monostate`.
- Smart pointers: `std::unique_ptr<T, D>` and `std::shared_ptr<T>` hash the stored pointer itself, never the pointee (matching the standard library policy); a null pointer hashes to `0`.
- Vectors: `std::vector<T>` (elements combined in order, with the length mixed into the result), plus a dedicated `std::vector<bool>` specialization that packs bits into words.
- Error types: `std::error_code` and `std::error_condition`, combining the integer value with the category object's address.
- `std::bitset<N>`: bits are packed into words and combined, so bits beyond the first 64 also participate in the hash.

```cpp
ccc::hash<int>()(42);                          // hash of an integer
ccc::hash<std::string>()("hello");             // MurmurHash3 over the bytes
ccc::hash<std::pair<int, int>>()( {1, 2});     // composite hash
ccc::hash<std::vector<std::string>>()({"a", "b"});

struct Config {};
// ccc::hash<Config> is disabled: calling it is a compile error
```

### `ccc::hash_combine`

Mixes the hash of one or more values into a seed and returns the new seed, following the classic Boost scheme. Use it to build hash support for your own composite types.

```cpp
template<typename T>
inline std::size_t hash_combine(std::size_t seed, const T& value) noexcept(/* hash of T is noexcept */);

template<typename T, typename... Rest>
inline std::size_t hash_combine(std::size_t seed, const T& value, const Rest&... rest)
    noexcept(/* hash of all values is noexcept */);
```

```cpp
struct Point {
    int x;
    int y;
};

struct HashPoint {
    std::size_t operator()(const Point& p) const noexcept
    {
        return ccc::hash_combine(0U, p.x, p.y);
    }
};

std::unordered_map<Point, std::string, HashPoint> map;
```

## Compile-Time Usage (C++20)

In C++20 and later, hash computations are `constexpr` and can run fully at compile time:

```cpp
static_assert(ccc::hash<int>()(42) == ccc::hash<int>()(42));

constexpr std::size_t seed = ccc::hash_combine(0U, 1, 2, 3);
constexpr std::size_t hash_of_string = ccc::hash<std::string_view>()("hello");
```

Note that whether a given computation is a constant expression also depends on the hashed type itself: `std::vector` elements and `std::variant` accessors only became `constexpr` in C++20, and `std::unique_ptr` in C++23.

## Characteristics and Limitations

- **Not a cryptographic hash.** The underlying algorithm is MurmurHash3 (a non-cryptographic, general-purpose hash). It is **not** collision-resistant against adversarial input and must **not** be used in security or cryptography contexts — signatures, password storage, message authentication, key derivation, etc. Use a dedicated primitive such as SHA-256 or BLAKE3 for those.
- **Hash values are implementation-defined.** They are stable within one program and across identical runs, but may differ across `ccc` versions, compilers, or `std::size_t` widths. Never persist, serialize, or compare hash values across programs.
- Like `std::hash`, no guarantee is made that distinct values hash distinctly; collisions are acceptable and expected for hash-table use.
- Composite specializations return different hashes for differently-ordered or differently-sized inputs (order and length participate in the result), but no minimum-distance guarantee exists between distinct inputs.

## Complete Examples

For more usage examples, see the test files:
- `test/hash_test/basic_types.cc`, `test/hash_test/string.cc`
- `test/hash_test/combine.cc`, `test/hash_test/tuple.cc`
- `test/hash_test/optional_variant.cc`, `test/hash_test/smart_ptr.cc`
- `test/hash_test/vector.cc`, `test/hash_test/bitset.cc`, `test/hash_test/system_error.cc`
