---
name: configure-clangd
description: 为现有 C/C++ 项目设计、生成、审查和排查 clangd 工具链配置。用于 `.clangd`、`compile_commands.json`、`compile_flags.txt`、`.clang-format`、`.clang-tidy`、编辑器启动参数、`--query-driver`、交叉编译、索引性能以及本地与 CI 一致性；当用户反馈 clangd 找不到头文件、编辑器诊断与实际构建不一致、补全或跳转异常、格式化不一致、clang-tidy 未生效时使用。不要用于仅修改 C/C++ 业务代码，或仅配置不使用 clangd 的 IDE IntelliSense。
---

# 配置 clangd 工具链

把实际构建命令作为 clangd 的事实来源，先恢复准确解析，再逐层启用格式化、静态检查和体验增强。不要用大而全的模板掩盖错误的编译命令数据库（compilation database）。

## 遵循核心原则

- 让构建系统生成 `compile_commands.json`；不要手工维护生成文件，也不要在 `.clangd` 中重复项目已有的 `-I`、`-D`、`-std` 和警告选项。
- 区分职责：编译命令数据库描述如何解析翻译单元，`.clangd` 调整语言服务器，`.clang-format` 规定排版，`.clang-tidy` 规定静态检查。
- 先读取项目约定、现有配置和构建入口。修改配置时保留有意的本地差异，不覆盖无关更改。
- 只提交团队共享且可移植的设置。把编译器绝对路径、`--query-driver`、并发数和日志级别留在用户或编辑器配置中。
- 以实际安装版本为准。生成配置前检查 `clangd`、`clang-format` 和 `clang-tidy` 版本；不要写入该版本不识别的字段或检查项。
- 逐步启用诊断。先消除解析错误，再增加 Include Cleaner 和 clang-tidy；不要一次开启所有检查组后批量抑制结果。

## 执行工作流

### 1. 明确目标与变更边界

确认用户要求的是创建配置、审查现有配置、诊断故障，还是同时配置编辑器和 CI。只在请求包含实施时修改文件；单纯审查或诊断时保持只读。

检查仓库说明和工作区状态，并识别：

- 构建系统、生成器、CMake Preset 或构建配置（profile）；
- C/C++ 标准、编译器驱动程序（compiler driver）、目标平台和标准库；
- 源码、生成代码、测试和第三方目录；
- 现有 `.clangd`、`.clang-format`、`.clang-tidy` 与编辑器设置；
- 编译命令数据库的生成位置、刷新方式和忽略规则；
- 本地、编辑器和 CI 使用的 LLVM 工具版本。

### 2. 建立准确的编译命令数据库

优先解决 `compile_commands.json`。读取其中代表性翻译单元，核对工作目录、编译器、目标三元组（target triple）、语言标准、宏和头文件搜索路径是否与成功构建一致。

- CMake + Ninja/Makefiles：启用 `CMAKE_EXPORT_COMPILE_COMMANDS`，使用项目既有的配置（configure）入口生成编译命令数据库。正式构建使用不导出数据库的生成器时，再建立语义一致的工具专用配置。
- 其他构建系统：优先使用其原生导出器；没有原生支持时再考虑 Bear 等构建捕获工具。
- 所有文件确实共享相同参数的简单项目：可使用 `compile_flags.txt`，并说明它不能向后台索引提供完整源文件清单。
- 多配置、多 Preset 或多目标项目：明确一个当前使用的编译命令数据库，或按源码范围拆分配置；不要无条件合并互相冲突的命令。

clangd 默认会从源文件目录向父目录搜索编译命令数据库，并检查名为 `build/` 的子目录。仅在默认发现不可靠时设置 `CompileFlags.CompilationDatabase` 或编辑器的 `--compile-commands-dir`。

不要把 `.clangd` 的 `CompileFlags.Add` 描述成“只对编译命令数据库外的文件兜底”：它会追加到所有编译命令。若数据库内命令正确，额外的 `-std`、`-D` 或警告选项可能改变语义，或因 GCC 风格与 MSVC 风格选项混用产生无关诊断。

### 3. 生成最小 `.clangd`

只添加有明确需求的配置：

- 用 `CompilationDatabase` 修正编译命令数据库位置；默认 `Ancestors` 能工作时省略。
- 用 `Add` 或 `Remove` 修复已复现的命令兼容问题，不复制构建系统的策略。
- 仅在编译器驱动程序识别错误时设置 `Compiler`；它会影响参数解析和目标三元组推断。
- 默认保留后台索引；需要跳过特定翻译单元时使用条件片段，并说明这不会阻止其头文件被其他翻译单元索引，也不会禁用已打开文件的动态索引。
- 在解析正确后再按目标 clangd 版本确认 Include Cleaner 默认值，并显式启用需要的 `MissingIncludes`、`UnusedIncludes` 与 clang-tidy 策略。
- 将内联提示（inlay hints）、全作用域补全等视为体验选项，不与正确性配置混在一起。

