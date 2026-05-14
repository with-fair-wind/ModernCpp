# CLAUDE.md

本文件为 Claude Code（claude.ai/code）在本仓库中工作时提供指引。

## 项目骨架

Modern C++ 学习仓库。`modules/NN_shortname/` 下的每个"模块"都是自包含单元，包含
`demos/*.cpp`、可选的 `tests/*.cpp`、文档以及一份小巧的 `CMakeLists.txt`。顶层
`CMakeLists.txt` 会自动 glob `modules/*/CMakeLists.txt` —— **新增模块无需改动顶层
文件**，直接创建 `modules/NN_*/CMakeLists.txt` 即可。

基线：C++23、CMake ≥ 3.25、Ninja（Linux / clang-cl）或 VS 17 2022 多配置（MSVC）。
GoogleTest 是唯一的第三方依赖，通过 vcpkg 或 Conan 拉取（绝不使用 FetchContent）。

## 构建 / 测试（仅用 preset）

仓库由 preset 驱动。**绝不要**直接调用 `cmake -S . -B ...`，必须走 preset 才能保证
triplet 与 toolchain 配线正确。

```bash
cmake --list-presets                               # 查看可用 preset
cmake --preset gcc-relwithdebinfo                  # configure
cmake --build --preset gcc-relwithdebinfo --parallel
ctest --preset gcc-relwithdebinfo                  # ctest preset 与之同名
```

Preset 家族：`gcc-*`、`clang-*`（仅 Linux —— 通过 `hostSystemName != Windows` 门控），
`clang-cl-*` 与 `msvc` / `msvc-*`（仅 Windows）。Conan 平行版本：`*-conan`。Build-type：
`debug` / `release` / `relwithdebinfo` / `minsizerel`。MSVC 是多配置：用 `msvc` configure
一次，再用 `msvc-<config>` 构建/测试。

跑单个测试：`ctest --preset gcc-debug -R '^01_basics\.'`（`gtest_discover_tests` 会给
测试名加上 `<module>.` 前缀）。或直接执行可执行：
`./build/gcc-debug/bin/test_basics --gtest_filter=...`。

顶层开关：`-DMCPP_BUILD_DEMOS=OFF`（不构建 demo 可执行）、`-DMCPP_BUILD_TESTS=OFF`
（跳过 gtest 依赖）、`-DMCPP_ENABLE_SANITIZERS=ON`（GCC/Clang Unix 驱动启用 ASan +
UBSan；MSVC ABI 下的 cl.exe 与 clang-cl 都启用 ASan，UBSan 不可用；仅 Debug/RelWithDebInfo
生效）、`-DMCPP_WARNINGS_AS_ERRORS=ON`。

格式化：`cmake --build --preset <p> --target format`（原地修复）/ `--target format-check`
（dry-run，与 CI 行为一致）。clang-format 不在 PATH 时这两个 target 不会注册。

## 模块辅助函数 —— 你需要的全部 API

每个模块的 `CMakeLists.txt` 只调用两个函数，定义在 `cmake/ModuleHelpers.cmake` 中：

```cmake
mcpp_add_demo(NAME my_demo SOURCES demos/my_demo.cpp
 [STANDARD 23] [LINK_LIBS <targets...>]
 [INCLUDE_DIRS <dirs...>] [PCH <headers...>])
mcpp_add_test(NAME test_x SOURCES tests/test_x.cpp
 [STANDARD 23] [LINK_LIBS <targets...>]
 [INCLUDE_DIRS <dirs...>] [PCH <headers...>])
```

二者都会创建名为 `<module>__<name>` 的 target（全局唯一），链接 `mcpp::warnings` +
`mcpp::sanitizers`，并把 `OUTPUT_NAME` 设为 `<name>`。`mcpp_add_test` 额外链接
`GTest::gtest_main` 并通过 `gtest_discover_tests` 注册（前缀 `<module>.`）。
`STANDARD` 用于覆盖单 target 的 C++ 标准（例如某些 demo 故意要用旧标准）。
`LINK_LIBS` 追加 PRIVATE 链接依赖（例如 `Threads::Threads`），让模块不需要知道
`<module>__<name>` 这个内部 target 命名规则。
`INCLUDE_DIRS` 追加 PRIVATE 头文件搜索路径（替代 directory-scoped 的
`include_directories()`，粒度到单个 target）。
`PCH` 指定预编译头（调用 `target_precompile_headers` PRIVATE）。

## 跨平台不可破坏的约定

- **vcpkg triplet 在 Windows preset 内显式固化**：`msvc-*` / `clang-cl-*` 用
  `x64-windows`，`mingw-gcc-*` / `mingw-clang-*` 用 `x64-mingw-dynamic`；Linux/macOS
  上的 `gcc-*` / `clang-*` 不固化，由 vcpkg 按主机自动检测。`mingw-*` 两组 preset
  以 `${hostSystemName} == Windows` 门控，需要在 MSYS2 UCRT64 shell 里跑（或把
  `<msys2>/ucrt64/bin` 放进 PATH）。`CMakeUserPresets.json` 仍然 gitignored，但
  仅用于"覆盖编译器绝对路径 / 切到非默认 triplet" 这类个人化场景，不再是 MinGW 的
  必需步骤。
- **MSVC `/std:c++latest`** 在顶层 `CMakeLists.txt` 中强制启用，因为当前 MSVC 上
  `CMAKE_CXX_STANDARD=23` 还无法启用 `<print>` / `std::expected` / 新一批 ranges。
- **MSVC ASan 会剥离 `/RTC1`** —— 在 `cmake/CompilerSanitizers.cmake` 中处理（两者
  互不兼容）。
