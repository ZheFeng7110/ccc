# ccc.hash

[English](../en/hash.md) | 简体中文

---

`ccc.hash` 提供一套支持编译期计算的哈希设施：仿照 `std::hash` 策略设计的哈希函数对象 `ccc::hash`，以及用于组合自定义类型哈希的 `ccc::hash_combine`。覆盖基础类型（算术类型、枚举、指针）以及常用标准库类型（字符串、pair、tuple、optional、variant、智能指针、vector、bitset 等）。

## 特性

- 兼容 C++11 及以上版本；C++20 起所有哈希计算均为 `constexpr`，可在编译期使用（如 `static_assert`、`constexpr` 变量）。
- 遵循 `std::hash` 的策略：不支持的类型不参与重载决议（主模板处于禁用状态），各特化在其组成部分不抛异常时均为 `noexcept`。
- 字符串采用 MurmurHash3（x86 32 位变体）计算，一种快速、通用的**非加密**哈希算法。
- `hash_combine` 采用经典 Boost 方案（`seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2)`），提供二元与变参两种形式。
- 组合类型的特化（pair/tuple/optional/variant/vector）仅在所有元素类型均可哈希时启用。
- 同时支持 `#include` 与 C++20 模块（`import ccc.hash`）两种引入方式。

## 快速使用

引入头文件：

```cpp
#include "ccc/hash.hh"
```

使用 C++20 模块时：

```cpp
import ccc.hash;
```

## 核心组件

### `ccc::hash<T>`

无状态函数对象，其 `operator()` 返回 `std::size_t` 类型的哈希值。

已启用的特化：

- 基础类型：所有 cv 不限定的算术类型、枚举（按底层整数类型哈希）、对象指针，以及 `std::nullptr_t`（恒为 `0`）。
- 字符串：`std::basic_string`，以及 C++17 起的 `std::basic_string_view`——对字节序列运行 MurmurHash3，与分配器/Traits 无关。
- pair 与 tuple：`std::pair<T1, T2>` 与 `std::tuple<Ts...>`，要求所有元素类型均可哈希；空 tuple 的哈希值为 `0`。
- optional 与 variant（C++17）：`std::optional<T>`（空态使用固定种子，与任何有效值区分开）、`std::variant<Ts...>`（混入备选项索引，同一值在不同备选项中哈希不同），以及 `std::monostate`。
- 智能指针：`std::unique_ptr<T, D>` 与 `std::shared_ptr<T>` 对存储的指针本身哈希，绝不解引用所指对象（与标准库策略一致）；空指针哈希为 `0`。
- vector：`std::vector<T>`（按顺序逐元素合并，并把长度混入结果），另有专门的 `std::vector<bool>` 特化，将位打包成字后计算。
- `std::bitset<N>`：位打包成字后合并，第 64 位之后的位同样参与哈希。

```cpp
ccc::hash<int>()(42);                          // 整数哈希
ccc::hash<std::string>()("hello");             // 对字节运行 MurmurHash3
ccc::hash<std::pair<int, int>>()( {1, 2});     // 组合哈希
ccc::hash<std::vector<std::string>>()({"a", "b"});

struct Config {};
// ccc::hash<Config> 处于禁用状态：调用将导致编译错误
```

### `ccc::hash_combine`

将一个或多个值的哈希混入种子并返回新种子，采用经典 Boost 方案。用于为自定义组合类型构建哈希支持。

```cpp
template<typename T>
inline std::size_t hash_combine(std::size_t seed, const T& value) noexcept(/* T 的哈希为 noexcept */);

template<typename T, typename... Rest>
inline std::size_t hash_combine(std::size_t seed, const T& value, const Rest&... rest)
    noexcept(/* 所有值的哈希均为 noexcept */);
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

## 编译期使用（C++20）

C++20 及以上版本中，哈希计算为 `constexpr`，可完全在编译期完成：

```cpp
static_assert(ccc::hash<int>()(42) == ccc::hash<int>()(42));

constexpr std::size_t seed = ccc::hash_combine(0U, 1, 2, 3);
constexpr std::size_t hash_of_string = ccc::hash<std::string_view>()("hello");
```

注意：某次计算能否成为常量表达式，还取决于被哈希类型本身——`std::vector` 的元素访问与 `std::variant` 的 `std::get` 在 C++20 才成为 `constexpr`，`std::unique_ptr` 则要到 C++23。

## 特性与限制

- **不是加密哈希。** 底层算法为 MurmurHash3（非加密的通用哈希）。它**不具备**抵抗恶意构造输入的抗碰撞性，**请勿**用于安全或加密领域——签名、口令存储、消息认证、密钥派生等。此类需求请使用专用算法（如 SHA-256、BLAKE3）。
- **哈希值是实现定义的。** 同一程序内、相同输入下哈希值稳定，但可能随 `ccc` 版本、编译器或 `std::size_t` 位宽的不同而不同。切勿持久化、序列化或跨程序比较哈希值。
- 与 `std::hash` 一致，不保证不同值得到不同哈希；哈希表场景下碰撞是正常且预期的行为。
- 组合类型的特化对元素顺序与长度敏感（顺序和长度均参与计算），但不同输入之间不存在最小距离保证。

## 完整示例

更多用法请参阅测试文件：
- `test/hash_test/basic_types.cc`、`test/hash_test/string.cc`
- `test/hash_test/combine.cc`、`test/hash_test/tuple.cc`
- `test/hash_test/optional_variant.cc`、`test/hash_test/smart_ptr.cc`
- `test/hash_test/vector.cc`、`test/hash_test/bitset.cc`
