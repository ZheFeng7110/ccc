# tassert 测试说明

本目录测试 `ccc::tassert` 的头文件接口、模块接口、配置传播、失败处理、格式化后端以及安装后的使用方式。每个子目录都是一个独立的小工程，由 [projects.json](projects.json) 描述，源文件、CMake 工程和 mcpp 清单放在同一目录中。

## 如何运行

CMake 场景由统一驱动运行：

```sh
python3 test/tassert/run_projects.py --tool cmake --fail-fast
python3 test/tassert/run_projects.py --tool cmake \
    --generator "Visual Studio 18 2026" --fail-fast
```

mcpp 场景必须分开执行构建和运行：

```sh
python3 test/tassert/run_projects.py --tool mcpp --fail-fast
```

驱动不会把所有场景合并到一个构建树。每个 CMake 变体都有独立的构建目录；每个 mcpp 变体都会复制库源码和测试工程，并拥有独立的模块缓存。这样可以检查配置头、模块 BMI 和编译宏是否真的随变体重新生成，而不会被其他场景的构建结果污染。

mcpp 驱动会为每个场景生成临时清单，并保留场景原有的依赖声明。fmt 依赖使用 `fmtlib.fmt = ">=11.2.0"`，通过 `--cache local` 使用本地缓存；fmt 后端还会加入 `FMT_ATTACH_TO_GLOBAL_MODULE=1`，使 fmt 的头文件接口能和库模块一起使用。驱动依次执行 `mcpp build -p test`、`mcpp run -p test`，并直接运行生成的二进制，再比较两种运行方式的退出码和输出。

每个成功场景都会先构建并运行 `exit_failure` 探针，读取当前平台的 `EXIT_FAILURE` 数值。测试中的 “failure” 预期因此适用于 Windows、Linux 和 macOS，而不依赖固定的数字。负例只要求编译失败，并从编译器错误诊断中匹配预期关键词。运行输出通过正则表达式检查标准输出、标准错误和必须缺失的文本。

## 公共测试基础设施

- [api.hh](api.hh) 定义两个公共 tag：启用的 `active` 和由测试配置禁用的 `disabled`。它根据宏选择头文件、`ccc.tassert` 模块或聚合模块。
- [config.hh](config.hh) 提供内建 `std::string_view` 数组，包含重复 tag、空 tag 和带内嵌空字符的字符串，用来验证精确匹配。
- [format_support.hh](format_support.hh) 为 `std::format` 和 fmt 各提供一个 `Item` formatter。formatter 统计调用次数，并可主动抛出异常。
- [project.cmake](project.cmake) 统一设置 C++ 标准、后端、配置头、模块模式、警告选项和线程库；负例直接使用头文件，避免根 CMake 在错误配置阶段提前终止。
- [fmt.cmake](fmt.cmake) 通过本目录的 [get_cpm.cmake](get_cpm.cmake) 集成 fmt，仅在场景选择 fmt 后端时引入；[CMakeLists.txt](CMakeLists.txt) 提供独立依赖工程，供安装消费者场景构建和安装 fmt。
- [run_projects.py](run_projects.py) 负责变体展开、独立构建、运行检查、直接二进制检查、编译错误匹配和模块配置重建检查。
- CMake 矩阵共有 76 个场景：56 个运行场景和 20 个编译负例。mcpp 矩阵共有 64 个场景，因为安装消费者只由 CMake 测试。

## 运行场景

下表中的“默认”指项目的基础配置；“模块”表示直接导入 `ccc.tassert`，“聚合”表示导入 `ccc`，“混合”表示同一测试中同时使用头文件和模块。

