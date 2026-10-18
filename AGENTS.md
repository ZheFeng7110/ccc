# AGENTS.md

## 头文件 + 模块双支持

每个库组件必须同时支持 `#include` 和 `import` 两种引入方式。具体模式参见 skill 文件 `.agents/skills/cpp-lib-headers-modules-both-supports/SKILL.md`。

## 命名空间与模块导出

- 面向用户的功能定义在 `ccc` 命名空间内，通过在 `.ccm` 文件中使用 `export namespace ccc { using ::ccc::xxx; }` 导出。
- 内部实现细节定义在 `ccc::detail` 命名空间内，**不**通过模块导出（不在 `.ccm` 文件的 `export` 块内）。

## mcpp 的测试

使用 mcpp 进行测试时，**不要** `mcpp test`，而是应该：

```shell
mcpp build -p test
mcpp run -p test
```

## 代码风格

- 根目录 `.clang-format` 使用 Google 风格并做了定制（4 空格缩进、120 列宽限制、禁用参数装箱等）。
- 头文件包含保护模式：`#pragma once` + `#ifndef`/`#define`/`#endif`。
- 分支与循环语句（`if`、`else`、`switch`、`for`、`while`、`do`-`while` 等）即使内部只有一行也必须加上大括号。
- 类的 `private` / `protected` 成员（成员变量、成员函数、嵌套类型等）的命名均以下划线结尾（例如 `int value_;`、`void helper_();`），`public` 成员不使用下划线结尾。

## Git 提交规范

由智能体产生的 git 提交的最后一行要标注 AI 辅助信息，格式为
`Generated-by: AGENT_NAME:MODEL_VERSION[, AGENT_NAME2:MODEL_VERSION2, ...]`:
  - AGENT_NAME：你使用的 AI 智能体的名称（例如 Claude, Copilot, Codex 等）。
  - MODEL_VERSION：具体调用的模型版本（例如 claude-opus-5-5, gpt-6-astra, deepseek-flash 等）。
  - 若包含多个 AGENT_NAME:MODEL_VERSION，则用逗号分隔。
  - 示例：
    - `Generated-by: Codex:gpt-6.1-sol`
    - `Generated-by: OpenCode:deepseek-flash`
    - `Generated-by: Claude:claude-opus-5-5`
    - `Generated-by: Kimi Code:"kimi-k3", OpenCode:glm-5.3`
