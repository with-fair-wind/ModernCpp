# `.clang-format` 配置完整指南

> 本文档面向当前 ModernCpp 项目（Windows + MinGW UCRT64 + CMake Presets），系统介绍 `.clang-format` 的所有常用配置项、典型场景与编辑器集成方式。
>
> 官方完整文档：https://clang.llvm.org/docs/ClangFormatStyleOptions.html

---

## 目录

1. [clang-format 是什么](#1-clang-format-是什么)
2. [配置文件加载规则](#2-配置文件加载规则)
3. [基础风格预设（BasedOnStyle）](#3-基础风格预设basedonstyle)
4. [核心配置项分类详解](#4-核心配置项分类详解)
   - 4.1 [缩进与列宽](#41-缩进与列宽)
   - 4.2 [大括号风格](#42-大括号风格)
   - 4.3 [单行短结构](#43-单行短结构)
   - 4.4 [对齐相关](#44-对齐相关)
   - 4.5 [指针与引用](#45-指针与引用)
   - 4.6 [空格控制](#46-空格控制)
   - 4.7 [换行控制](#47-换行控制)
   - 4.8 [include 排序与分组](#48-include-排序与分组)
   - 4.9 [命名空间](#49-命名空间)
   - 4.10 [注释](#410-注释)
   - 4.11 [函数 / lambda / 模板](#411-函数--lambda--模板)
   - 4.12 [现代 C++ 特性](#412-现代-c-特性)
5. [按语言区分配置（Language 多段）](#5-按语言区分配置language-多段)
6. [禁用范围（注释开关）](#6-禁用范围注释开关)
7. [常见场景配方](#7-常见场景配方)
8. [编辑器与 CI 集成](#8-编辑器与-ci-集成)
9. [与 `.clangd` / `.clang-tidy` 的协作](#9-与-clangd--clang-tidy-的协作)
10. [常见问题排查](#10-常见问题排查)

---

## 1. clang-format 是什么

`clang-format` 是 LLVM 项目提供的 C/C++/Objective-C/Java/JavaScript/Proto 等多语言代码格式化工具。它读取项目中的 `.clang-format` 文件，按规则重排空白、换行、缩进、include 顺序，但**不会修改语义**。

clangd LSP 在收到 `textDocument/formatting` 请求时，会调用 clang-format 完成实际格式化，因此**只要项目里有 `.clang-format`，VS Code / Neovim 等编辑器自动格式化就会按它来**。

---

## 2. 配置文件加载规则

- **就近优先**：clang-format 从被格式化的源文件所在目录开始，**向上递归**寻找最近的 `.clang-format`（或 `_clang-format`）文件
- **多文件分层**：子目录可以放自己的 `.clang-format` 完全覆盖父级，或用 `BasedOnStyle: InheritParentConfig` 增量继承
- **找不到时**：使用 clang-format CLI 的 `--style=...` 或 clangd 的 `--fallback-style=...` 指定的内置风格；都没有就用 LLVM 默认
- **行内禁用**：源代码中 `// clang-format off` / `// clang-format on` 之间的内容跳过格式化

---

## 3. 基础风格预设（BasedOnStyle）

| 预设 | 风格特点 | 缩进 | 列宽 |
|---|---|---|---|
| **LLVM** | LLVM 项目自身风格 | 2 | 80 |
| **Google** | Google C++ Style Guide | 2 | 80 |
| **Chromium** | 基于 Google 微调 | 2 | 80 |
| **Mozilla** | Firefox 项目 | 2 | 80 |
| **WebKit** | Safari 引擎 | 4 | 不限 |
| **Microsoft** | 微软风格（与 .NET 接近） | 4 | 120 |
| **GNU** | GNU 项目（大括号 Allman） | 2 | 79 |
| **InheritParentConfig** | 继承父级 .clang-format（用于子目录覆盖） | — | — |

**用法**：先选一个最接近的预设，再用其它字段做覆盖。本项目选 `Google` + 4 空格缩进 + 100 列宽。

```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
```

---

## 4. 核心配置项分类详解

### 4.1 缩进与列宽

| 选项 | 取值 | 说明 |
|---|---|---|
| `IndentWidth` | int | 一级缩进空格数 |
| `TabWidth` | int | Tab 显示宽度 |
| `UseTab` | `Never` / `ForIndentation` / `ForContinuationAndIndentation` / `Always` | 是否用 Tab |
| `ColumnLimit` | int（0 = 不限） | 每行最大字符数 |
| `ContinuationIndentWidth` | int | 续行缩进（默认 4） |
| `ConstructorInitializerIndentWidth` | int | 构造函数初始化列表缩进 |
| `AccessModifierOffset` | int（可负） | `public:` 等相对类体的偏移 |
| `IndentCaseLabels` | bool | switch 的 case 是否额外缩进 |
| `IndentPPDirectives` | `None` / `AfterHash` / `BeforeHash` | `#define` 等预处理指令缩进 |
| `IndentWrappedFunctionNames` | bool | 长函数名换行后是否缩进 |
| `NamespaceIndentation` | `None` / `Inner` / `All` | namespace 内是否缩进 |

### 4.2 大括号风格

| 选项 | 说明 |
|---|---|
| `BreakBeforeBraces` | 整体大括号风格：`Attach` / `Linux` / `Mozilla` / `Stroustrup` / `Allman` / `GNU` / `WebKit` / `Custom` |
| `BraceWrapping` | `Custom` 时的细粒度控制（每种结构单独设是否换行） |

`BraceWrapping` 子项（仅 `Custom` 生效）：

```yaml
BreakBeforeBraces: Custom
BraceWrapping:
  AfterClass:      false   # class Foo {
  AfterControlStatement: Never   # if (x) {
  AfterEnum:       false
  AfterFunction:   true    # void foo()\n{
  AfterNamespace:  false
  AfterStruct:     false
  AfterUnion:      false
  BeforeCatch:     false   # } catch
  BeforeElse:      false   # } else
  BeforeLambdaBody: false
  BeforeWhile:     false   # do {} while
  IndentBraces:    false
  SplitEmptyFunction: false
  SplitEmptyRecord:   false
  SplitEmptyNamespace: false
```

### 4.3 单行短结构

| 选项 | 取值 | 说明 |
|---|---|---|
| `AllowShortBlocksOnASingleLine` | `Never` / `Empty` / `Always` | 短代码块 `{ stmt; }` |
| `AllowShortCaseLabelsOnASingleLine` | bool | `case 1: return;` |
| `AllowShortEnumsOnASingleLine` | bool | `enum E {A, B};` |
| `AllowShortFunctionsOnASingleLine` | `None` / `InlineOnly` / `Empty` / `Inline` / `All` | 短函数 |
| `AllowShortIfStatementsOnASingleLine` | `Never` / `WithoutElse` / `OnlyFirstIf` / `AllIfsAndElse` | 短 if |
| `AllowShortLambdasOnASingleLine` | `None` / `Empty` / `Inline` / `All` | 短 lambda |
| `AllowShortLoopsOnASingleLine` | bool | 短 for/while |

### 4.4 对齐相关

| 选项 | 说明 |
|---|---|
| `AlignAfterOpenBracket` | `Align` / `DontAlign` / `AlwaysBreak` / `BlockIndent` —— 函数调用括号后参数是否对齐 |
| `AlignArrayOfStructures` | `None` / `Left` / `Right` —— 结构体数组对齐 |
| `AlignConsecutiveAssignments` | 连续赋值的 `=` 对齐 |
| `AlignConsecutiveBitFields` | 连续位域对齐 |
| `AlignConsecutiveDeclarations` | 连续变量声明对齐 |
| `AlignConsecutiveMacros` | 连续 `#define` 对齐 |
| `AlignEscapedNewlines` | `DontAlign` / `Left` / `Right` —— 宏中的 `\` 对齐 |
| `AlignOperands` | `DontAlign` / `Align` / `AlignAfterOperator` —— 多行表达式操作数对齐 |
| `AlignTrailingComments` | bool / 详细对象 —— 行尾注释对齐 |

`AlignConsecutive*` 系列的可用对象写法：

```yaml
AlignConsecutiveAssignments:
  Enabled: true
  AcrossEmptyLines: false   # 跨空行是否仍对齐
  AcrossComments: false
  AlignCompound: true       # +=, -= 也对齐
  PadOperators: true
```

### 4.5 指针与引用

| 选项 | 取值 | 效果 |
|---|---|---|
| `PointerAlignment` | `Left` / `Right` / `Middle` | `int*` / `int *` / `int * ` |
| `ReferenceAlignment` | `Pointer` / `Left` / `Right` / `Middle` | `int&` / `int &` |
| `DerivePointerAlignment` | bool | true 时从代码统计推导，覆盖上面两项 |

### 4.6 空格控制

| 选项 | 说明 |
|---|---|
| `SpaceAfterCStyleCast` | `(int)x` vs `(int) x` |
| `SpaceAfterLogicalNot` | `!x` vs `! x` |
| `SpaceAfterTemplateKeyword` | `template<>` vs `template <>` |
| `SpaceAroundPointerQualifiers` | `Default` / `Before` / `After` / `Both` |
| `SpaceBeforeAssignmentOperators` | bool |
| `SpaceBeforeCpp11BracedList` | `Foo {1}` vs `Foo{1}` |
| `SpaceBeforeCtorInitializerColon` | `Foo(): x()` vs `Foo() : x()` |
| `SpaceBeforeInheritanceColon` | bool |
| `SpaceBeforeParens` | `Never` / `ControlStatements` / `ControlStatementsExceptControlMacros` / `NonEmptyParentheses` / `Always` / `Custom` |
| `SpaceBeforeRangeBasedForLoopColon` | bool |
| `SpaceBeforeSquareBrackets` | bool |
| `SpaceInEmptyBlock` | `{}` vs `{ }` |
| `SpaceInEmptyParentheses` | `()` vs `( )` |
| `SpacesBeforeTrailingComments` | int —— 行尾注释前空格数 |
| `SpacesInAngles` | `Never` / `Always` / `Leave` —— `<T>` |
| `SpacesInCStyleCastParentheses` | bool |
| `SpacesInConditionalStatement` | bool —— `if ( x )` |
| `SpacesInContainerLiterals` | bool |
| `SpacesInParentheses` | bool |
| `SpacesInSquareBrackets` | bool |

### 4.7 换行控制

| 选项 | 说明 |
|---|---|
| `AllowAllArgumentsOnNextLine` | 允许所有参数换到下一行 |
| `AllowAllParametersOfDeclarationOnNextLine` | 同上但用于声明 |
| `AlwaysBreakAfterReturnType` | `None` / `All` / `TopLevel` / `AllDefinitions` / `TopLevelDefinitions` |
| `AlwaysBreakBeforeMultilineStrings` | bool |
| `AlwaysBreakTemplateDeclarations` | `No` / `MultiLine` / `Yes` |
| `BinPackArguments` | bool —— false 时每个实参单独一行 |
| `BinPackParameters` | bool —— false 时每个形参单独一行 |
| `BreakBeforeBinaryOperators` | `None` / `NonAssignment` / `All` |
| `BreakBeforeConceptDeclarations` | `Never` / `Allowed` / `Always` |
| `BreakBeforeTernaryOperators` | bool |
| `BreakConstructorInitializers` | `BeforeColon` / `BeforeComma` / `AfterColon` |
| `BreakInheritanceList` | 同上 |
| `BreakStringLiterals` | bool —— 长字符串是否拆行 |
| `MaxEmptyLinesToKeep` | int —— 连续空行最大数 |
| `KeepEmptyLinesAtTheStartOfBlocks` | bool |

### 4.8 include 排序与分组

| 选项 | 说明 |
|---|---|
| `SortIncludes` | `Never` / `CaseSensitive` / `CaseInsensitive` |
| `IncludeBlocks` | `Preserve` / `Merge` / `Regroup` |
| `IncludeCategories` | 列表，按正则匹配指定 Priority |
| `IncludeIsMainRegex` | 主源文件名正则（用于把 `foo.cc` 对应的 `foo.h` 放最上面） |
| `IncludeIsMainSourceRegex` | 反向：主头文件名正则 |

完整示例：

```yaml
IncludeBlocks: Regroup
IncludeCategories:
  - Regex: '^"(my_project)/'   # 项目内绝对路径
    Priority: 1
  - Regex: '^"'                # 其他双引号
    Priority: 2
  - Regex: '^<.*\.h(pp)?>'     # 三方库
    Priority: 3
  - Regex: '^<[a-z_]+>'        # 标准库
    Priority: 4
SortIncludes: CaseSensitive
IncludeIsMainRegex: '(_test)?$'
```

### 4.9 命名空间

| 选项 | 说明 |
|---|---|
| `FixNamespaceComments` | 自动加/修正 `// namespace foo` |
| `NamespaceIndentation` | 内部缩进：`None` / `Inner` / `All` |
| `CompactNamespaces` | `namespace A { namespace B {` 压成一行 |
| `ShortNamespaceLines` | int —— 多短的 namespace 不加 // 注释 |

### 4.10 注释

| 选项 | 说明 |
|---|---|
| `ReflowComments` | 自动 reflow 长注释到 ColumnLimit |
| `CommentPragmas` | 正则：匹配的注释不动（如 `IWYU pragma:`） |

### 4.11 函数 / lambda / 模板

| 选项 | 说明 |
|---|---|
| `LambdaBodyIndentation` | `Signature` / `OuterScope` |
| `RequiresClausePosition` | `OwnLine` / `WithPreceding` / `WithFollowing` / `SingleLine` —— C++20 requires 子句位置 |
| `RequiresExpressionIndentation` | `OuterScope` / `Keyword` |
| `IndentRequiresClause` | bool |
| `PackConstructorInitializers` | `Never` / `BinPack` / `CurrentLine` / `NextLine` |
| `EmptyLineAfterAccessModifier` | `Never` / `Leave` / `Always` |
| `EmptyLineBeforeAccessModifier` | 同上 |
| `SeparateDefinitionBlocks` | `Leave` / `Always` / `Never` —— 函数/类定义之间是否强制空行 |

### 4.12 现代 C++ 特性

| 选项 | 说明 |
|---|---|
| `Cpp11BracedListStyle` | true 时按函数调用风格格式化列表初始化 |
| `FixNamespaceComments` | 见上 |
| `Standard` | `c++03` / `11` / `14` / `17` / `20` / `Latest` —— 解析时使用的标准 |
| `AlwaysBreakTemplateDeclarations` | template<> 后是否强制换行 |
| `BreakBeforeConceptDeclarations` | concept 前是否换行 |
| `RequiresClausePosition` | requires 子句位置（C++20） |

---

## 5. 按语言区分配置（Language 多段）

如果同一项目有 C++/Java/Proto 等多语言代码，可以在一个 `.clang-format` 里写多段：

```yaml
---
Language: Cpp
BasedOnStyle: Google
IndentWidth: 4

---
Language: Proto
BasedOnStyle: Google
IndentWidth: 2

---
Language: Java
BasedOnStyle: Google
ContinuationIndentWidth: 8
```

每段以 `---` 分隔，clang-format 根据文件后缀自动选择对应段。

---

## 6. 禁用范围（注释开关）

源代码内可以局部关闭 / 打开格式化：

```cpp
// clang-format off
const int magic_table[5][5] = {
    { 1, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0 },
    { 0, 0, 1, 0, 0 },
    { 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 1 },
};
// clang-format on
```

适用场景：手工对齐的表格、ASCII art、宏展开、特殊宽度的对齐代码。

---

## 7. 常见场景配方

### 7.1 严格 80 列 + LLVM 风格

```yaml
BasedOnStyle: LLVM
ColumnLimit: 80
```

### 7.2 偏 Qt / KDE 的 Allman 大括号

```yaml
BasedOnStyle: WebKit
BreakBeforeBraces: Allman
IndentWidth: 4
```

### 7.3 强制每个参数单独一行（适合参数多的工厂函数）

```yaml
BinPackParameters: false
BinPackArguments: false
AllowAllParametersOfDeclarationOnNextLine: false
```

### 7.4 子目录覆盖父级

```yaml
# third_party/.clang-format
BasedOnStyle: InheritParentConfig
ColumnLimit: 0           # 三方代码不限列宽
SortIncludes: Never      # 三方代码 include 顺序不动
```

### 7.5 头文件分组（与本项目一致）

```yaml
IncludeBlocks: Regroup
IncludeCategories:
  - Regex: '^<[a-z_]+>$'      # 标准库
    Priority: 1
  - Regex: '^<.*\.h(pp)?>$'   # 三方
    Priority: 2
  - Regex: '^".*"$'           # 本地
    Priority: 3
SortIncludes: CaseSensitive
```

---

## 8. 编辑器与 CI 集成

### 8.1 VS Code

settings.json：

```json
{
  "editor.formatOnSave": true,
  "[cpp]":   { "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd" },
  "[c]":     { "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd" },
  "clangd.arguments": [
    "--fallback-style=Google"   // 找不到 .clang-format 时的兜底
  ]
}
```

### 8.2 Neovim (nvim-lspconfig + clangd)

clangd 自带格式化能力，绑定保存时格式化即可：

```lua
vim.api.nvim_create_autocmd("BufWritePre", {
  pattern = { "*.cpp", "*.hpp", "*.c", "*.h" },
  callback = function() vim.lsp.buf.format({ async = false }) end,
})
```

### 8.3 命令行

```bash
# 格式化单个文件（写回）
clang-format -i src/foo.cpp

# 检查未格式化的文件（CI）
clang-format --dry-run --Werror src/**/*.cpp src/**/*.hpp

# 用项目 .clang-format 验证
git ls-files '*.cpp' '*.hpp' | xargs clang-format --dry-run --Werror
```

### 8.4 Git pre-commit hook

```bash
#!/usr/bin/env bash
git diff --cached --name-only --diff-filter=ACM \
  | grep -E '\.(cpp|hpp|h|cc|c)$' \
  | xargs -r clang-format --dry-run --Werror
```

### 8.5 GitHub Actions

```yaml
- name: Check clang-format
  run: |
    sudo apt-get install -y clang-format
    git ls-files '*.cpp' '*.hpp' | xargs clang-format --dry-run --Werror
```

---

## 9. 与 `.clangd` / `.clang-tidy` 的协作

`.clang-format` 只控制「格式」。它与另外两个配置文件分工如下：

| 配置文件 | 关心什么 | 何时触发 |
|---|---|---|
| `.clang-format` | **怎么排版**（空白/换行/对齐/include 顺序） | 保存时格式化 |
| `.clang-tidy`   | **代码逻辑/风格规则**（命名/魔法数/move 优化） | 编辑时实时诊断、CI |
| `.clangd`       | **clangd LSP 自身行为**（要不要跑 tidy、提示样式） | clangd 启动时 |

**触发链路**：

```
你保存文件
   │
   ▼
编辑器 → clangd（textDocument/formatting）
                │
                └─ 调用 clang-format 读取 .clang-format → 返回格式化结果
```

```
你编辑代码
   │
   ▼
编辑器 → clangd（持续诊断）
                │
                ├─ 编译诊断（基于 compile_commands.json）
                └─ clang-tidy 诊断（读取 .clang-tidy） → 红线/quickfix
```

详见 [`clangd-config-guide.md`](./clangd-config-guide.md) 的「整体框架」一节。

---

## 10. 常见问题排查

### Q1: 保存时没有自动格式化

- 检查 VS Code `editor.formatOnSave: true`
- 检查 `[cpp]` 区是否设置了 `defaultFormatter` 为 `llvm-vs-code-extensions.vscode-clangd`
- 确认有装 clangd 插件而非 ms-vscode.cpptools

### Q2: 部分文件被忽略

- 检查 `.gitignore` / `.clangd-format` 文件名拼写（注意是 `.clang-format` 不是 `.clang_format`）
- 检查文件后缀是否被 clang-format 识别（`.cppm` 等模块文件需要更新工具链）

### Q3: 想看当前规则下某段代码会被改成什么

```bash
clang-format -style=file < src/foo.cpp | diff -u src/foo.cpp -
```

### Q4: 生成 yaml 表示当前内置风格

```bash
clang-format -style=Google -dump-config
```

把这个输出作为起点，删掉默认值后只保留覆盖项即可。

### Q5: 格式化和 `.clang-tidy` 修复冲突

`.clang-tidy` 中设置 `FormatStyle: file`，这样 `clang-tidy --fix` 会调用 clang-format 用项目规则收尾。

### Q6: 想为某个目录单独定一套规则

```
project/
├── .clang-format            # BasedOnStyle: Google
├── src/
│   └── (使用顶层规则)
└── third_party/
    └── .clang-format        # BasedOnStyle: InheritParentConfig + ColumnLimit: 0
```

---

## 附：本项目当前配置回顾

```yaml
BasedOnStyle: Google      # 起点
IndentWidth: 4            # 4 空格缩进（Google 默认 2）
ColumnLimit: 100          # 100 列（Google 默认 80）
PointerAlignment: Left    # int* p
BreakBeforeBraces: Attach # K&R 风格
NamespaceIndentation: None
IncludeBlocks: Regroup    # 自动分组排序
SortIncludes: CaseSensitive
FixNamespaceComments: true
```

如果你想切换为 LLVM / Microsoft / Allman 风格，只需要改 `BasedOnStyle` 和 `BreakBeforeBraces`，其余项 clang-format 会自动用预设默认值填充。
