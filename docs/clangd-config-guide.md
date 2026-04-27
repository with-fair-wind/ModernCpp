# clangd 配置完全指南

> 面向本项目（Windows + MinGW UCRT64 + vcpkg + CMake Presets）的实战手册。
> 同时适用于 VS Code、Neovim、Emacs、Sublime、CLion 等所有支持 LSP 的编辑器。

---

## 目录

- [1. 概述：clangd 是什么](#1-概述clangd-是什么)
- [2. 配置层级与优先级](#2-配置层级与优先级)
- [3. `.clangd` 文件加载规则](#3-clangd-文件加载规则)
- [4. 完整配置项参考](#4-完整配置项参考)
  - [4.1 `CompileFlags`](#41-compileflags)
  - [4.2 `Completion`](#42-completion)
  - [4.3 `Diagnostics`](#43-diagnostics)
  - [4.4 `Index`](#44-index)
  - [4.5 `InlayHints`](#45-inlayhints)
  - [4.6 `Hover`](#46-hover)
  - [4.7 `SemanticTokens`](#47-semantictokens)
  - [4.8 `Style`](#48-style)
  - [4.9 `If` —— 条件配置](#49-if--条件配置)
- [5. clangd CLI 参数（编辑器侧）](#5-clangd-cli-参数编辑器侧)
- [6. 与编辑器插件的配合](#6-与编辑器插件的配合)
  - [6.1 VS Code](#61-vs-code)
  - [6.2 Neovim](#62-neovim)
  - [6.3 其他编辑器](#63-其他编辑器)
- [7. 常见场景配置示例](#7-常见场景配置示例)
- [8. 与 `.clang-format` / `.clang-tidy` 的关系](#8-与-clang-format--clang-tidy-的关系)
- [9. 排查清单（FAQ）](#9-排查清单faq)
- [10. 本项目当前配置回顾](#10-本项目当前配置回顾)

---

## 1. 概述：clangd 是什么

**clangd** 是 LLVM 项目官方的 C/C++ 语言服务器，遵循 **LSP（Language Server Protocol）** 协议，提供：

- 跳转定义、查找引用、重命名
- 智能补全（含跨作用域、自动 include）
- 实时诊断（语法错误 / clang-tidy 静态检查）
- 代码格式化（调用 clang-format）
- 内联类型提示（InlayHints）
- 语义高亮、悬浮信息、调用层级

**重要事实**：

> clangd **是独立的二进制程序**，不属于任何编辑器。  
> VS Code / Neovim / Emacs / Sublime / CLion 都只是 LSP 客户端，最终启动同一个 `clangd.exe`。  
> **`.clangd` 配置文件由 clangd 程序自身解析，与编辑器无关。**

---

## 2. 配置层级与优先级

clangd 的配置来自三个层级，**优先级从高到低**：

```
┌──────────────────────────────────────────────────────────────┐
│  ① 编辑器传入的 CLI 参数（如 VS Code 的 clangd.arguments）   │  ← 最高
├──────────────────────────────────────────────────────────────┤
│  ② 项目 .clangd 文件（仓库内，进 git）                       │
├──────────────────────────────────────────────────────────────┤
│  ③ 用户级 .clangd 文件（~/.config/clangd/config.yaml）       │  ← 最低
└──────────────────────────────────────────────────────────────┘
```

**冲突处理规则**：

| 配置项性质 | 冲突时谁生效 | 例子 |
|---|---|---|
| 同名标量字段 | CLI > 项目 .clangd > 用户 .clangd | `--compile-commands-dir` 覆盖 `CompilationDatabase` |
| 列表追加（`Add:` 类） | 合并叠加 | `--clang-tidy-checks=...` 与 `Diagnostics.ClangTidy.Add` 同时生效 |
| 黑白名单（`Remove:` 类） | 合并叠加 | 多处 `Remove` 取并集 |

**给配置选位置的判断流程**：

```
是否随项目走？团队需要一致？
├─ 是 → .clangd 文件（进 git）
└─ 否 → 编辑器配置（VS Code settings.json / Neovim init.lua）
```

---

## 3. `.clangd` 文件加载规则

### 3.1 查找路径

clangd 启动后，对每个打开的源文件：

1. 从源文件所在目录开始**向上递归**找 `.clangd` 文件
2. 找到的多个 `.clangd` 会**叠加生效**（子目录的覆盖父目录的同名字段）
3. 还会读取用户全局配置：
   - **Linux/macOS**：`~/.config/clangd/config.yaml`
   - **Windows**：`%LOCALAPPDATA%\clangd\config.yaml`

### 3.2 启用条件

clangd 必须以 `--enable-config` 启动（**clangd 11+ 默认启用**），否则会无视所有 `.clangd` 文件。

### 3.3 多文档语法

`.clangd` 是 YAML 文件，支持用 `---` 分隔多个文档块，每个块可以有独立的 `If:` 条件：

```yaml
# 默认配置
CompileFlags:
  Add: [-std=c++23]

---
# 仅对测试目录生效
If:
  PathMatch: tests/.*
Diagnostics:
  ClangTidy:
    Remove: [cppcoreguidelines-*]
```

### 3.4 编辑器响应配置变更

修改 `.clangd` 后，clangd 默认**不会自动重启**。需要：

- **VS Code**：`Ctrl+Shift+P` → `clangd: Restart Language Server`，或在 settings.json 设 `"clangd.onConfigChanged": "restart"` 自动重启
- **Neovim**：`:LspRestart`
- **通用**：关闭并重开文件

---

## 4. 完整配置项参考

### 4.1 `CompileFlags`

控制 clangd 看到的编译命令，是最关键的配置块。

```yaml
CompileFlags:
  CompilationDatabase: .          # compile_commands.json 所在目录
                                  # 本仓库由 CMake 自动镜像到源码根，详见 §7.2
  Add:                            # 追加到所有命令的 flag
    - -std=c++23
    - -Wall
    - -DDEBUG_MODE
    - -isystem/path/to/private/include
  Remove:                         # 从命令中移除的 flag
    - -Werror
    - -Werror=*
    - -fsanitize=*                                  # IDE 不需要 sanitizer 链接
  Compiler: clang                                   # 强制把 cl.exe / g++ 当作 clang 解析
```

| 字段 | 类型 | 作用 | 典型用途 |
|---|---|---|---|
| `CompilationDatabase` | string | 指定 `compile_commands.json` 目录（相对 .clangd 路径） | 项目用 CMake、build 不在源目录 |
| `Add` | list[str] | 追加到每条编译命令的 flag | 兜底 `-std=`、补充 include 路径 |
| `Remove` | list[str] | 从每条命令中移除的 flag（支持 `*` 通配） | 屏蔽 `-Werror`、移除 sanitizer flag |
| `Compiler` | string | 把命令中的编译器改为指定值 | 强制把 `cl.exe` 当 clang-cl 处理 |

#### 适用场景

- ✅ **总是配 `Add: [-std=cxx]`**：保护新建文件、未在 DB 中的文件
- ✅ **使用 sanitizer 的项目**：`Remove: [-fsanitize=*]`，IDE 不需要也无法运行 sanitizer
- ✅ **多目标交叉编译**：用 `If.PathMatch` 配合不同的 `CompilationDatabase`

---

### 4.2 `Completion`

控制代码补全行为。

```yaml
Completion:
  AllScopes: Yes              # 全作用域补全
  HeaderInsertion: IWYU       # 头文件自动插入策略
```

| 字段 | 取值 | 默认 | 说明 |
|---|---|---|---|
| `AllScopes` | `Yes` / `No` | `No` | 即使当前作用域不可见，也建议全局符号；选中后自动补全限定名 |
| `HeaderInsertion` | `IWYU` / `NeverInsert` | `IWYU` | 是否自动插入 `#include` |

**`AllScopes: Yes` 实战效果**：

```cpp
int main() {
    cout << "hi";   // ← clangd 建议 std::cout
                    //   接受补全后变成 std::cout
                    //   并自动插入 #include <iostream>
}
```

**`HeaderInsertion: IWYU` 实战效果**：

```cpp
#include <vector>          // <vector> 内部 include 了 <iterator>
std::vector<int> v;
for (auto it = std::begin(v); ...) {}   // 用了 std::begin
//                                      ↑ clangd 提示：建议直接 #include <iterator>
```

#### 适用场景

- ✅ **强烈推荐开 `AllScopes: Yes`**：极大降低写代码时的认知负担
- ✅ **现代项目开 `HeaderInsertion: IWYU`**：避免「靠间接传递白嫖头文件」的脆弱代码
- ❌ **超大项目或代码风格不规范的旧代码库**：`AllScopes: Yes` 可能产生过多噪音

---

### 4.3 `Diagnostics`

控制诊断（红线 / 黄线 / quickfix）。

```yaml
Diagnostics:
  Suppress:                   # 屏蔽特定 clang 编译器诊断
    - unused-parameter
    - shadow
  MissingIncludes: Strict     # 报告缺失 include
  UnusedIncludes: Strict      # 报告多余 include
  UnusedIncludesAllowAngled: Yes  # 角括号 include 不报多余（用于第三方库）
  ClangTidy:
    Add:
      - modernize-*
      - performance-*
    Remove:
      - modernize-use-trailing-return-type
    CheckOptions:
      readability-identifier-naming.VariableCase: camelBack
```

| 字段 | 取值 | 说明 |
|---|---|---|
| `Suppress` | list[str] / `"*"` | 屏蔽指定 clang 警告 ID（不带 `-W` 前缀），`"*"` 全部 |
| `MissingIncludes` | `Strict` / `None` | 是否报告「用了某符号但没直接 include」 |
| `UnusedIncludes` | `Strict` / `None` | 是否报告「include 了但没用」 |
| `UnusedIncludesAllowAngled` | `Yes` / `No` | 仅检查 `"..."` 形式的 include，跳过 `<...>`（第三方库友好） |
| `ClangTidy.Add` | list[str] | 启用的 clang-tidy 检查（支持 `*` 通配组） |
| `ClangTidy.Remove` | list[str] | 关闭的具体规则 |
| `ClangTidy.CheckOptions` | map | 给具体规则传选项（如命名风格、阈值） |

#### `Suppress` 使用警告

> ⚠️ **谨慎使用 `Suppress`** —— 它**只对 IDE 屏蔽，不影响实际编译**。  
> 如果 CI 用 `-Werror`，IDE 看不到的 warning 会让构建失败，本地无法复现。

合理场景：

```yaml
# 仅对第三方库屏蔽，不影响项目代码
If:
  PathMatch: .*/vcpkg_installed/.*
Diagnostics:
  Suppress: "*"
```

#### clang-tidy 检查规则速查

| 组 | 用途 |
|---|---|
| `bugprone-*` | 易出错模式（建议**全开**） |
| `cert-*` | CERT 安全编码规范 |
| `clang-analyzer-*` | clang 静态分析器（默认就开） |
| `concurrency-*` | 并发相关问题 |
| `cppcoreguidelines-*` | C++ Core Guidelines（含部分严苛规则） |
| `google-*` | Google C++ Style |
| `hicpp-*` | High Integrity C++ |
| `llvm-*` | LLVM 项目编码规范 |
| `misc-*` | 杂项检查 |
| `modernize-*` | 现代 C++ 改写建议（建议**全开**） |
| `performance-*` | 性能优化建议（建议**全开**） |
| `portability-*` | 可移植性问题 |
| `readability-*` | 可读性改进（建议**全开**） |

**通用配方**（本项目就是这套）：

```yaml
ClangTidy:
  Add: [modernize-*, performance-*, bugprone-*, readability-*, cppcoreguidelines-*]
  Remove:
    - modernize-use-trailing-return-type
    - readability-identifier-length
    - readability-magic-numbers
    - cppcoreguidelines-avoid-magic-numbers
    - cppcoreguidelines-pro-bounds-pointer-arithmetic
    - cppcoreguidelines-pro-bounds-array-to-pointer-decay
```

---

### 4.4 `Index`

控制后台索引（驱动跳转、查找引用、全局补全）。

```yaml
Index:
  Background: Build           # 后台索引模式
  StandardLibrary: Yes        # 索引标准库（默认 Yes）
  External:                   # 引入外部预建索引
    File: /path/to/external.idx
    Server: tcp://host:port
    MountPoint: /repo/path
```

| 字段 | 取值 | 默认 | 说明 |
|---|---|---|---|
| `Background` | `Build` / `Skip` | `Build` | 后台是否索引该范围的文件 |
| `StandardLibrary` | `Yes` / `No` | `Yes` | 是否索引标准库（关闭可省内存，但失去 STL 跳转） |
| `External` | object | - | 加载预生成的索引文件或连接索引服务器 |

#### 适用场景

```yaml
# 跳过第三方代码索引（节省 CPU/磁盘）
---
If:
  PathMatch: third_party/.*
Index:
  Background: Skip
```

```yaml
# 大型 monorepo 用预建索引
Index:
  External:
    File: /shared/index/myproject.idx
```

---

### 4.5 `InlayHints`

控制编辑器内联提示。

```yaml
InlayHints:
  Enabled: Yes                # 总开关
  ParameterNames: Yes         # 形参名提示
  DeducedTypes: Yes           # auto 推导类型
  Designators: Yes            # 聚合初始化成员名
  BlockEnd: Yes               # 长块结尾标注
  DefaultArguments: Yes       # 默认参数值
  TypeNameLimit: 32           # 类型名长度上限（超过则省略）
```

| 字段 | 默认 | 效果示例 |
|---|---|---|
| `Enabled` | `Yes` | 总开关，关掉则全部不显示 |
| `ParameterNames` | `Yes` | `foo(/*x:*/ 1, /*y:*/ 2)` |
| `DeducedTypes` | `Yes` | `auto x = getValue();` 旁显示 `/* int */` |
| `Designators` | `Yes` | `Point{/*.x=*/ 1, /*.y=*/ 2}` |
| `BlockEnd` | `No` | 长函数 `}` 后显示 `// foo` |
| `DefaultArguments` | `No` | `foo(1)` 显示 `foo(1, /*y=*/ 10)` |
| `TypeNameLimit` | 32 | `DeducedTypes` 类型名超长时截断 |

#### 编辑器侧渲染

| 编辑器 | 是否需要额外开启 |
|---|---|
| VS Code | ❌ 自动渲染 |
| Neovim 0.10+ | ✅ 需 `vim.lsp.inlay_hint.enable(true)` |
| Helix | ❌ 自动渲染 |
| Emacs eglot | ✅ 需 `M-x eglot-inlay-hints-mode` |

#### 适用场景

- ✅ **开发期开全部**：信息密度高，理解代码更快
- ⚠️ **结对编程 / 演讲分享**：可能让屏幕显得拥挤，按需关闭 `DefaultArguments` 和 `Designators`

---

### 4.6 `Hover`

控制鼠标悬停提示。

```yaml
Hover:
  ShowAKA: Yes                # 显示类型别名的真实类型（Also Known As）
```

| 字段 | 默认 | 效果 |
|---|---|---|
| `ShowAKA` | `No` | 悬停 `using MyInt = int; MyInt x;` 时显示 `MyInt (aka int)` |

**适用**：项目重度使用 type alias 时建议开。

---

### 4.7 `SemanticTokens`

控制语义高亮。

```yaml
SemanticTokens:
  DisabledKinds: [Operator]
  DisabledModifiers: [Deprecated]
```

| 字段 | 类型 | 说明 |
|---|---|---|
| `DisabledKinds` | list[str] | 不发送语义 token 的种类（编辑器退化为正则高亮该种类） |
| `DisabledModifiers` | list[str] | 不发送的修饰符 |

**常见取值**：`Type` `Class` `Property` `Variable` `Function` `Method` `Macro` `Concept` `Operator` `Bracket` `Number` `Comment` `Keyword`

**绝大多数情况下不需要配置**，只有调主题颜色与 token 类型冲突时才考虑关闭某类。

---

### 4.8 `Style`

控制部分代码风格相关行为（不是格式化，格式化看 `.clang-format`）。

```yaml
Style:
  FullyQualifiedNamespaces: Yes
```

| 字段 | 默认 | 说明 |
|---|---|---|
| `FullyQualifiedNamespaces` | `No` | 自动补全的命名空间是否用完整路径（`std::vector` 而非 `vector`） |

⚠️ **注意**：`.clangd` **没有 `FallbackStyle` 字段**。设置默认格式化风格只能用 CLI 参数 `--fallback-style=Google`，或更推荐 —— 在仓库根放 `.clang-format` 文件。

---

### 4.9 `If` —— 条件配置

让任意配置块仅对特定文件生效。

```yaml
If:
  PathMatch: tests/.*\.cpp$       # 路径正则
  PathExclude: build/.*           # 排除路径
Diagnostics:
  ClangTidy:
    Remove: [cppcoreguidelines-*]
```

| 字段 | 类型 | 说明 |
|---|---|---|
| `PathMatch` | string / list | 仅对匹配的路径应用本配置块（正则） |
| `PathExclude` | string / list | 排除匹配的路径 |

**实战示例**：

```yaml
# 默认严格规则
Diagnostics:
  UnusedIncludes: Strict

---
# 测试代码放宽（gtest 大量使用 macro，UnusedIncludes 容易误报）
If:
  PathMatch: .*/tests/.*
Diagnostics:
  UnusedIncludes: None

---
# 第三方库完全不查
If:
  PathMatch: .*/(vcpkg_installed|third_party|_deps)/.*
Diagnostics:
  Suppress: "*"
Index:
  Background: Skip
```

---

## 5. clangd CLI 参数（编辑器侧）

不能写在 `.clangd` 里、必须通过编辑器传给 clangd 进程的常用参数：

| CLI 参数 | 作用 | 推荐值 |
|---|---|---|
| `--background-index` | 后台索引（默认开） | 显式声明 |
| `--clang-tidy` | 启用 clang-tidy（默认开） | 显式声明 |
| `--enable-config` | 启用 .clangd 文件（默认开） | 显式声明 |
| `--query-driver=<glob>` | 允许 clangd 查询非 clang 编译器 | **MinGW/交叉编译必配** |
| `--header-insertion-decorators` | 补全列表用圆点标记「未引入头文件」 | 推荐 |
| `--completion-style=detailed` | 重载函数显示每个签名 | 推荐 |
| `--function-arg-placeholders=true` | 补全函数生成参数占位符 | 推荐 |
| `--fallback-style=Google` | 无 .clang-format 时的格式化风格 | 见下 |
| `--pch-storage=memory` | PCH 放内存加快索引 | 内存够时推荐 |
| `--ranking-model=decision_forest` | 补全排序用决策树模型 | 推荐 |
| `--log=verbose` | 详细日志（调试用） | 定型后改 `info` |
| `-j=N` | 索引并发线程数 | CPU 核数 - 2 |

**关于 `--query-driver`**：

- 路径**必须用绝对路径**，相对 glob（`**/g++*`）在新版 clangd 可能不生效
- MinGW 项目示例：`--query-driver=F:/scoop/apps/msys2/current/ucrt64/bin/*`
- 没配会导致 `<bit>` `<cstdint>` 等 STL 头找不到，或加载错版本（误用 MSVC STL）

**关于 `--fallback-style`**：

- `.clangd` **不支持**对应字段
- 项目级风格强烈建议**用 `.clang-format` 文件**（生成方法：`clang-format -style=Google -dump-config > .clang-format`）
- 有 `.clang-format` 后 `--fallback-style` 永远不会触发

---

## 6. 与编辑器插件的配合

### 6.1 VS Code

**插件**：`llvm-vs-code-extensions.vscode-clangd`

**关键设置**（`.vscode/settings.json` 或用户 settings）：

```jsonc
{
  // 检测与 Microsoft C/C++ 扩展冲突
  "clangd.detectExtensionConflicts": true,

  // .clangd 改动后自动重启
  "clangd.onConfigChanged": "restart",

  // CLI 参数（不能写进 .clangd 的项目）
  "clangd.arguments": [
    "--query-driver=F:/scoop/apps/msys2/current/ucrt64/bin/*",
    "--completion-parse=auto",
    "--completion-style=detailed",
    "--fallback-style=Google",
    "--function-arg-placeholders=true",
    "--header-insertion-decorators",
    "--log=verbose",
    "--pch-storage=memory",
    "--pretty",
    "--ranking-model=decision_forest",
    "-j=12"
  ],

  // 仅在没有 compile_commands.json 时使用的 flags
  "clangd.fallbackFlags": ["-std=c++23"]
}
```

**禁用 Microsoft C/C++ 扩展的 IntelliSense**（避免冲突）：

```jsonc
{
  "C_Cpp.intelliSenseEngine": "disabled"
}
```

---

### 6.2 Neovim

**插件**：`nvim-lspconfig`（社区主流）或 Neovim 0.11+ 内置 `vim.lsp.config`

**`nvim-lspconfig` 写法**：

```lua
require('lspconfig').clangd.setup({
  cmd = {
    'clangd',
    '--query-driver=F:/scoop/apps/msys2/current/ucrt64/bin/*',
    '--completion-parse=auto',
    '--completion-style=detailed',
    '--fallback-style=Google',
    '--function-arg-placeholders=true',
    '--header-insertion-decorators',
    '--log=verbose',
    '--pch-storage=memory',
    '--pretty',
    '--ranking-model=decision_forest',
    '-j=12',
  },
  init_options = {
    fallbackFlags = { '-std=c++23' },
  },
  on_attach = function(client, bufnr)
    vim.lsp.inlay_hint.enable(true, { bufnr = bufnr })  -- 启用 InlayHints 渲染
  end,
})
```

**Neovim 0.11+ 原生写法**：

```lua
vim.lsp.config('clangd', {
  cmd = { 'clangd', '--query-driver=...', ... },
  root_markers = { '.clangd', 'compile_commands.json', '.git' },
  filetypes = { 'c', 'cpp', 'objc', 'objcpp' },
})
vim.lsp.enable('clangd')
```

---

### 6.3 其他编辑器

| 编辑器 | 插件 | CLI 参数配置位置 |
|---|---|---|
| Emacs (eglot) | 内置 | `eglot-server-programs` 中的 cmd |
| Emacs (lsp-mode) | `lsp-mode` | `lsp-clients-clangd-args` |
| Sublime Text | LSP-clangd | Package settings 的 `command` |
| Helix | 内置 | `languages.toml` 的 `language-server.clangd` |
| CLion | 内置（fork） | Settings → Languages & Frameworks → C/C++ → Clangd |

**通用原则**：所有编辑器的「clangd 参数配置」最终都对应 clangd 进程的 CLI 参数，写法不同但含义相同。

---

## 7. 常见场景配置示例

### 7.1 Windows + MinGW（本项目场景）

```yaml
CompileFlags:
  CompilationDatabase: .   # 见 §7.2，本仓库已采用"自动镜像到源码根"方案
  Add: [-std=c++23, -Wall, -Wextra, -Wpedantic]
```

**额外**：VS Code `clangd.arguments` 必须配 `--query-driver=F:/scoop/apps/msys2/current/ucrt64/bin/*`。

---

### 7.2 多 Preset 切换（本仓库的解决方案）

`.clangd` 硬编码某个 preset 的 build 目录不灵活——切到别的 preset 后 clangd 还在读旧
目录。本仓库采用 **CMake 自动镜像** 方案：顶层 `CMakeLists.txt` 注册了一个 ALL custom
target `mcpp_link_compile_commands`，每次 build 时把
`build/<active-preset>/compile_commands.json` 复制到源码根目录：

```cmake
add_custom_target(mcpp_link_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    BYPRODUCTS "${CMAKE_SOURCE_DIR}/compile_commands.json")
```

`.clangd` 配 `CompilationDatabase: .`，clangd 直接从源码根读 DB。结果：**切 preset
后只需要重跑一次 `cmake --build --preset <new>`，不需要手动改 `.clangd`**。
多配置生成器（VS）不产 `compile_commands.json`，`if(NOT _mcpp_is_multi_config)`
门控会跳过此 target——VS 用户走 IntelliSense 或装 LLVM 插件。

#### 备选方案：把 .clangd 加进 .gitignore

如果不喜欢"复制到源码根"方案（例如担心源码根被污染），也可以把 `.clangd` 加进
`.gitignore`，每人本地维护一份 `CompilationDatabase: build/<my-preset>` 的硬编码版本。
本仓库未采用此方案——成本更高且无法跨设备同步。

#### 备选方案：用环境变量（少见）

CLI 参数 `--compile-commands-dir="$PRESET_BUILD_DIR"`，shell 启动 clangd 前 export 变量。

---

### 7.3 屏蔽第三方库诊断

```yaml
---
If:
  PathMatch: .*/(vcpkg_installed|third_party|_deps|/usr/include)/.*
Diagnostics:
  Suppress: "*"
  ClangTidy:
    Remove: ["*"]
Index:
  Background: Skip
```

---

### 7.4 测试代码放宽规则

```yaml
---
If:
  PathMatch: .*/tests/.*
Diagnostics:
  UnusedIncludes: None         # gtest 用大量 macro，容易误报
  ClangTidy:
    Remove:
      - cppcoreguidelines-avoid-non-const-global-variables  # gtest fixture
      - readability-function-cognitive-complexity            # 测试逻辑常常很长
```

---

### 7.5 强制特定命名规范

```yaml
Diagnostics:
  ClangTidy:
    Add: [readability-identifier-naming]
    CheckOptions:
      readability-identifier-naming.ClassCase: CamelCase
      readability-identifier-naming.FunctionCase: camelBack
      readability-identifier-naming.VariableCase: lower_case
      readability-identifier-naming.ConstantCase: UPPER_CASE
      readability-identifier-naming.PrivateMemberSuffix: _
```

---

### 7.6 性能调优（巨型项目）

```yaml
Index:
  Background: Build
  StandardLibrary: No          # 不索引 STL（损失 STL 跳转，节省内存）

---
If:
  PathMatch: .*/(generated|proto|grpc)/.*
Index:
  Background: Skip             # 跳过自动生成的代码
```

CLI 配合：`--pch-storage=disk`（内存吃紧时）、`-j=4`（限制并发）。

---

## 8. 与 `.clang-format` / `.clang-tidy` 的关系

`.clangd` 是「**语言服务行为**」配置，clang 工具链中其他配置文件各有职责：

| 文件 | 作用 | 由谁读 | 是否进 git |
|---|---|---|---|
| `.clangd` | clangd 行为配置 | clangd | ✅ 推荐 |
| `.clang-format` | 代码格式化规则 | clang-format / clangd | ✅ 必须 |
| `.clang-tidy` | clang-tidy 规则 | clang-tidy / clangd | ✅ 推荐 |
| `compile_commands.json` | 编译命令数据库 | clangd / clang-tidy | ❌ 自动生成 |
| `.clangd-tidy` | （不存在） | - | - |

### 8.1 `.clang-format` —— 代码格式化

```yaml
# .clang-format
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
AllowShortFunctionsOnASingleLine: Inline
```

**生成方法**：

```bash
clang-format -style=Google -dump-config > .clang-format
```

**与 `.clangd` 的关系**：clangd 调用 clang-format 时自动读取 `.clang-format`。`.clangd` 没有任何与代码格式化相关的字段。

---

### 8.2 `.clang-tidy` —— 独立的 clang-tidy 配置

```yaml
# .clang-tidy
Checks: 'modernize-*,bugprone-*,-modernize-use-trailing-return-type'
WarningsAsErrors: ''
HeaderFilterRegex: '^(?!.*third_party).*$'
CheckOptions:
  - key: readability-identifier-length.MinimumVariableNameLength
    value: 2
```

**与 `.clangd.Diagnostics.ClangTidy` 的关系**：

- 命令行 `clang-tidy` 工具读 `.clang-tidy`
- clangd 内置的 clang-tidy **同时读 `.clang-tidy` 和 `.clangd` 中的 `Diagnostics.ClangTidy`**
- 两者**合并叠加**

**推荐策略**：

- **小项目**：只在 `.clangd` 里配，避免冗余
- **大项目（CI 也跑 clang-tidy）**：把 checks 写在 `.clang-tidy`（命令行和 IDE 共用），`.clangd` 只做 IDE 特定的微调

---

## 9. 排查清单（FAQ）

### Q1：clangd 找不到第三方库头文件（如 `<gtest/gtest.h>`）

**检查顺序**：

1. `compile_commands.json` 是否存在且包含该文件？
   ```bash
   grep "your_file.cpp" build/<preset>/compile_commands.json
   ```
2. `.clangd` 的 `CompilationDatabase` 是否指向正确的 build 目录？
3. VS Code `clangd.arguments` 中 `--compile-commands-dir` 是否覆盖了 `.clangd`？
4. compile_commands.json 中的 `-I` / `-isystem` 路径是否实际存在？
5. 重启 clangd（`Ctrl+Shift+P` → `clangd: Restart Language Server`）
6. 清理 clangd 缓存：删除 `%LOCALAPPDATA%\clangd\index\` 后重启编辑器

---

### Q2：clangd 找不到标准库头文件（如 `<bit>` `<cstdint>`）

通常是 **`--query-driver` 没配或没生效**。

```jsonc
"clangd.arguments": [
  "--query-driver=F:/scoop/apps/msys2/current/ucrt64/bin/*"
]
```

验证：VS Code → 输出面板 → 选 `clangd`，看启动日志：

```
I[xx:xx:xx.xxx] Compiler driver F:\scoop\apps\msys2\current\ucrt64\bin\g++.exe is allowed by query-driver
```

如果出现 `is not allowed by query-driver`，路径未匹配。

---

### Q3：clang-tidy 检查没生效

1. CLI 是否带 `--clang-tidy`（默认开，但有些发行版关了）
2. `.clangd` 的 `Diagnostics.ClangTidy.Add` 是否写对（注意 `*` 是组通配，不是正则）
3. 项目里是否有 `.clang-tidy` 文件覆盖了 `.clangd` 的设置
4. clangd 输出日志中是否有 `clang-tidy: enabled checks: ...`

---

### Q4：InlayHints 不显示

- **VS Code**：检查 settings.json 中 `editor.inlayHints.enabled` 是否为 `on`
- **Neovim**：缺少 `vim.lsp.inlay_hint.enable(true)` 调用
- 确认 `.clangd` 中 `InlayHints.Enabled: Yes`

---

### Q5：`.clangd` 修改后没生效

1. 重启 clangd（`Ctrl+Shift+P` → `clangd: Restart Language Server`）
2. 配置 `"clangd.onConfigChanged": "restart"` 让它自动重启
3. YAML 缩进/语法是否正确（VS Code 中 `.clangd` 文件不会语法高亮，建议用 yamllint 检查）

---

### Q6：补全很慢 / clangd 占用大量 CPU

1. 缩小索引范围：`If: { PathMatch: third_party/.* } Index: { Background: Skip }`
2. 限制并发：CLI 加 `-j=4`
3. PCH 存内存：`--pch-storage=memory`（前提是内存够）
4. 关闭 STL 索引：`Index: { StandardLibrary: No }`（损失 STL 跳转）

---

### Q7：VS Code 中红线很多但实际能编译

1. `compile_commands.json` 中的 flag 与实际编译时不一致
2. clangd 平台与编译器平台不匹配（如 MSVC clangd 解析 MinGW 项目，未配 `--query-driver`）
3. `-D` 宏定义缺失，看实际编译时的 -D 是否在 compile_commands.json 中

---

## 10. 本项目当前配置回顾

```yaml
CompileFlags:
  CompilationDatabase: .
  Add: [-std=c++23, -Wall, -Wextra, -Wpedantic]

Completion:
  AllScopes: Yes              # 全局补全 + 自动补 std::
  HeaderInsertion: IWYU       # 自动 #include

Diagnostics:
  MissingIncludes: Strict     # 缺 include 警告
  UnusedIncludes: Strict      # 多余 include 警告
  ClangTidy:
    Add: [modernize-*, performance-*, bugprone-*, readability-*, cppcoreguidelines-*]
    Remove: [
      modernize-use-trailing-return-type,
      readability-identifier-length,
      readability-magic-numbers,
      cppcoreguidelines-avoid-magic-numbers,
      cppcoreguidelines-pro-bounds-pointer-arithmetic,
      cppcoreguidelines-pro-bounds-array-to-pointer-decay,
    ]

Index:
  Background: Build           # 后台索引

InlayHints:
  Enabled: Yes
  ParameterNames: Yes
  DeducedTypes: Yes
  Designators: Yes
  BlockEnd: Yes
  DefaultArguments: Yes
```

**配套 VS Code `clangd.arguments`**：

```jsonc
[
  "--query-driver=F:/scoop/apps/msys2/current/ucrt64/bin/*",
  "--completion-parse=auto",
  "--completion-style=detailed",
  "--fallback-style=Google",
  "--function-arg-placeholders=true",
  "--header-insertion-decorators",
  "--log=verbose",
  "--pch-storage=memory",
  "--pretty",
  "--ranking-model=decision_forest",
  "-j=12"
]
```

**当前未启用但可考虑的扩展**：

- 在仓库根放 `.clang-format` 取代 `--fallback-style`
- 用 `If.PathMatch` 对 `tests/` 放宽 `UnusedIncludes`
- 用 `If.PathMatch` 对 `vcpkg_installed/` 全局 Suppress
- 切换 preset 频繁时改用 CMake `add_custom_target` 自动复制 compile_commands.json

---

## 参考资料

- clangd 官方配置文档：<https://clangd.llvm.org/config>
- clang-tidy 检查列表：<https://clang.llvm.org/extra/clang-tidy/checks/list.html>
- clang-format 选项：<https://clang.llvm.org/docs/ClangFormatStyleOptions.html>
- LSP 协议规范：<https://microsoft.github.io/language-server-protocol/>
