# clangd 工具链整体使用框架

> 本文档描述 clangd 如何串联 `.clangd`、`.clang-format`、`.clang-tidy`、`compile_commands.json` 这一整套配置文件，作为 C++ LSP 工具链的"鸟瞰图"。
>
> 单文件细节请看：
> - [`clangd-config-guide.md`](./clangd-config-guide.md) —— `.clangd` 完整配置参考
> - [`clang-format-config-guide.md`](./clang-format-config-guide.md) —— `.clang-format` 完整配置参考
> - [`clang-tidy-config-guide.md`](./clang-tidy-config-guide.md) —— `.clang-tidy` 完整配置参考

---

## 目录

1. [整体架构图](#1-整体架构图)
2. [四类配置文件的分工](#2-四类配置文件的分工)
3. [clangd 启动流程](#3-clangd-启动流程)
4. [典型 LSP 请求的内部链路](#4-典型-lsp-请求的内部链路)
5. [优先级与覆盖关系](#5-优先级与覆盖关系)
6. [推荐的项目布局](#6-推荐的项目布局)
7. [本项目实际配置全貌](#7-本项目实际配置全貌)
8. [新建项目的最小可用模板](#8-新建项目的最小可用模板)
9. [常见配置组合速查](#9-常见配置组合速查)
10. [排查问题的顺序](#10-排查问题的顺序)

---

## 1. 整体架构图

```
                    ┌──────────────────────────────────┐
                    │  编辑器（VS Code / Neovim / ...）│
                    │   - clangd 插件 / nvim-lspconfig │
                    │   - clangd.arguments（CLI 参数） │
                    └─────────────┬────────────────────┘
                                  │ LSP（JSON-RPC over stdio）
                                  ▼
                    ┌──────────────────────────────────┐
                    │             clangd               │
                    │  （C++ Language Server）         │
                    │                                  │
                    │  读取：                          │
                    │   - CLI 参数（最高优先级）       │
                    │   - .clangd 文件                 │
                    │   - compile_commands.json        │
                    └─┬────────┬───────────┬───────────┘
                      │        │           │
            ┌─────────┘        │           └──────────┐
            ▼                  ▼                      ▼
   ┌──────────────────┐  ┌──────────────┐    ┌─────────────────┐
   │   编译诊断       │  │ clang-tidy   │    │  clang-format   │
   │ (libclang AST)   │  │ (内嵌调用)    │    │  (内嵌调用)     │
   │                  │  │              │    │                 │
   │ 用：             │  │ 读取：       │    │ 读取：          │
   │  compile_        │  │  .clang-tidy │    │  .clang-format  │
   │  commands.json   │  │              │    │                 │
   └──────────────────┘  └──────────────┘    └─────────────────┘
```

四个数据来源、三个执行引擎、一个对外接口（LSP）。

---

## 2. 四类配置文件的分工

| 文件 | 谁用 | 关心的事 | 何时被读 |
|---|---|---|---|
| **`.clangd`** | clangd 自身 | LSP 行为：CompileFlags、Completion、Diagnostics 总开关、InlayHints、Index、Hover... | clangd 启动时（需 `--enable-config`，默认开） |
| **`compile_commands.json`** | clangd 的编译前端 | 每个 .cpp 用什么 flag 编译（标准、include 路径、宏） | 打开文件时 |
| **`.clang-format`** | clang-format（被 clangd 调用） | 代码格式化规则 | 触发 `formatting` 请求时 |
| **`.clang-tidy`** | clang-tidy（被 clangd 内嵌调用） | 静态检查规则、命名规范、性能/安全建议 | 文件打开/编辑时 |

**记住一句话**：
- `compile_commands.json` 决定 clangd「**懂不懂**」你的代码（找头文件、解析符号）
- `.clangd` 决定 clangd「**做什么**」（要不要跑 tidy、要不要插头、提示样式）
- `.clang-tidy` 决定 tidy「**查什么**」（具体哪些规则）
- `.clang-format` 决定格式化「**怎么排**」

---

## 3. clangd 启动流程

时间顺序：

```
1. 编辑器启动 → 启动 clangd 子进程，传入 CLI 参数
                  （--compile-commands-dir, --query-driver, --fallback-style ...）

2. clangd 接收 LSP initialize 请求
   - 拿到 workspace 根目录
   - 加载 ~/.config/clangd/config.yaml（用户全局）
   - 加载 <workspace>/.clangd（项目级）
   - 合并：CLI 参数 > .clangd > 全局 config.yaml > 内置默认

3. 用户打开一个 .cpp 文件
   - clangd 沿着文件目录向上找 compile_commands.json
     · 优先用 CLI --compile-commands-dir 指定的位置
     · 否则用 .clangd 中 CompileFlags.CompilationDatabase 指定
     · 否则递归向上找
   - 拿到该文件的编译命令（含 -std、-I、-D、-isystem 等）
   - 启动后台索引（如果 Index.Background: Build）

4. 解析 + 诊断
   - 用 libclang 解析为 AST
   - 跑编译器警告
   - 跑 clang-tidy（如果 .clangd 没禁用）
     · tidy 沿着文件目录向上找 .clang-tidy
     · 合并 .clangd 中的 Diagnostics.ClangTidy 配置
   - 把所有诊断包装成 LSP diagnostics 推给编辑器
```

---

## 4. 典型 LSP 请求的内部链路

### 4.1 打开文件 / 实时编辑

```
编辑器: textDocument/didOpen / didChange
   │
   ▼
clangd
   ├─ 查 compile_commands.json → 拿到编译参数
   ├─ libclang 解析 → AST
   ├─ 编译警告（-Wall -Wextra -Wpedantic 由 .clangd 的 CompileFlags.Add 追加）
   └─ clang-tidy 内嵌运行 → 读 .clang-tidy → 合并 .clangd 的 ClangTidy 配置
                                                      │
                                                      ▼
   ◄── 所有诊断 → textDocument/publishDiagnostics ──┘
```

### 4.2 代码补全

```
编辑器: textDocument/completion
   │
   ▼
clangd
   ├─ 查当前作用域可见符号
   ├─ 如果 .clangd 的 Completion.AllScopes: Yes → 全局符号也建议
   ├─ 如果 .clangd 的 Completion.HeaderInsertion: IWYU → 自动追加 #include
   └─ 返回补全候选列表
```

### 4.3 格式化

```
编辑器: textDocument/formatting
   │
   ▼
clangd
   ├─ 找 .clang-format（向上递归）
   ├─ 找不到 → 用 CLI --fallback-style 指定的内置风格
   └─ 调用内嵌 clang-format → 返回 TextEdit 列表
```

### 4.4 跳转定义

```
编辑器: textDocument/definition
   │
   ▼
clangd
   ├─ 查后台索引（如果 Index.Background: Build）
   ├─ 没索引就实时 parse 项目
   └─ 返回 LocationLink
```

### 4.5 InlayHints

```
编辑器: textDocument/inlayHint
   │
   ▼
clangd
   ├─ 检查 .clangd 的 InlayHints.Enabled
   ├─ 按子开关（ParameterNames / DeducedTypes / ...）生成提示
   └─ 返回 InlayHint 列表
```

---

## 5. 优先级与覆盖关系

### 5.1 编译参数（CompileFlags）

```
[最高] CLI 参数（如 VS Code clangd.arguments 中的 --query-driver）
   ▲
   │ 覆盖
.clangd 中的 CompileFlags.Add / CompileFlags.Remove
   ▲
   │ 应用到
compile_commands.json 里的原始命令
```

### 5.2 tidy 规则

```
[最高] CLI 的 --checks=... （手动跑 clang-tidy 时）
   ▲
   │ 覆盖
.clang-tidy 中的 Checks
   ▲
   │ 覆盖
.clangd 中的 Diagnostics.ClangTidy
```

注意：**只要项目有 `.clang-tidy`，`.clangd` 里的 ClangTidy 配置基本就被忽略了**。所以二选一，要么集中在 `.clang-tidy`（推荐），要么集中在 `.clangd`。

### 5.3 格式化风格

```
[最高] CLI 的 --style=... （手动跑 clang-format 时）
   ▲
   │ 覆盖
.clang-format（最近的，向上递归）
   ▲
   │ 覆盖（找不到 .clang-format 时）
clangd CLI 的 --fallback-style
   ▲
   │ 覆盖
clang-format 内置默认（LLVM）
```

### 5.4 配置文件就近优先

`.clangd`、`.clang-format`、`.clang-tidy` 都遵循**就近优先**原则：

```
project/
├── .clang-tidy                  # 主配置
├── tests/
│   └── .clang-tidy              # 测试目录覆盖（默认替换；继承需 InheritParentConfig: true）
└── third_party/
    └── .clang-tidy              # 三方目录关掉所有
```

`.clangd` 比较特殊，它支持 `If` 条件块在**同一个文件内**按路径分流，所以一般只在项目根放一份。

---

## 6. 推荐的项目布局

```
ModernCpp/
├── CMakeLists.txt
├── CMakePresets.json
├── CMakeUserPresets.json
├── compile_commands.json        # （可选）软链接到当前活跃 build
│
├── .clangd                      # clangd LSP 配置（已有）
├── .clang-format                # 代码格式化规则（已有）
├── .clang-tidy                  # 静态检查规则（已有）
│
├── .vscode/
│   └── settings.json            # 含 clangd.arguments
│
├── docs/
│   ├── clangd-toolchain-overview.md   # 本文（鸟瞰）
│   ├── clangd-config-guide.md         # .clangd 详细
│   ├── clang-format-config-guide.md   # .clang-format 详细
│   └── clang-tidy-config-guide.md     # .clang-tidy 详细
│
├── modules/
│   └── 01_basics/
│       ├── CMakeLists.txt
│       ├── demos/
│       └── tests/
│           └── (可选) .clang-tidy   # 测试目录放宽规则
│
├── third_party/
│   └── (可选) .clang-tidy           # Checks: '-*'
│
└── build/
    └── mingw-gcc-relwithdebinfo/
        └── compile_commands.json    # CMake 生成
```

---

## 7. 本项目实际配置全貌

### 7.1 三个配置文件的分工

| 文件 | 主要内容 |
|---|---|
| **`.clangd`** | `CompilationDatabase: .`（DB 由 CMake 自动镜像到源码根）、`-std=c++23 -Wall -Wextra -Wpedantic`、`AllScopes/IWYU`、`MissingIncludes/UnusedIncludes: Strict`、ClangTidy 启用组、Index.Background: Build、InlayHints 全开 |
| **`.clang-format`** | `BasedOnStyle: Google` + 4 空格缩进 + 100 列宽 + `PointerAlignment: Left` + `IncludeBlocks: Regroup` |
| **`.clang-tidy`** | 主流 6 组 check（bugprone/cert/cppcoreguidelines/modernize/performance/readability）+ Google 命名风格 + 函数大小阈值 |

### 7.2 编辑器侧（VS Code 推荐配置）

```jsonc
{
  // 保存时格式化（调用 clang-format 经 clangd）
  "editor.formatOnSave": true,
  "[cpp]": { "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd" },
  "[c]":   { "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd" },

  // clangd 启动参数
  "clangd.arguments": [
    // MinGW 必需：让 clangd 用 g++ 推断标准头/宏
    "--query-driver=F:/scoop/apps/msys2/current/ucrt64/bin/g++.exe",

    // 找不到 .clang-format 时的兜底（项目里有 .clang-format 时可注释掉）
    // "--fallback-style=Google",

    // 以下都可由 .clangd 配置实现，CLI 中无需重复：
    // --background-index           （默认开）
    // --clang-tidy                 （默认开）
    // --enable-config              （默认开）
    // --all-scopes-completion      （已在 .clangd Completion.AllScopes）
    // --header-insertion=iwyu      （已在 .clangd Completion.HeaderInsertion）
    // --compile-commands-dir=...   （已在 .clangd CompileFlags.CompilationDatabase）
  ]
}
```

### 7.3 切换 build preset 时

本仓库不需要改 `.clangd`：顶层 `CMakeLists.txt` 注册的 `mcpp_link_compile_commands`
custom target 会在每次 build 时把活跃 preset 的 `compile_commands.json` 镜像到源码
根目录，clangd 配 `CompilationDatabase: .` 即可拿到最新 DB。

```bash
cmake --preset mingw-gcc-debug          # 切到 debug preset
cmake --build --preset mingw-gcc-debug  # build 时自动更新源码根的 compile_commands.json
```

clangd 检测到 DB 变化会自动重新加载（或通过编辑器命令 `clangd: Restart language server` 强制重启）。

---

## 8. 新建项目的最小可用模板

### 8.1 `.clangd`

```yaml
CompileFlags:
  CompilationDatabase: build
  Add: [-std=c++20, -Wall, -Wextra]

Completion:
  AllScopes: Yes
  HeaderInsertion: IWYU

Diagnostics:
  MissingIncludes: Strict
  UnusedIncludes: Strict

Index:
  Background: Build

InlayHints:
  Enabled: Yes
```

### 8.2 `.clang-format`

```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
PointerAlignment: Left
```

### 8.3 `.clang-tidy`

```yaml
Checks: 'bugprone-*,modernize-*,performance-*,readability-*,-readability-magic-numbers'
HeaderFilterRegex: '^.*/(src|include)/.*\.(h|hpp)$'
FormatStyle: file
```

---

## 9. 常见配置组合速查

| 需求 | 改哪里 |
|---|---|
| 切换 build 目录 | `.clangd` 的 `CompileFlags.CompilationDatabase` |
| 改 C++ 标准（仅影响 clangd） | `.clangd` 的 `CompileFlags.Add: [-std=c++NN]` |
| 改 C++ 标准（影响实际编译） | `CMakeLists.txt` 的 `set(CMAKE_CXX_STANDARD ...)` |
| 关闭某个编译警告 | `.clangd` 的 `CompileFlags.Add: [-Wno-xxx]` |
| 关闭某个 tidy check | `.clang-tidy` 的 `Checks: ... ,-foo-bar` |
| 改命名风格 | `.clang-tidy` 的 `CheckOptions.readability-identifier-naming.*` |
| 改缩进/列宽 | `.clang-format` |
| 改 include 排序 | `.clang-format` 的 `IncludeBlocks` + `IncludeCategories` |
| 关掉 InlayHints | `.clangd` 的 `InlayHints.Enabled: No` |
| 关掉某类 InlayHint | `.clangd` 的 `InlayHints.{ParameterNames,DeducedTypes,...}: No` |
| 三方目录不分析 | `third_party/.clang-tidy` 写 `Checks: '-*'`，或 `.clangd` 用 `If.PathMatch` |
| MinGW 找不到标准头 | VS Code `clangd.arguments` 加 `--query-driver=...g++.exe` |
| 没装 .clang-format 的临时风格 | VS Code `clangd.arguments` 加 `--fallback-style=Google` |

---

## 10. 排查问题的顺序

遇到 clangd 行为异常时，按下面的顺序检查：

### Step 1：确认 clangd 真的读到了配置

VS Code 中 `clangd: Open project configuration file` → 看是否打开的是项目根的 `.clangd`。

或者命令行：

```bash
clangd --check=path/to/file.cpp 2>&1 | head -40
```

输出会打印它用的编译命令、是否启用了 tidy、配置文件路径。

### Step 2：确认 compile_commands.json 找到了

```bash
# 看 clangd 用的是哪个 compile_commands.json
clangd --check=src/foo.cpp 2>&1 | grep -i 'compile_commands'
```

如果没找到 → 检查 `.clangd` 的 `CompilationDatabase` 路径，或 VS Code 的 `--compile-commands-dir`。

### Step 3：确认 tidy 在跑

打开一个 .cpp 文件，故意写：

```cpp
int* p = 0;
```

应该看到 `modernize-use-nullptr` 警告。如果没有 → tidy 没启用。

### Step 4：确认 format 在跑

手动 `Format Document`，看是否按 `.clang-format` 排版。如果不动 → 检查编辑器默认 formatter。

### Step 5：看 clangd 日志

VS Code: `View → Output → clangd`。
能看到 tidy 启动信息、配置加载顺序、报错原因。

### Step 6：升级 clangd

不少 check 和配置项是新版本才有。`clangd --version` 看一眼，<= 14 的版本很多 check 不可用。

---

## 总结

```
.clangd          → 控制 clangd 怎么工作（总指挥）
.clang-format    → 格式化规则（被 clangd 调用）
.clang-tidy      → 静态检查规则（被 clangd 内嵌运行）
compile_commands.json → 编译参数数据库（从 CMake 生成）
```

四份文件各司其职，互不重叠。把它们一起 commit 进版本库，团队所有人 IDE 行为就一致了——这是 clangd 工具链的核心价值。
