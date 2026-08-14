# clang-format 与 clang-tidy

这两个工具与 clangd 协作，但配置职责不同。版本支持和项目既有规则优先于下列示例。

## 目录

- [设计 `.clang-format`](#设计-clang-format)
- [设计 `.clang-tidy`](#设计-clang-tidy)
- [`.clangd` 与 `.clang-tidy` 的组合语义](#clangd-与-clang-tidy-的组合语义)
- [分层与例外](#分层与例外)
- [修复与 CI](#修复与-ci)
- [上游依据](#上游依据)

## 设计 `.clang-format`

先选择最接近团队习惯的预设，再只覆盖已达成共识的差异：

```yaml
---
Language: Cpp
BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 100
DerivePointerAlignment: false
PointerAlignment: Left
```

原则：

- 不提交 `clang-format -dump-config` 的全部默认项；默认项越多，升级越困难。
- `Standard` 只影响格式化器解析，不设置项目的实际 C++ 标准。现代语法无法解析时才显式指定，并核对当前版本支持的取值。
- `#include` 分组依赖项目头文件约定。先对代表性文件执行只读检查，避免把主头文件、生成头文件、模块或第三方聚合头文件（umbrella header）排错。
- 多语言仓库用 `---` + `Language` 分段；子目录需要少量差异时使用 `BasedOnStyle: InheritParentConfig`。
- `// clang-format off/on` 只用于表格、宏或生成片段等局部例外，并写明原因；不要用它逃避团队风格。

先通过 `clang-format --help` 确认当前版本支持所用选项。支持 `--dry-run` 和 `--Werror` 时执行：

```text
clang-format --version
clang-format -style=file -dump-config
clang-format --dry-run --Werror <tracked-cpp-files...>
```

旧版本不支持只读检查选项时，将 `clang-format -style=file <file>` 的输出与源文件比较。让 CI 显式使用团队支持的版本；格式化版本不同可能导致同一配置产生不同 diff。

## 设计 `.clang-tidy`

从高信噪比检查开始，不要默认启用 `*`：

```yaml
Checks: >
  -*,
  bugprone-*,
  performance-*,
  portability-*,
  modernize-use-nullptr,
  modernize-use-override
WarningsAsErrors: ''
HeaderFilterRegex: '(^|.*/)(src|include)/.*'
FormatStyle: file
```

这只是起点。检查组会随 LLVM 版本增加新规则，升级后必须重新列出有效检查并评估新增诊断。安全关键、嵌入式、异常禁用、Qt、CUDA 或 C 兼容项目需要不同的检查组合。

先通过 `clang-tidy --help` 确认当前版本支持所用选项。当前版本支持时，使用以下命令理解生效配置：

```text
clang-tidy --version
clang-tidy --verify-config
clang-tidy -list-checks -p <build-dir>
clang-tidy --dump-config -p <build-dir> <source-file>
clang-tidy --explain-config -p <build-dir> <source-file>
```

旧版本缺少 `--verify-config` 或 `--explain-config` 时，至少使用 `--dump-config`、`-list-checks` 和单文件实际运行验证。`-p` 指向含编译命令数据库的目录。检查单个代表性翻译单元后，再通过项目包装目标或 `run-clang-tidy` 扩大范围。

`HeaderFilterRegex` 只控制输出哪些头文件中的诊断，不会阻止 clang-tidy 解析这些头文件，也不会减少其分析成本；主翻译单元中的诊断始终输出。要限制静态检查输入，应筛选传给 clang-tidy 或 `run-clang-tidy` 的翻译单元，而不是把 `HeaderFilterRegex` 当成输入排除规则。

## `.clangd` 与 `.clang-tidy` 的组合语义

clangd 会读取项目 `.clang-tidy`，并把 `.clangd` 中的增量规则应用到它：

```yaml
Diagnostics:
  ClangTidy:
    Add: [performance-*]
    Remove: [performance-no-int-to-ptr]
    FastCheckFilter: Strict
```

- clangd 会合并 `.clang-tidy` 与 `.clangd` 中的 clang-tidy 设置；冲突时 `.clangd` 优先，同一 clangd 配置中的 `Remove` 优先于 `Add`。
- `FastCheckFilter: Strict` 在当前 clangd 中是默认值；仅在核对目标版本后，为明确记录团队意图或调整策略而显式设置。某些检查不能或不适合在 clangd 中运行。
- 团队规则应集中在 `.clang-tidy`，便于 CLI 和 CI 复用；`.clangd` 只保留编辑器性能或路径范围所需的差异。
- `.clangd` 的 `Diagnostics.ClangTidy.CheckOptions` 使用映射；`.clang-tidy` 接受的表示形式会随 LLVM 版本变化，当前文档使用映射，旧版本还可能接受由 `key`/`value` 组成的序列。不要在两种文件间盲目复制；用目标版本的文档、`clang-tidy --dump-config` 和受支持的 `--verify-config` 确认最终形态。

## 分层与例外

子目录 `.clang-tidy` 要继承父配置时显式设置：

```yaml
InheritParentConfig: true
Checks: '-readability-function-size'
```

对第三方代码，优先从传给 clang-tidy 或 `run-clang-tidy` 的翻译单元列表中排除，而不是在源代码中散布 `NOLINT`。需要控制头文件诊断输出时，再使用 `HeaderFilterRegex` 或受支持版本中的 `ExcludeHeaderFilterRegex`。对项目代码中的真实局部误报：

1. 先调整 check option 或缩小检查范围；
2. 再使用带具体检查名和理由的 `NOLINT`/`NOLINTNEXTLINE`；
3. 避免无检查名的全局抑制；
4. 保持 `NOLINTBEGIN` 与 `NOLINTEND` 参数完全匹配。

## 修复与 CI

- 先以只读模式运行，分类现有债务和误报。
- `--fix` 只对可恢复工作区和限定文件运行；修复后执行 clang-format、构建和测试。
- 开发期可让 `WarningsAsErrors` 为空，在 CI 通过 `--warnings-as-errors=*` 或选定组执行门禁。
- CI 必须使用同一编译命令数据库生成方式和兼容的 clang-tidy/clang-format 版本。
- 不要用仅扫描改动行的脚本声称完成全仓静态分析；头文件和跨翻译单元影响需要单独策略。

## 上游依据

- clang-format 配置：<https://clang.llvm.org/docs/ClangFormatStyleOptions.html>
- clang-format 工具：<https://clang.llvm.org/docs/ClangFormat.html>
- clang-tidy 配置与命令：<https://clang.llvm.org/extra/clang-tidy/index.html>
- clang-tidy 编辑器集成：<https://clang.llvm.org/extra/clang-tidy/Integrations.html>
- clangd 配置中的 clang-tidy：<https://clangd.llvm.org/config#clangtidy>