创建或修改具体片段时读取 [configuration-patterns.md](references/configuration-patterns.md)。

### 4. 处理编译器驱动程序与系统头文件

当 clangd 找不到标准库头文件、推断出错误的目标平台或 ABI，或把 GCC、MinGW、交叉编译命令按本机 Clang/MSVC 解释时：

1. 从 clangd 日志确认最终编译命令和 `argv[0]`。
2. 优先让编译命令数据库记录绝对路径、带目标信息的真实编译器驱动程序。
3. 必要时在编辑器启动参数中加入范围严格受限的 `--query-driver` 白名单。
4. 只有确认需要采用被查询驱动程序的编译器内建头文件（compiler built-in headers）时，才设置 `BuiltinHeaders: QueryDriver`；查询非 Clang 驱动程序所得的内建头文件可能产生误诊断。

`--query-driver` 会授权 clangd 执行与 glob 匹配、且出现在编译命令 `argv[0]` 中的编译器驱动程序。不要使用覆盖整个磁盘或任意可写目录的宽泛通配符，也不要把个人绝对路径提交到共享 `.clangd`。

配置用户级文件、编辑器启动命令或格式化提供程序时，读取 [editor-clients.md](references/editor-clients.md)。

### 5. 配置格式化与静态检查

需要创建或调整 `.clang-format`、`.clang-tidy` 时读取 [format-and-tidy.md](references/format-and-tidy.md)。遵循以下边界：

- `.clang-format` 从最接近团队风格的 `BasedOnStyle` 起步，只保留有意覆盖项；不要提交完整 `-dump-config` 输出。
- 格式选项随 LLVM 演进。使用项目支持的最低版本验证配置，并让 CI 与开发环境采用兼容版本。
- 把项目级 clang-tidy 规则集中在 `.clang-tidy`。clangd 会合并 `.clang-tidy` 与 `.clangd` 的 `Diagnostics.ClangTidy`；冲突时 `.clangd` 优先，并且同一 clangd 配置中的 `Remove` 优先于 `Add`。
- 从少量高信噪比检查开始，用代码样本评估误报和修复安全性，再扩展检查组。
- 将 `WarningsAsErrors` 通常留给 CI 命令行；编辑器保持可迭代，但不得与 CI 使用不同的检查集合而不说明。
- `--fix` 前保持工作区可恢复，先限定文件范围并审查 diff；不要对整个仓库盲目应用修复。

### 6. 验证实际行为

至少选择一个能代表主工具链、依赖和生成头文件的翻译单元。先检查版本和帮助文本，再只使用当前版本支持的选项：

```text
clangd --version
clangd --help
clangd --check=<source-file> --log=verbose
clang-format --version
clang-format --help
clang-tidy --version
clang-tidy --help
```

若帮助文本确认支持，再运行 `clang-format --dry-run --Werror <files...>` 和 `clang-tidy --verify-config`。旧版本缺少这些选项时，分别比较 `clang-format -style=file` 的输出，并结合 `clang-tidy --dump-config`、`-list-checks` 与单文件 `clang-tidy -p <build-dir> <source-file>` 验证实际行为。

再运行项目原有的配置、构建、`format-check`、`tidy-check` 或测试入口。不要用手写命令绕过仓库规定的 CMake Preset、工具链文件或包装脚本。

验证时确认：

- clangd 日志加载了预期编译命令数据库和配置片段；
- 最终命令包含正确的目标三元组、语言标准、宏和头文件搜索路径；
- 项目内与标准库头可解析，跳转和补全指向正确工具链；
- `.clang-format` 和 `.clang-tidy` 没有未知选项；
- 编辑器实时诊断与命令行/CI 的差异有明确原因；
- 路径条件对直接打开文件、直接翻译单元和被包含头文件的作用符合预期，没有把第三方代码或生成目录误纳入不必要的诊断与静态检查范围。

### 7. 按证据排障

遇到问题时读取 [troubleshooting.md](references/troubleshooting.md)，按“配置解析 → 编译命令数据库发现 → 最终命令 → 编译器驱动程序查询 → 生成文件 → clang-tidy/clang-format → 性能”的顺序排查。不要先删除缓存或加入抑制规则；这些操作可能隐藏根因。

## 交付结果

完成后说明：

- 检测到的构建系统、活动工具链和 LLVM 版本；
- 新增或修改了哪些共享配置，哪些步骤必须由用户在编辑器本地完成；
- 编译命令数据库如何生成和刷新；
- 实际运行的验证命令及结果；
- 尚未启用的严格检查、版本限制或已知误报。

若只验证了单个翻译单元，明确说明覆盖范围，不得声称整个仓库无诊断。
