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

顶层开关：`-DMCPP_BUILD_TESTS=OFF`（跳过 gtest 依赖）、`-DMCPP_ENABLE_SANITIZERS=ON`
（GCC/Clang 上启用 ASan + UBSan，MSVC 上启用 ASan；仅 Debug/RelWithDebInfo 生效）、
`-DMCPP_WARNINGS_AS_ERRORS=ON`。

## 模块辅助函数 —— 你需要的全部 API

每个模块的 `CMakeLists.txt` 只调用两个函数，定义在 `cmake/ModuleHelpers.cmake` 中：

```cmake
mcpp_add_demo(NAME my_demo SOURCES demos/my_demo.cpp [STANDARD 23])
mcpp_add_test(NAME test_x  SOURCES tests/test_x.cpp  [STANDARD 23])
```

二者都会创建名为 `<module>__<name>` 的 target（全局唯一），链接 `mcpp::warnings` +
`mcpp::sanitizers`，并把 `OUTPUT_NAME` 设为 `<name>`。`mcpp_add_test` 额外链接
`GTest::gtest_main` 并通过 `gtest_discover_tests` 注册（前缀 `<module>.`）。
`STANDARD` 用于覆盖单 target 的 C++ 标准（例如某些 demo 故意要用旧标准）。

## 跨平台不可破坏的约定

- **vcpkg triplet 已固化在 `CMakePresets.json`**（gcc/clang 用 `x64-linux`，
  msvc/clang-cl 用 `x64-windows`）。MinGW UCRT64 用户通过 gitignored 的
  `CMakeUserPresets.json` 叠加，并用 `"condition": null` 解除主机 OS 门控；模式见
  `README.md`。
- **MSVC `/std:c++latest`** 在顶层 `CMakeLists.txt` 中强制启用，因为当前 MSVC 上
  `CMAKE_CXX_STANDARD=23` 还无法启用 `<print>` / `std::expected` / 新一批 ranges。
- **MSVC ASan 会剥离 `/RTC1`** —— 在 `cmake/CompilerSanitizers.cmake` 中处理（两者
  互不兼容）。
- **clangd 读取 `build/clang-cl-relwithdebinfo`** 作为 `compile_commands.json` 来源
  （在 `.clangd` 中设置）。其中 `Remove: [/Fo*, /Fd*, /FS]` 这一块至关重要 —— Ninja
  生成 clang-cl 命令时会带上这些 flag，clangd 改写命令后会报 "Cannot specify
  '/Fo...' when compiling multiple source files"。**切换当前活跃 preset 时，记得同步
  修改 `.clangd` 里的 `CompilationDatabase`**。

## CI

`.github/workflows/ci.yml` 在每次 push/PR 上跑四个 job：`linux-gcc`、`linux-clang`、
`windows-msvc`（流程：vcpkg → preset configure → build → ctest），加一个 `lint` job
（`clang-format-18 --dry-run --Werror`）。四个全过才能 merge。vcpkg 的二进制产物通过
`x-gha` 缓存。

## 需要保留的约定

- 不要引入 FetchContent，也不要 vendor 第三方代码；保持 `find_package` + manifest 作为
  唯一的依赖路径。
- 不要在单 target 上加裸 flag 的 `add_compile_options` —— 应当扩展 `mcpp::warnings`
  （或在 `cmake/` 下新增 INTERFACE target），以保持策略对所有编译器一致。
- 模块的 README/文档跟模块放一起（`modules/NN_*/README.md`、
  `modules/NN_*/docs/{zh-CN,en-US}.md`）；项目级 `docs/` 目录专门用于工具链指南
  （clangd / clang-format / clang-tidy / CI）。
- 完整的模块索引与快速开始见 `README.md`。
