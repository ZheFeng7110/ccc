## 如何约束函数（Agent 执行规范）

### 目标

当函数或函数模板需要“按条件参与重载决议”时，必须按以下双分支规则实现：

- C++20 及以上：使用 `concept` / `requires`。
- C++20 之前：使用 SFINAE（`enable_if_t`）。

本仓库统一使用 `__cpp_concepts` 作为能力判断宏。

### 决策流程

Agent 在生成或修改受约束函数时，按以下顺序执行：

1. 识别是否存在“仅在条件成立时可用”的需求。
2. 保留同一函数签名语义，在 `#ifdef __cpp_concepts` / `#else` 下提供两套等价约束。
3. C++20 分支使用 `requires(...)`；pre-C++20 分支使用 `enable_if_t<...>`。
4. 两个分支的约束条件表达式必须逻辑一致。

### 规范写法

#### 1) 非模板函数（返回类型 SFINAE）

适用于：普通函数按布尔条件启用/禁用。

```C++
auto f1()
#ifdef __cpp_concepts
    -> int
    requires(cond1)
#else
    -> enable_if_t<cond1, int>
#endif
{
    // function body
}
```

#### 2) 函数模板（模板参数 SFINAE）

适用于：模板函数按类型特征启用/禁用。

```C++
template<typename T
#ifndef __cpp_concepts
    , typename = enable_if_t<!is_void_v<T>>
#endif
>
#ifdef __cpp_concepts
    requires(!is_void_v<T>)
#endif
int f2(int v)
{
    // function body
}
```

注意：在这种情况下，如果 `enable_if_t` 的第二个模板参数是 `void`，则需要显式写出以增加可读性。

#### 3) 表达式可用性检测（`void_t + decltype` / `requires`）

适用于：约束条件是“某个表达式是否有效”（detection idiom）。

```C++
template<typename T
#ifndef __cpp_concepts
    , typename = void_t<decltype(std::declval<T>().value() == std::declval<T>().value())>
#endif
>
#ifdef __cpp_concepts
    requires requires(T t, T u) { t.value() == u.value(); }
#endif
bool operator==(const T& lhs, const T& rhs)
{
    return lhs.value() == rhs.value();
}
```

#### 4) 复合约束（可用性 + 可转换性 + 业务谓词）

适用于：除了表达式有效，还需要额外类型性质或业务条件。

```C++
template<typename T
#ifndef __cpp_concepts
    , typename = void_t<decltype(std::declval<T>().value() == std::declval<T>().value())>
    , typename = enable_if_t<
        is_valid_v<T> &&
        is_convertible_v<decltype(std::declval<T>().value() == std::declval<T>().value()), bool>
    >
#endif
>
#ifdef __cpp_concepts
    requires(
        is_valid_v<T> &&
        requires(T t, T u) { { t.value() == u.value() } -> std::convertible_to<bool>; }
    )
#endif
bool operator==(const T& lhs, const T& rhs)
{
    return lhs.value() == rhs.value();
}
```

### 约束一致性要求

- `requires(expr)` 与 `enable_if_t<expr, ...>` 中的 `expr` 必须一致。
- 不要在两个分支写不同语义的条件。
- 能放在模板参数处的 pre-C++20 约束，优先放模板参数处（避免污染函数返回类型语义）。
- 对“表达式可用性”约束：`requires` 中的检测项要与 `void_t<decltype(...)>` 一一对应。
- 对“可转换性”约束：`std::convertible_to<bool>` 与 `is_convertible_v<..., bool>` 要语义对齐。

### Agent 自检清单

- [ ] 是否使用 `__cpp_concepts` 进行分支判断。
- [ ] C++20 分支是否使用 `requires` 或等价 `concept` 约束。
- [ ] pre-C++20 分支是否使用 SFINAE（`enable_if_t`）。
- [ ] 两个分支条件是否完全等价。
- [ ] 若涉及表达式检测，是否使用 `requires` vs `void_t + decltype` 的对应写法。
- [ ] 是否避免重复书写检测表达式导致语义漂移（必要时提取别名以复用）。
- [ ] 是否仅改动约束机制，不改变函数行为。

