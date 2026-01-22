## 简介

Linux 内核的静态分析和代码质量检查工具，用于在编译前或编译过程中发现代码中的
潜在问题。

```bash
$ make help
checkstack      - Generate a list of stack hogs and consider all functions
                  with a stack size larger than MINSTACKSIZE (default: 100)
versioncheck    - Sanity check on version.h usage
includecheck    - Check for duplicate included header files
headerdep       - Detect inclusion cycles in headers
coccicheck      - Check with Coccinelle
clang-analyzer  - Check with clang static analyzer
clang-tidy      - Check with clang-tidy
```

## checkstack

分析内核中每个函数的栈帧大小，找出可能消耗过多栈空间的函数。这对于内核开发
至关重要，因为内核栈空间通常很小且固定。

## versioncheck

检查内核源码中 `#include <linux/version.h>` 的使用是否正确。该头文件包含了
内核版本信息，但通常不应被直接包含，因为 `<linux/module.h>` 等头文件已自动包含了它。

## includecheck

检查源码中是否存在重复包含同一个头文件的情况。虽然通常无害，但重复包含
可能降低编译速度，有时也暗示了代码组织问题。

## headerdep

检测头文件之间的循环依赖（即 A.h 包含 B.h，B.h 又包含 A.h）。循环依赖会导致
编译错误，是头文件设计需要避免的。

## coccicheck

`coccicheck` 是 Linux 内核源码树中的一个代码检查与转换工具，它基于
Coccinelle 工具，用于在 C 代码中搜索指定的模式并进行代码转换或检查。

### 主要功能

1. 模式匹配与转换

   - 使用语义补丁（Semantic Patch）语言描述代码模式。
   - 可自动将匹配的代码转换为更安全、更高效或更符合规范的版本。

2. 常见用途

   - 查找并修复错误的 API 使用（如资源泄漏、错误处理）。
   - 检查代码规范违规（如拼写错误、多余的空格）。
   - 协助API 迁移（如函数重命名、参数变更）。
   - 检测常见的错误模式（如空指针解引用、越界访问）。

### 解析参数

- 在内核源码根目录执行：

```bash
$ make coccicheck [MODE=patch|report|context]
```

- `report`：仅报告匹配项，不修改。
- `patch`：生成补丁文件，可手动应用。
- `context`：提供更详细的上下文信息。

### 安装

```bash
$ sudo apt install coccinelle   ## Ubuntu/Debian
$ sudo dnf install coccinelle   ## Fedora/RHEL
```

### 使用示例

```bash
$ make coccicheck MODE=report M=mm/
```

检查 mm 目录下的所有语义补丁

### 优势与局限

- 优势：

  - 高效准确：基于语义分析，避免正则表达式的误匹配。
  - 批量处理：可一次性修复整个代码库中的同类问题。
  - 社区驱动：内核维护者维护了大量常用补丁（位于 `scripts/coccinelle/`）。

- 局限：

  - 复杂模式可能需要编写自定义补丁。
  - 对宏展开和条件编译的处理有时受限。

### 应用场景

- 内核开发者：提交补丁前，用 `coccicheck` 检查常见错误。
- 维护者：批量更新 API 调用或规范代码风格。
- 代码审查：自动化检测潜在缺陷。

## clang-analyzer

运行 Clang 静态分析器。这是一个强大的、由 LLVM/Clang 项目提供的源码分析工具，
可以执行路径敏感的数据流分析，发现空指针解引用、内存泄漏、逻辑错误等深层 Bug。

## clang-tidy

运行 clang-tidy 工具。这是一个基于 Clang 的“代码检查器”，侧重于代码风格、
现代 C 语言特性使用（如 C11）、潜在的性能问题和可读性改进。
