# ccc.pipe_operator_helper

[English](../en/pipe_operator_helper.md) | 简体中文

---

`pipe_operator_helper` 是一个小巧的 C++ 工具库，它支持管道风格的调用语法和管道风格的类型转换。

传统写法：

- `f1(x)`
- `f2(x, y)`
- `static_cast<int>(a)`

使用管道风格可以写成：

- `x | f1()`
- `x | f2(y)`
- `a > static_cast_to<int>()`

这使得某些调用链和转换可以更自然地从左到右阅读。

## 特性

- 管道风格函数调用辅助工具（支持函数对象和模板）。
- 管道风格类型转换：`static_cast_to`、`dynamic_cast_to`、`const_cast_to`、`reinterpret_cast_to`、`bit_cast_to`，
  以及 `move_to` 和 `forward_to` 辅助工具。
- 设计为可作为单一头文件使用（也可在配置模块构建时作为模块使用）。
- 支持引用、指针、值类型以及多种转换场景（详见测试）。

## 快速使用

- 引入头文件：

  `#include "ccc/pipe_operator_helper.hpp"`

  或在使用 C++ 模块时（如果已配置）：

  `import ccc.pipe_operator_helper;`

- 函数管道：
    - 定义一个派生自 `pipe_operator_helper::pipe_operator<Derived, Arity>` 的可调用类型（`Arity` 为函数参数个数），
      将实际调用实现为 `operator()`，并通过 `using` 声明重新暴露基类的部分应用 `operator()`。然后创建一个
      `inline constexpr` 实例。

    ```C++
    struct add_t : pipe_operator_helper::pipe_operator<add_t, 2> {
        using pipe_operator_helper::pipe_operator<add_t, 2>::operator();

        constexpr int operator()(const int a, const int b) const noexcept { return a + b; }
    };
    inline constexpr add_t add{};
    ```

    - 然后你可以同时使用 `add(x, y)` 和 `x | add(y)`。
    - 左侧参数的类型在管道调用处推导，因此模板不再需要显式指定模板参数
      （例如用 `x | add(y)` 代替 `x | add<T>(y)`）。
    - 相同的模式也适用于引用、模板、`constexpr` 和 `consteval` 辅助工具（参见
      `test/pipe_operator_helper_test/pipe_operators_tests/1arg.cc` 和
      `test/pipe_operator_helper_test/pipe_operators_tests/2args.cc`）。

- 类型转换操作符：
    - 用 `v > pipe_operator_helper::static_cast_to<T>()` 或 `v > pipe_operator_helper::cast_to<T>()` 替代 `static_cast<T>(v)`
      （`cast_to` 与 `static_cast_to` 效果相同）。
    - 同样地，还有 `dynamic_cast_to<T>()`、`const_cast_to<T>()`、`reinterpret_cast_to<T>()` 和
      `bit_cast_to<T>()` 工具。
    - `std::move` 和 `std::forward` 也有类型转换操作符：
        - `v > pipe_operator_helper::move_to()` 与 `std::move(v)` 效果相同。
        - `v > pipe_operator_helper::forward_to<T>()` 与 `std::forward<T>(v)` 效果相同。

## 注意事项与提示

- 该库同时支持纯头文件使用和模块使用。如果你的工具链支持 C++20 模块，并且项目配置为构建模块
  （`-DCCC_USE_CPP_MODULES=ON`），你可以 `import ccc.pipe_operator_helper` 而不是引入头文件。
- 若管道版本的可调用对象与已有的函数冲突了，可以统一放入 `pp` 命名空间内，参考
  `test/pipe_operator_helper_test/pipe_operators_tests/wrap_already_exists.cc`。
- 最新的使用模式请参阅：
  - `test/pipe_operator_helper_test/pipe_operators_tests/1arg.cc`
  - `test/pipe_operator_helper_test/pipe_operators_tests/2args.cc`
  - `test/pipe_operator_helper_test/pipe_operators_tests/wrap_already_exists.cc`
  - `test/pipe_operator_helper_test/type_cast_test.cc`

## 特别感谢

* [原始仓库](https://github.com/ZheFeng7110/cpp_pipe_operator_helper) 的 `v1.x.x` 版本的实现受 [wzxzhuxi/cpp-functional-programming](https://github.com/wzxzhuxi/cpp-functional-programming/tree/main/06-composition) 启发。