- **clangd 的 `compile_commands.json` 由顶层 CMakeLists.txt 自动镜像到源码根**：
  `mcpp_link_compile_commands` 这个 ALL custom target 会在 build 时把
  `build/<active-preset>/compile_commands.json` 复制到 `${CMAKE_SOURCE_DIR}/`，所以
  `.clangd` 里只写 `CompilationDatabase: .` 即可，**切换 preset 不需要改 .clangd**
  （只需重新 `cmake --build --preset <new>`）。多配置生成器（VS）不产 DB，跳过此机制。
  另：`Remove: [/Fo*, /Fd*, /FS]` 这一块对 clang-cl preset 至关重要 —— Ninja 生成的
  clang-cl 命令会带上这些 flag，clangd 改写命令后会报 "Cannot specify '/Fo...' when
  compiling multiple source files"。

## CI

`.github/workflows/ci.yml` 在每次 push/PR 上跑一个 9 路 build/test 矩阵 + 一个 lint
job + 一个聚合门禁 job：

- **build-test 矩阵**（`fail-fast: false`，任一失败即阻 merge）：
  - `linux-gcc`、`linux-clang`、`linux-gcc-asan`（`-DMCPP_ENABLE_SANITIZERS=ON`）、
    `linux-gcc-conan`（vcpkg 路径外的唯一 Conan 覆盖）
  - `windows-msvc`、`windows-clang-cl`、`windows-msvc-asan`、
    `windows-mingw-gcc`（MSYS2 UCRT64）
  - `macos-clang`（Apple Clang，验证 `clang-*` preset 的 Darwin 分支）
- **lint** job：`clang-format --dry-run --Werror`（`format-check` target）+
  `clang-tidy`（`tidy-check` target），都跑在 `clang-relwithdebinfo` configure
  之上。Linux jobs 在 `archlinux:base-devel` 容器里跑，pacman 滚动提供 GCC / Clang
  / clang-format / clang-tidy（当前 GCC 15.x、Clang 22.x，与本地 LLVM 22+ 一致）。
- **required-ci** 聚合门禁：`needs: [build-test, lint]` + `if: always()`，把整个
  矩阵的成功/失败汇总成单一稳定状态，分支保护规则只需 require 这一个就够。

vcpkg 的二进制产物通过 `x-gha` 缓存（每个 vcpkg job 都启用 `VCPKG_BINARY_SOURCES`）。
Conan 路径不走 vcpkg cache。完整指南见 `docs/ci-guide.md`。

## 需要保留的约定

- 不要引入 FetchContent，也不要 vendor 第三方代码；保持 `find_package` + manifest 作为
  唯一的依赖路径。
- 不要在单 target 上加裸 flag 的 `add_compile_options` —— 应当扩展 `mcpp::warnings`
  （或在 `cmake/` 下新增 INTERFACE target），以保持策略对所有编译器一致。
- 模块的 README/文档跟模块放一起（`modules/NN_*/README.md`、
  `modules/NN_*/docs/{zh-CN,en-US}.md`）；项目级 `docs/` 目录专门用于工具链指南
  （clangd / clang-format / clang-tidy / CI）。
- 完整的模块索引与快速开始见 `README.md`。

## 注释语言约定

- 项目相关文件（`modules/**/*.cpp`、`modules/**/CMakeLists.txt`、顶层 `CMakeLists.txt`、`cmake/*.cmake`）里的注释一律使用**中文**。
- 适用范围包括：文件头说明、行内注释、`//` 与 `/* … */` 注释、CMake 的 `#` 注释。
- 标识符（变量名、函数名、target 名等）仍保持英文，以匹配 `.clang-tidy` 规则。
- 文档文件（`*.md`）保留中英双语版本（`docs/zh-CN.md` / `docs/en-US.md`）的现有规则不变。
- 引用标准库术语、错误信息、第三方接口名时可保留原文（例如 `std::expected`、`gtest_discover_tests`），但围绕它们的解释用中文

## 提交流程（针对 C++ 源文件改动）

CI 的 lint job 用 `archlinux:base-devel` 容器里 pacman 滚动的 clang-format / clang-tidy
（当前 LLVM 22.x）跑 `--target format-check` + `--target tidy-check`，且 clang-tidy 用
`--warnings-as-errors=*` —— 任何 warning 都会让 PR 红。**修改 `modules/**/*.cpp` 或
`modules/**/*.h*` 后，commit 之前必须本地依次跑下面三条命令，全部 exit 0 才提交**：

```bash
cmake --build --preset <你的 preset> --target format        # in-place 修复
cmake --build --preset <你的 preset> --target format-check  # 复核
cmake --build --preset <你的 preset> --target tidy-check    # 静态检查
```

为了让本地 clang-format / clang-tidy 输出与 CI 一致：
- Windows：`scoop install llvm@22.1.4`（或更高 22.x patch）
- macOS：`brew install llvm@22`
- Linux：用 Arch / Tumbleweed 等滚动发行版；或 apt.llvm.org 装 22

低于 22 的版本会出现"本地通过 / CI 失败"的版本错配（默认值随 LLVM 大版本会变 ——
如 `IndentPPDirectives`、`AlignTrailingComments`，以及 21+ 新增的 lint check）。

**不需要跑这一套的场景**：只改文档（`*.md`）/ CI yml / CMakeLists / .clang-* 配置时
（这些都不在 format-check / tidy-check 的输入集里）—— 但如果同时改了 `.clang-format`
/ `.clang-tidy`，记得仍要跑一遍 check 验证规则没把现有代码变红。
