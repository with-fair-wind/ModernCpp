# `.clang-tidy` 配置完整指南

> 本文档系统介绍 clang-tidy 静态检查规则、`.clang-tidy` 的所有顶层字段、常用 check 模块、CheckOptions 配置以及与 clangd / CI 的集成。
>
> 官方文档：https://clang.llvm.org/extra/clang-tidy/
> 全部 check 列表：https://clang.llvm.org/extra/clang-tidy/checks/list.html

---

## 目录

1. [clang-tidy 是什么](#1-clang-tidy-是什么)
2. [配置文件加载规则](#2-配置文件加载规则)
3. [`.clang-tidy` 顶层字段](#3-clang-tidy-顶层字段)
4. [Checks 表达式语法](#4-checks-表达式语法)
5. [Check 模块全览](#5-check-模块全览)
6. [关键 check 详解](#6-关键-check-详解)
7. [CheckOptions 细粒度参数](#7-checkoptions-细粒度参数)
8. [命名规范（readability-identifier-naming）](#8-命名规范readability-identifier-naming)
9. [按目录分层覆盖](#9-按目录分层覆盖)
10. [行内禁用与 NOLINT](#10-行内禁用与-nolint)
11. [与 `.clangd` 的关系](#11-与-clangd-的关系)
12. [命令行使用](#12-命令行使用)
13. [CI 集成](#13-ci-集成)
14. [常见问题](#14-常见问题)

---

## 1. clang-tidy 是什么

`clang-tidy` 是基于 Clang AST 的 C/C++ 静态分析与现代化重构工具。它不仅能发现潜在 bug，还能自动 `--fix` 替换代码（如 `NULL → nullptr`、传统 for → 范围 for、`typedef → using`）。

clangd LSP 在打开 C++ 文件时会**内嵌**调用 clang-tidy（行为受 clangd `--clang-tidy` 控制，默认开），把 tidy 警告作为 LSP 诊断显示在编辑器中。

---

## 2. 配置文件加载规则

- **就近优先**：clang-tidy 从被检查文件所在目录**向上递归**寻找最近的 `.clang-tidy`
- **分层继承**：默认子目录的 `.clang-tidy` 是「替换」父级。要继承，使用 `InheritParentConfig: true`
- **多份配置**：可以让 `tests/`、`third_party/` 各自有更宽松的配置
- **CLI 覆盖**：`clang-tidy --checks=...` 会与文件中的 Checks 合并

---

## 3. `.clang-tidy` 顶层字段

| 字段 | 类型 | 说明 |
|---|---|---|
| `Checks` | string | 启用/禁用规则的逗号分隔表达式 |
| `WarningsAsErrors` | string | 哪些警告升级为错误（同 Checks 语法） |
| `HeaderFilterRegex` | regex | 哪些头文件被分析（默认只分析当前 TU 的源） |
| `ExcludeHeaderFilterRegex` | regex | 头文件排除模式（clang-tidy 17+） |
| `FormatStyle` | string | `--fix` 后用什么风格格式化：`file` / `Google` / `LLVM` / `none` |
| `User` | string | NOLINT 注释的作者标记 |
| `CheckOptions` | list/dict | 各 check 的细粒度参数 |
| `SystemHeaders` | bool | 是否分析系统头（一般 false） |
| `InheritParentConfig` | bool | 是否继承父目录的 .clang-tidy |
| `UseColor` | bool | 终端着色 |
| `AnalyzeTemporaryDtors` | bool | 已废弃 |

**最小可用配置**：

```yaml
Checks: 'modernize-*,bugprone-*,readability-*,-readability-magic-numbers'
HeaderFilterRegex: '^.*/(src|include)/.*\.(h|hpp)$'
FormatStyle: file
```

---

## 4. Checks 表达式语法

| 语法 | 含义 |
|---|---|
| `*` | 启用所有 check |
| `foo-*` | 启用 foo 模块下所有 check |
| `foo-bar` | 启用具体 check |
| `-foo-bar` | 禁用具体 check（前缀 `-`） |
| `-foo-*` | 禁用整个模块 |
| `*,-foo-*` | 启用所有，再排除 foo |

**多行写法**（YAML block scalar）：

```yaml
Checks: >
  modernize-*,
  performance-*,
  -modernize-use-trailing-return-type,
  -performance-no-int-to-ptr
```

**重要**：表达式按顺序解析，**后面的覆盖前面的**。所以 `*,-modernize-*` 会启用所有再关掉 modernize；而 `-modernize-*,*` 等于全启用（被 `*` 覆盖）。

---

## 5. Check 模块全览

| 模块 | 数量 | 用途 |
|---|---|---|
| **abseil-** | ~20 | Abseil 库使用规范 |
| **altera-** | ~5 | OpenCL FPGA 相关 |
| **android-** | ~10 | Android NDK 编程 |
| **boost-** | ~5 | Boost 库使用 |
| **bugprone-** | ~80 | **易出错代码模式** |
| **cert-** | ~30 | CERT C/C++ 安全编码 |
| **clang-analyzer-** | ~120 | Clang 静态分析器 |
| **concurrency-** | ~3 | 并发问题 |
| **cppcoreguidelines-** | ~50 | **C++ Core Guidelines** |
| **darwin-** | ~2 | macOS 平台 |
| **fuchsia-** | ~10 | Fuchsia OS |
| **google-** | ~20 | **Google C++ Style Guide** |
| **hicpp-** | ~30 | High Integrity C++ |
| **linuxkernel-** | ~2 | Linux 内核风格 |
| **llvm-** | ~20 | LLVM 项目 |
| **llvmlibc-** | ~5 | LLVM libc |
| **misc-** | ~20 | 杂项 |
| **modernize-** | ~40 | **现代 C++ 现代化** |
| **mpi-** | ~3 | MPI 并行 |
| **objc-** | ~10 | Objective-C |
| **openmp-** | ~3 | OpenMP 并行 |
| **performance-** | ~30 | **性能优化** |
| **portability-** | ~5 | 可移植性 |
| **readability-** | ~50 | **可读性** |
| **zircon-** | ~1 | Zircon 内核 |

**通用项目最常用的 6 组**：

```yaml
Checks: >
  bugprone-*,
  cert-*,
  cppcoreguidelines-*,
  modernize-*,
  performance-*,
  readability-*
```

---

## 6. 关键 check 详解

### 6.1 modernize-（推荐 modern C++）

| Check | 作用 |
|---|---|
| `modernize-use-nullptr` | `NULL` / `0` → `nullptr` |
| `modernize-use-auto` | 长类型 → `auto` |
| `modernize-use-using` | `typedef` → `using` |
| `modernize-use-default-member-init` | 类成员就地初始化 |
| `modernize-use-equals-default` | `Foo() {}` → `Foo() = default;` |
| `modernize-use-equals-delete` | 私有未实现 → `= delete` |
| `modernize-loop-convert` | 传统 for → 范围 for |
| `modernize-make-unique` / `make-shared` | `new T` → `make_unique<T>` |
| `modernize-use-emplace` | `push_back(T(...))` → `emplace_back(...)` |
| `modernize-pass-by-value` | 大对象参数 → 按值 + std::move |
| `modernize-use-trailing-return-type` | `int f()` → `auto f() -> int`（**通常关闭**） |
| `modernize-use-nodiscard` | 给查询函数加 `[[nodiscard]]` |
| `modernize-concat-nested-namespaces` | C++17 合并嵌套 namespace |
| `modernize-use-designated-initializers` | C++20 指定初始化器 |

### 6.2 performance-

| Check | 作用 |
|---|---|
| `performance-unnecessary-copy-initialization` | 不必要的拷贝构造 |
| `performance-unnecessary-value-param` | 大对象按值传参 → const& |
| `performance-move-const-arg` | `std::move(const T)` 无效 move |
| `performance-for-range-copy` | for 循环中 `auto x` 拷贝 → `const auto&` |
| `performance-implicit-conversion-in-loop` | 隐式转换造成临时对象 |
| `performance-inefficient-string-concatenation` | `s = s + ...` → `s += ...` |
| `performance-noexcept-move-constructor` | move ctor 应 noexcept |
| `performance-trivially-destructible` | 可平凡析构的类应去掉 dtor |

### 6.3 bugprone-

| Check | 作用 |
|---|---|
| `bugprone-use-after-move` | move 后再次使用对象 |
| `bugprone-dangling-handle` | string_view/span 悬挂 |
| `bugprone-narrowing-conversions` | 隐式窄化（int → short） |
| `bugprone-undefined-memory-manipulation` | 对非 POD 类型 memcpy/memset |
| `bugprone-string-integer-assignment` | `string = int`（隐式转 char） |
| `bugprone-suspicious-semicolon` | 可疑的孤立分号 |
| `bugprone-branch-clone` | if/else 分支体相同 |
| `bugprone-easily-swappable-parameters` | 同类型相邻参数易被调反（**常关**） |
| `bugprone-exception-escape` | noexcept 函数中可能抛异常 |

### 6.4 cppcoreguidelines-

| Check | 作用 |
|---|---|
| `cppcoreguidelines-pro-type-cstyle-cast` | 禁 C 风格 cast |
| `cppcoreguidelines-pro-type-reinterpret-cast` | 禁 reinterpret_cast |
| `cppcoreguidelines-pro-bounds-pointer-arithmetic` | 禁指针算术（**底层代码常关**） |
| `cppcoreguidelines-special-member-functions` | 五法则（定义任一就要全定义） |
| `cppcoreguidelines-init-variables` | 局部变量必须初始化 |
| `cppcoreguidelines-avoid-magic-numbers` | 魔法数（**常关**） |
| `cppcoreguidelines-owning-memory` | 用 `gsl::owner` 标注所有权 |
| `cppcoreguidelines-rvalue-reference-param-not-moved` | && 参数没 move 也没 forward |

### 6.5 readability-

| Check | 作用 |
|---|---|
| `readability-identifier-naming` | **命名规范**（详见第 8 节） |
| `readability-function-size` | 函数行数/复杂度 |
| `readability-function-cognitive-complexity` | 认知复杂度 |
| `readability-magic-numbers` | 魔法数（**常关**） |
| `readability-implicit-bool-conversion` | 隐式 bool 转换 |
| `readability-named-parameter` | 函数声明必须有形参名 |
| `readability-redundant-*` | 各种冗余写法 |
| `readability-simplify-boolean-expr` | bool 表达式简化 |
| `readability-uppercase-literal-suffix` | `1.0f` 而非 `1.0F`（或反过来） |
| `readability-container-size-empty` | `v.size() == 0` → `v.empty()` |
| `readability-else-after-return` | return 后的 else 多余 |
| `readability-qualified-auto` | `auto*` 而非 `auto` 拿指针 |

### 6.6 cert-

CERT 安全编码部分常用：

| Check | 等价 |
|---|---|
| `cert-dcl21-cpp` | 后置自增运算符返回 const |
| `cert-dcl50-cpp` | 不要定义 C 风格变参函数 |
| `cert-err58-cpp` | 全局变量构造可能抛异常（**常关**） |
| `cert-err60-cpp` | 异常类型应可拷贝且 noexcept |
| `cert-flp30-c` | float 不要用作 for 循环计数 |
| `cert-msc51-cpp` | 不要用 `std::rand()`，用 `<random>` |

### 6.7 google-

| Check | 作用 |
|---|---|
| `google-readability-braces-around-statements` | 单语句必须加大括号 |
| `google-readability-namespace-comments` | namespace 闭合加注释 |
| `google-explicit-constructor` | 单参数构造必须 explicit |
| `google-build-using-namespace` | 头文件中禁止 `using namespace` |
| `google-runtime-int` | 用 int32_t / int64_t 而非 short/long |

### 6.8 concurrency-

| Check | 作用 |
|---|---|
| `concurrency-mt-unsafe` | 调用 MT-unsafe 的 C 函数（如 `localtime`） |

---

## 7. CheckOptions 细粒度参数

`CheckOptions` 是 list 形式（旧版）或 dict 形式（新版），key 形如 `<check>.<option>`：

```yaml
CheckOptions:
  # list 形式（兼容性最好）
  - key: readability-function-size.LineThreshold
    value: '120'
  - key: modernize-loop-convert.MinConfidence
    value: reasonable

  # dict 形式（clang-tidy 15+）
  readability-function-size.StatementThreshold: '60'
```

**常用参数速查**：

| Check.Option | 默认 | 说明 |
|---|---|---|
| `readability-function-size.LineThreshold` | none | 函数行数上限 |
| `readability-function-size.StatementThreshold` | 800 | 语句数上限 |
| `readability-function-size.ParameterThreshold` | none | 参数数上限 |
| `readability-function-size.NestingThreshold` | none | 嵌套深度上限 |
| `readability-function-cognitive-complexity.Threshold` | 25 | 认知复杂度上限 |
| `modernize-loop-convert.MinConfidence` | reasonable | safe / reasonable / risky |
| `modernize-use-auto.MinTypeNameLength` | 5 | 类型名 ≥ 多长才替换 |
| `modernize-use-auto.RemoveStars` | false | 替换时是否吃掉 `*` |
| `modernize-use-nullptr.NullMacros` | NULL | 等价 NULL 的宏列表 |
| `performance-unnecessary-value-param.AllowedTypes` | '' | 允许按值传的类型正则 |
| `performance-for-range-copy.WarnOnAllAutoCopies` | false | 即使 trivial 类型也警告 |
| `cppcoreguidelines-special-member-functions.AllowSoleDefaultDtor` | false | 允许只 default 析构函数 |
| `cppcoreguidelines-special-member-functions.AllowMissingMoveFunctions` | false | 允许缺 move 操作 |
| `bugprone-argument-comment.StrictMode` | 0 | 是否强制 `/*x=*/` 注释名匹配 |
| `bugprone-easily-swappable-parameters.MinimumLength` | 2 | 最小相邻同类型参数数 |
| `misc-include-cleaner.IgnoreHeaders` | '' | 排除的头正则 |

---

## 8. 命名规范（readability-identifier-naming）

可设的实体（**约 50 种**）：

```
ClassCase / StructCase / UnionCase / EnumCase / EnumConstantCase
NamespaceCase / TypeAliasCase / TypedefCase
FunctionCase / MemberFunctionCase / ClassMethodCase
VariableCase / ParameterCase / LocalVariableCase
GlobalVariableCase / StaticVariableCase
MemberCase / PrivateMemberCase / ProtectedMemberCase / PublicMemberCase
ConstantCase / GlobalConstantCase / LocalConstantCase / StaticConstantCase
ConstexprFunctionCase / ConstexprVariableCase / ConstexprMethodCase
MacroDefinitionCase
TemplateParameterCase / TypeTemplateParameterCase / ValueTemplateParameterCase
```

每个实体都可设 4 个属性：

| 后缀 | 含义 | 示例 |
|---|---|---|
| `.Case` | 大小写风格 | `lower_case` / `CamelCase` |
| `.Prefix` | 前缀 | `k`（常量）/ `m_`（成员） |
| `.Suffix` | 后缀 | `_`（私有成员） |
| `.IgnoredRegexp` | 不检查的正则 | `^[A-Z]_test$` |

可用风格值：

| 值 | 示例 |
|---|---|
| `lower_case` | `do_something` |
| `UPPER_CASE` | `MAX_SIZE` |
| `camelBack` | `doSomething` |
| `CamelCase` | `DoSomething` |
| `Camel_Snake_Case` | `Do_Something` |
| `aNy_CasE` | 不检查大小写 |
| `Leading_upper_snake_case` | `Do_something` |

**Google 风格示例**：

```yaml
CheckOptions:
  - { key: readability-identifier-naming.NamespaceCase,        value: lower_case }
  - { key: readability-identifier-naming.ClassCase,            value: CamelCase }
  - { key: readability-identifier-naming.FunctionCase,         value: CamelCase }
  - { key: readability-identifier-naming.VariableCase,         value: lower_case }
  - { key: readability-identifier-naming.PrivateMemberCase,    value: lower_case }
  - { key: readability-identifier-naming.PrivateMemberSuffix,  value: _ }
  - { key: readability-identifier-naming.ConstexprVariableCase, value: CamelCase }
  - { key: readability-identifier-naming.ConstexprVariablePrefix, value: k }
  - { key: readability-identifier-naming.MacroDefinitionCase,  value: UPPER_CASE }
```

**LLVM 风格示例**：

```yaml
CheckOptions:
  - { key: readability-identifier-naming.ClassCase,    value: CamelCase }
  - { key: readability-identifier-naming.FunctionCase, value: camelBack }
  - { key: readability-identifier-naming.VariableCase, value: CamelCase }
```

---

## 9. 按目录分层覆盖

典型布局：

```
project/
├── .clang-tidy                # 主配置（严格）
├── src/                       # 用主配置
├── tests/
│   └── .clang-tidy            # 测试代码放宽
└── third_party/
    └── .clang-tidy            # 三方代码几乎全关
```

**测试代码放宽示例**：

```yaml
# tests/.clang-tidy
InheritParentConfig: true
Checks: >
  -readability-function-cognitive-complexity,
  -cppcoreguidelines-avoid-non-const-global-variables,
  -cppcoreguidelines-owning-memory,
  -bugprone-exception-escape
CheckOptions:
  - key: readability-function-size.LineThreshold
    value: '300'
```

**三方目录关掉所有检查**：

```yaml
# third_party/.clang-tidy
Checks: '-*'
```

---

## 10. 行内禁用与 NOLINT

源代码中可以局部关闭某些 tidy 检查：

```cpp
// 关闭所有 check（仅当前行）
int x = 0xdeadbeef;  // NOLINT

// 关闭具体 check（仅当前行）
int y = 42;  // NOLINT(readability-magic-numbers)

// 关闭多个 check
int z = 0;  // NOLINT(readability-magic-numbers, cppcoreguidelines-init-variables)

// 关闭下一行
// NOLINTNEXTLINE(modernize-use-nullptr)
int *p = NULL;

// 关闭一段范围
// NOLINTBEGIN(readability-magic-numbers)
const int table[] = {1, 2, 3, 5, 8, 13, 21};
// NOLINTEND(readability-magic-numbers)
```

**最佳实践**：始终注明被关掉的具体 check 名，方便审计。

---

## 11. 与 `.clangd` 的关系

`.clangd` 中的 `Diagnostics.ClangTidy` 子项：

```yaml
Diagnostics:
  ClangTidy:
    Add:    [modernize-*, performance-*]
    Remove: [modernize-use-trailing-return-type]
    CheckOptions:
      readability-function-size.LineThreshold: '120'
```

**优先级**：

```
.clang-tidy（项目根/向上递归找的最近文件）
        ▲ 覆盖
.clangd 中的 Diagnostics.ClangTidy
```

也就是说：

- **没有 `.clang-tidy`** → 用 `.clangd` 里写的 ClangTidy 配置
- **有 `.clang-tidy`** → 以它为准，`.clangd` 里的 ClangTidy 配置基本被忽略

**推荐做法**：把所有 tidy 规则集中在 `.clang-tidy`，`.clangd` 里只留一句 `ClangTidy:` 启用即可（或者完全省略，clangd 默认会跑 tidy）。

---

## 12. 命令行使用

```bash
# 检查单个文件（自动找 compile_commands.json）
clang-tidy -p build/my-gcc-relwithdebinfo src/foo.cpp

# 自动修复
clang-tidy -p build/my-gcc-relwithdebinfo --fix src/foo.cpp

# 只跑特定 check（覆盖 .clang-tidy）
clang-tidy --checks='-*,bugprone-*' src/foo.cpp

# 把警告升级为错误
clang-tidy --warnings-as-errors='*' src/foo.cpp

# 看当前生效的配置
clang-tidy --dump-config

# 看某个 check 的所有选项
clang-tidy --checks=readability-function-size --dump-config

# 批量分析（LLVM 自带脚本）
run-clang-tidy.py -p build/my-gcc-relwithdebinfo \
  -header-filter='^.*/(src|include)/.*' \
  src/

# 并行 + 修复
run-clang-tidy.py -p build/my-gcc-relwithdebinfo -fix
```

---

## 13. CI 集成

### 13.1 GitHub Actions

```yaml
- name: clang-tidy
  run: |
    sudo apt-get install -y clang-tidy
    cmake --preset my-gcc-relwithdebinfo
    cmake --build build/my-gcc-relwithdebinfo
    git ls-files 'modules/**/*.cpp' \
      | xargs clang-tidy -p build/my-gcc-relwithdebinfo --warnings-as-errors='*'
```

### 13.2 只检查 PR 改动的文件

```bash
git diff --name-only origin/main...HEAD \
  | grep -E '\.(cpp|hpp|h)$' \
  | xargs -r clang-tidy -p build/my-gcc-relwithdebinfo
```

### 13.3 pre-commit hook

```bash
#!/usr/bin/env bash
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp|h)$')
[ -z "$files" ] && exit 0
clang-tidy -p build/my-gcc-relwithdebinfo --warnings-as-errors='*' $files
```

---

## 14. 常见问题

### Q1: 编辑器里没有 tidy 警告

- 确认 `.clangd` 里没有显式 `ClangTidy: { Add: [] }` 全清空
- 确认 clangd 启动参数没有 `--clang-tidy=false`
- 确认 `compile_commands.json` 路径正确（tidy 需要它）
- 重启 clangd（VS Code: `clangd: Restart language server`）

### Q2: 警告太多，怎么逐步推行

```yaml
# 先启动一小组
Checks: 'modernize-use-nullptr,modernize-use-auto'
# 之后逐步增加
```

或用 NOLINT 标记现有违规，再用 CI 拦截新增。

### Q3: 三方头被分析

- 设置 `HeaderFilterRegex` 只匹配项目路径
- 设置 `SystemHeaders: false`（默认）
- 三方目录加一份 `.clang-tidy` 写 `Checks: '-*'`

### Q4: `--fix` 把代码改坏了

- `--fix` 是激进操作，先 commit 再跑
- 用 `--fix-errors` 只在错误级别修复
- 用 `--fix-notes` 包含关联 note

### Q5: 命名规范报警太多

- 旧项目可以先关掉 `readability-identifier-naming`，新代码逐步启用
- 用 `IgnoredRegexp` 跳过历史命名

### Q6: clangd 内嵌的 tidy 和 CI 用的版本不一致

- 检查 clangd 版本（`clangd --version`）和 clang-tidy 版本（`clang-tidy --version`）
- 不同版本支持的 check 集合不同，新 check 用旧 tidy 会报错
- 建议在 CI 中用与本地 clangd 同版本的 clang-tidy

### Q7: 想看哪些 check 是默认开启的

```bash
clang-tidy --list-checks         # 当前配置启用的
clang-tidy --list-checks -checks='*'  # 所有可用的
```

---

## 附：本项目当前配置回顾

```yaml
Checks: >
  bugprone-*, cert-*, concurrency-*,
  cppcoreguidelines-*, modernize-*,
  performance-*, portability-*, readability-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers,
  ... (其他 ergonomic 关闭项)

HeaderFilterRegex: '^.*/(modules|include|src)/.*\.(h|hpp)$'
FormatStyle: file

CheckOptions:
  - readability-identifier-naming.*  → Google 风格
  - readability-function-size.LineThreshold: 120
  - readability-function-cognitive-complexity.Threshold: 25
  - modernize-use-auto.MinTypeNameLength: 5
  - cppcoreguidelines-special-member-functions.AllowSoleDefaultDtor: 1
```

如需更严，把 `WarningsAsErrors: 'bugprone-*,cert-*'` 打开。如需更宽松，按目录加子级 `.clang-tidy`。