| 项目 | 变体和引入方式 | 测定内容 |
| --- | --- | --- |
| `basic` | 默认、`NDEBUG`、C++20 自动后端、C++23 自动后端；头文件 | tag 名称和启用状态可在编译期使用；结果对象不可复制、不可移动且析构不抛异常；成功断言、惰性谓词和 `if_enabled` 正常工作；失败断言终止进程；`NDEBUG` 不改变行为；自动后端在 C++20 为 0、可用 C++23 `std::format` 时为 1。 |
| `ignore_tags` | 默认配置、`std::array` 配置、显式空配置；头文件 | 配置头被正确包含；重复项无额外影响；按内容和长度精确匹配；大小写、前缀、不同地址的同文本、空字符串和内嵌空字符均按设计处理；空配置重新启用所有 tag。 |
| `lazy_condition` | 默认、C++23 标准格式化；头文件 | 启用 tag 的谓词只调用一次并只转换一次；禁用 tag 不调用谓词；普通 bool 条件仍遵守函数实参求值；捕获初始化仍会发生；成功或禁用时 message 参数本身仍会求值。 |
| `constexpr_branch` | 默认、C++23 标准格式化；头文件 | `if constexpr` 和 `if_enabled` 在禁用时裁剪整个回调；禁用分支不要求回调可调用；启用分支可调用仅支持右值调用的对象；格式化调用也会被裁剪。 |
| `default_failure` | 默认、C++23 标准格式化；头文件 | 默认处理器输出 tag、文件、行号、列号和函数；显式 `source_location` 包装器保留最外层调用位置；裸调用、包装器调用和带消息调用都在失败后退出；析构函数和后续语句不会继续执行，而 `atexit` 仍运行。 |
| `handler` | 默认；头文件 | `get_handler`、`set_handler`、返回旧处理器、`nullptr` 恢复默认处理器的语义；成功和禁用断言不调用处理器；自定义处理器得到正确 tag、消息和调用位置；谓词失败也保留调用位置；处理器返回后库仍以失败状态退出。 |
| `handler_concurrency` | 默认；头文件 | 8 个线程共执行大量 handler 交换和读取，确认原子操作只观察有效函数指针；并发失败分发保持有效；处理器递归触发断言时只输出一次处理器标记并走递归保护路径。 |
| `format_std` | C++23；头文件、模块、聚合 | 标准格式化后端的编译期格式检查；成功断言和禁用断言不调用自定义 formatter；无参数消息和带整数、临时字符串、自定义类型的消息正确传递；失败处理器收到格式化后的 `zero` 和 `0007 custom temporary`。 |
| `format_fmt` | C++20、C++23；头文件、模块、聚合 | fmt 后端在两个语言标准下工作；fmt 的格式检查、消息生成、自定义 formatter、临时参数和模块导入路径与标准后端具有相同语义。 |
| `formatter_failure` | 标准后端、fmt 后端；头文件 | formatter 抛异常以及格式化结果分配失败时都使用稳定的 “message formatting failed” 回退消息；禁用或成功断言不触发 formatter；失败处理仍同步完成。 |
| `exceptions` | 默认、C++23 标准格式化；头文件 | 启用谓词抛出的异常原样传播；禁用谓词不被调用；message 参数在进入成员函数前求值，因此参数异常优先传播；格式化失败和正常断言失败不会伪装成谓词异常。 |
| `multiple_translation_units` | 默认；头文件 | 多个翻译单元看到同一个 inline handler 状态和同一份 tag 配置；一个翻译单元安装的处理器可被另一个翻译单元读取和调用；跨文件失败位置和退出行为正确。 |
| `modules` | 默认、C++23 标准格式化、fmt、显式空配置、配置重建；模块、聚合、混合 | `ccc.tassert`、聚合模块以及头文件与模块混用均可工作；标准和 fmt 后端能导出并使用；配置头能传入模块；空配置启用原本禁用的 tag；`rebuild` 在同一构建目录替换配置内容并重新构建，确认不会复用旧 BMI。 |
| `install_consumer` | 默认、带配置、C++23 标准格式化、fmt；头文件、模块、聚合 | 先安装 ccc，再由独立消费者通过 `find_package(ccc)` 使用；安装后的头文件和模块可用；用户配置目录由消费者加入 imported target；安装包的 fmt 依赖可被发现；安装后的格式化消息和配置 tag 与构建树内行为一致。Visual Studio 生成器目前不支持消费包含 BMI-only file set 的已安装目标，因此该平台只运行 installed-header 组合，库内模块和其他平台的安装模块仍单独覆盖。 |

## 编译负例

