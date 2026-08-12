# clangd 配置模式

仅复制与当前项目相符的片段。先确认安装版本支持字段，再用 `clangd --check=<file> --log=verbose` 验证。

## 目录

- [最小项目配置](#最小项目配置)
- [Include Cleaner](#include-cleaner)
- [按路径拆分](#按路径拆分)
- [多个构建目录](#多个构建目录)
- [GCC、MinGW 与交叉编译器](#gccmingw-与交叉编译器)
- [clang-cl 与 MSVC 风格参数](#clang-cl-与-msvc-风格参数)
- [仅有统一参数的简单项目](#仅有统一参数的简单项目)
- [体验选项](#体验选项)
- [上游依据](#上游依据)

## 最小项目配置

编译命令数据库位于固定的 `build/` 时：

```yaml
CompileFlags:
  CompilationDatabase: build
```

如果编译命令数据库能由默认的 `Ancestors` 搜索找到，则连 `CompileFlags` 也可以省略。后台索引的默认值是 `Build`；只有需要明确记录团队意图时才写出 `Index.Background: Build`。不要默认添加 `-std`、`-I`、`-D` 或警告标志；这些应来自实际构建命令。

## Include Cleaner

在编译命令准确、项目能无误解析后启用：

```yaml
Diagnostics:
  MissingIncludes: Strict
  UnusedIncludes: Strict
```

对聚合头文件（umbrella header）、宏密集框架和平台头文件出现误报时，先使用 IWYU pragma 或当前 clangd 支持的 `Diagnostics.Includes` 过滤能力，不要全局关闭全部诊断。

## 按路径拆分

`.clangd` 支持以 `---` 分隔多个配置片段。项目配置中的 `PathMatch` 相对配置文件目录匹配，使用正斜杠：

```yaml
If:
  PathMatch: (third_party|vendor|generated)/.*
Index:
  Background: Skip
Diagnostics:
  Suppress: '*'
```

只在仓库确实包含并直接打开这些文件时加入排除片段。依赖的系统头文件通常不需要复制到项目树或全局抑制。

## 多个构建目录

优先顺序：

1. 让用户或项目选择一个当前使用的 CMake Preset 或构建配置，并刷新对应编译命令数据库。
2. 编译命令数据库位置稳定时，在 `.clangd` 指向该目录。
3. 位置因开发者而异时，在编辑器配置中传 `--compile-commands-dir=<dir>`。
4. 团队已经有可靠机制时，将当前使用的编译命令数据库软链接或复制到约定位置，并把生成物加入忽略规则。
5. 只有源码树能按路径明确映射到不同构建目标时，才用多个 `If.PathMatch` + `CompilationDatabase` 片段。

不要直接编辑生成的 JSON。合并编译命令数据库时必须定义同一源文件出现多条命令时的选择规则，否则 clangd 实际采用的配置不可预测。

## GCC、MinGW 与交叉编译器

尽量让编译命令数据库中的 `argv[0]` 是真实编译器驱动程序的绝对路径，并携带目标前缀，例如 `aarch64-linux-gnu-g++` 或 `x86_64-w64-mingw32-g++`。clangd 可由驱动程序名称推断部分目标信息。

需要查询编译器驱动程序的默认目标三元组和系统头文件搜索路径时，在用户的编辑器启动参数中配置范围严格受限的白名单：

```text
--query-driver=/opt/toolchains/aarch64/bin/aarch64-linux-gnu-*
```

Windows 示例也应绑定到具体工具链目录，而不是 `C:/**`：

```text
--query-driver=D:/msys64/ucrt64/bin/*-g++.exe,D:/msys64/ucrt64/bin/*-gcc.exe
```

路径是用户环境信息，不应原样进入团队共享配置。查询行为会执行匹配程序，因此目录必须可信且不可由低权限来源任意写入。

## clang-cl 与 MSVC 风格参数

clangd 通常能解析 clang-cl 命令。若日志明确显示输出/PDB 参数破坏重写后的语法检查命令，可有针对性地移除：

```yaml
CompileFlags:
  Remove:
    - /Fo*
    - /Fd*
    - /FS
```

不要把这段无条件放进所有项目。先从最终命令和 clangd 报错确认具体参数；不要删除影响宏、ABI、语言模式或头文件搜索路径的 MSVC 参数。

## 仅有统一参数的简单项目

`compile_flags.txt` 每行一个参数，例如：

```text
-std=c++20
-Iinclude
-Wall
```

它适合所有文件参数一致的小项目。`compile_commands.json` 存在时它会被忽略，而且由于不列出项目源文件，后台索引不能获得完整的翻译单元集合。

## 体验选项

正确性稳定后再按团队偏好加入。当前 clangd 通常已默认启用 `AllScopes: Yes` 和 `HeaderInsertion: IWYU`；只有核对目标版本并需要明确记录团队意图时才显式写出：

```yaml
Completion:
  AllScopes: Yes
  HeaderInsertion: IWYU

InlayHints:
  Enabled: Yes
  ParameterNames: Yes
  DeducedTypes: Yes
```

体验设置不应成为排查解析问题的前提。大型项目先测量索引时间和内存，再决定并发、标准库索引、外部索引或跳过范围。

## 上游依据

- clangd 配置：<https://clangd.llvm.org/config>
- clangd 项目设置：<https://clangd.llvm.org/installation#project-setup>
- 编译命令设计：<https://clangd.llvm.org/design/compile-commands>
- CMake 编译命令数据库：<https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html>