这些项目的预期结果是“编译失败”。驱动不会运行生成的程序，而是检查失败诊断是否包含对应语义。它们验证错误必须尽早、明确地暴露给用户，不能静默退回默认配置或在运行时才失败。

| 项目 | 故意构造的错误 | 测定内容 |
| --- | --- | --- |
| `compile_errors/low_standard` | 使用 C++20 以下标准 | tassert 的最低语言标准检查。 |
| `compile_errors/missing_config` | 指向不存在的配置头 | 配置头路径错误不会被忽略。 |
| `compile_errors/missing_variable` | 配置命名空间没有 `ignore_tags` | 缺少配置变量会编译失败。 |
| `compile_errors/wrong_type` | `ignore_tags` 使用不支持的类型 | 只接受规定的 string_view 数组或 `std::array`。 |
| `compile_errors/non_constant` | 配置不是常量可求值对象 | 配置内容必须能在编译期读取。 |
| `compile_errors/bad_backend` | 后端宏设置为 0、1、2 之外的值 | 后端选择值严格校验。 |
| `compile_errors/std_cpp20` | C++20 强制选择标准格式化后端 | 标准后端要求 C++23 和可用的 `std::format`。 |
| `compile_errors/no_message_backend` | 后端为 0 却调用 `.message` | 未选择格式化后端时，message 接口给出明确错误。 |
| `compile_errors/copy` | 复制断言结果对象 | 结果对象不能被复制，避免重复报告或悬空状态。 |
| `compile_errors/move` | 移动断言结果对象 | 结果对象不能被移动，保持临时对象生命周期语义。 |
| `compile_errors/predicate` | 谓词返回不能转换为 bool 的类型 | 谓词重载约束拒绝无效返回类型。 |
| `compile_errors/format_type` | 标准格式说明符与参数类型不匹配 | 标准格式字符串在编译期检查。 |
| `compile_errors/format_index` | 使用越界的位置参数索引 | 格式参数索引错误在编译期诊断。 |
| `compile_errors/unformattable` | 标准后端传入不可格式化类型 | 未提供 formatter 的类型不能通过编译。 |
| `compile_errors/runtime_format` | 使用运行时字符串作为标准格式串 | 标准后端要求编译期格式字符串。 |
| `compile_errors/lvalue_message` | 对左值结果调用 `.message` | message 只允许在断言结果临时对象上调用。 |
| `compile_errors/missing_fmt` | 强制选择 fmt 后端但不提供 fmt 头文件 | fmt 后端缺少依赖时给出头文件错误。 |
| `compile_errors/disabled_format_type` | 禁用 tag 仍使用非法标准格式参数 | 编译期格式约束仍存在，禁用 tag 不允许写入无效 API。 |
| `compile_errors/fmt_format_type` | 禁用 tag 使用非法 fmt 格式参数 | fmt 后端的编译期检查与禁用状态无关。 |
| `compile_errors/fmt_unformattable` | fmt 后端传入不可格式化类型 | fmt formatter 约束会拒绝不可格式化类型。 |

## 结果判定与扩展测试

每个运行场景的 `runs` 数组可以指定命令行参数、期望退出码、标准输出、标准错误和禁止出现的文本；同一二进制会同时通过工具运行和直接运行两条路径验证。失败处理器通常通过退出码传递验证结果，默认处理器则通过诊断内容和 `EXIT_FAILURE` 验证。

新增场景时应：

1. 新建独立子目录，至少提供 `CMakeLists.txt`、`main.cc` 和 mcpp 清单（仅 CMake 专用场景可省略 mcpp）。
2. 在 `projects.json` 添加标准、后端、配置、引入方式、变体和运行期断言。
3. 对失败场景填写 `compile_error`，并确保关键词来自实际编译器错误诊断，而不是源文件路径。
4. 如果测试模块配置重建，使用 `rebuild_config`，让驱动在同一构建树中替换同名配置文件。
5. 同时检查工具运行结果和生成的直接二进制，避免只验证了 mcpp 的包装命令。
6. 使用 `--fail-fast` 先定位第一个失败场景，再用完整矩阵确认没有其他回归。
