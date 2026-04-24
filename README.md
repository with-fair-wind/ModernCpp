# ModernCpp

现代 C++ 学习仓库：每篇笔记对应一个可编译、可测试的模块，能在 **gcc / clang / msvc**
三种主流编译器下验证。

A Modern C++ learning repo — every knowledge-point doc is paired with a runnable
module (demos + optional unit tests), buildable on gcc, clang, and MSVC.

---

## 目录结构 / Layout

```
ModernCpp/
├── CMakeLists.txt            # 顶层 / top-level
├── CMakePresets.json         # gcc/clang/msvc presets
├── .clangd, .clang-format    # clangd 统一代码风格
├── vcpkg.json, conanfile.txt # 依赖清单（二选一）
├── cmake/                    # CompilerWarnings / ModuleHelpers / Dependencies
└── modules/
    └── NN_shortname/
        ├── CMakeLists.txt
        ├── README.md
        ├── docs/{zh-CN.md, en-US.md}
        ├── demos/*.cpp
        └── tests/*.cpp
```

模块索引见下方 [Module Index](#module-index--模块索引)。

---

## 前置 / Prerequisites

- CMake ≥ 3.25
- Ninja
- 任一编译器：GCC ≥ 13、Clang ≥ 16、MSVC (VS 2022 17.8+)（为完整 C++23 支持）
- **vcpkg** 或 **Conan** 任一（用于拉 GoogleTest）

---

## 依赖安装 / Installing Dependencies

项目使用 `find_package` 查询第三方依赖，不内置 FetchContent。
你可以在 vcpkg 与 Conan 中任选其一。

### Option A：vcpkg（manifest 模式）

```bash
# 1) 安装 vcpkg（首次）
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh     # Linux/macOS
# 或 .\vcpkg\bootstrap-vcpkg.bat  (Windows)

# 2) 把 vcpkg 根目录设到环境变量 VCPKG_ROOT 里（一次性，装机步骤）
#    Linux/macOS: 写进 .bashrc / .zshrc
export VCPKG_ROOT=/path/to/vcpkg
#    Windows PowerShell（持久化到用户级环境变量）
setx VCPKG_ROOT "D:\path\to\vcpkg"

# 3) 直接 configure，toolchain 由仓库 preset 通过 $env{VCPKG_ROOT} 找到
cmake --preset gcc-debug
```

vcpkg 会读取仓库根的 `vcpkg.json` 自动拉取 `gtest`。Linux / macOS / Windows
MSVC 到这里就好了，零额外配置。

> **Windows + MinGW GCC/Clang 的一个额外步骤**：vcpkg 在 Windows 上不会检测
> 当前编译器，总默认 triplet 为 `x64-windows`（MSVC ABI），用它装出来的
> GoogleTest 与 MinGW GCC 链接时会报大量 `undefined reference to testing::...`。
> 解决办法：在仓库根建一份 `CMakeUserPresets.json`，给 gcc/clang preset 叠一层
> triplet 覆盖（`x64-mingw-dynamic`）。已经在 `.gitignore`，不影响仓库：
>
> ```json
> {
>     "version": 6,
>     "configurePresets": [
>         {
>             "name": "_mingw-triplet",
>             "hidden": true,
>             "cacheVariables": { "VCPKG_TARGET_TRIPLET": "x64-mingw-dynamic" }
>         },
>         { "name": "my-gcc-debug",   "inherits": ["_mingw-triplet", "gcc-debug"] },
>         { "name": "my-clang-debug", "inherits": ["_mingw-triplet", "clang-debug"] }
>     ],
>     "buildPresets": [
>         { "name": "my-gcc-debug",   "configurePreset": "my-gcc-debug" },
>         { "name": "my-clang-debug", "configurePreset": "my-clang-debug" }
>     ],
>     "testPresets": [
>         { "name": "my-gcc-debug",   "configurePreset": "my-gcc-debug",
>           "output": { "outputOnFailure": true, "shortProgress": true } }
>     ]
> }
> ```
>
> 然后用 `cmake --preset my-gcc-debug` 代替 `gcc-debug`。需要 release /
> relwithdebinfo / minsizerel 就同样照着加一组 `my-*` preset。
>
> 备选：每次 configure 时手动加 `-DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic`。
>
> MSVC preset 用默认的 `x64-windows` 即可，不需要 user preset。

### Option B：Conan

```bash
# 1) 安装 Conan 2.x
pip install --upgrade conan

# 2) 让 Conan 安装依赖并生成 toolchain
conan install . --output-folder=build/gcc-debug --build=missing \
    -s compiler.cppstd=23

# 3) 配置项目
cmake --preset gcc-debug \
    -DCMAKE_TOOLCHAIN_FILE=build/gcc-debug/conan_toolchain.cmake
```

---

## 构建与测试 / Build & Test

项目支持 CMake 的四种构建类型：`Debug` / `Release` / `RelWithDebInfo` / `MinSizeRel`，
并在三种编译器上都可用。

| 编译器 | 生成器 / Generator | 选择 build type 的方式 |
| --- | --- | --- |
| GCC   | Ninja（单配置）    | 为每种 build type 提供独立 configurePreset：`gcc-debug` / `gcc-release` / `gcc-relwithdebinfo` / `gcc-minsizerel` |
| Clang | Ninja（单配置）    | 同上：`clang-debug` / `clang-release` / `clang-relwithdebinfo` / `clang-minsizerel` |
| MSVC  | Visual Studio 17 2022（多配置） | 单一 configurePreset `msvc`，在构建时通过 buildPreset 选择：`msvc-debug` / `msvc-release` / `msvc-relwithdebinfo` / `msvc-minsizerel` |

列出所有可用 preset：

```bash
cmake --list-presets
cmake --build --list-presets
ctest --list-presets
```

以 GCC Debug 为例：

```bash
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
```

切 build type / 切编译器只需换 preset 名。

MSVC 举例（需在 **x64 Native Tools Command Prompt for VS 2022** 或 Developer PowerShell 里跑）：

```powershell
cmake --preset msvc                      # 只 configure 一次，生成 4 种 config 的工程
cmake --build --preset msvc-release      # 构建 Release
cmake --build --preset msvc-debug        # 构建 Debug
ctest --preset msvc-release
```

### 顶层选项 / Top-level options

| 选项 | 默认 | 作用 |
| --- | --- | --- |
| `MCPP_BUILD_DEMOS` | ON | 构建所有 demo 可执行 |
| `MCPP_BUILD_TESTS` | ON | 构建 GoogleTest 测试；关掉可跳过依赖解析 |
| `MCPP_WARNINGS_AS_ERRORS` | OFF | 把 `-Wall/-Wextra/...` 或 `/W4` 提升为错误 |
| `MCPP_ENABLE_SANITIZERS` | OFF | 开启 AddressSanitizer（+ UBSan，仅 GCC/Clang），仅作用于 Debug / RelWithDebInfo |

例：`cmake --preset gcc-debug -DMCPP_BUILD_TESTS=OFF`。
开 sanitizer 的常用组合：`cmake --preset gcc-debug -DMCPP_ENABLE_SANITIZERS=ON`。

---

## 加新 demo / 加新模块

单个 demo：在对应 `modules/NN_*/CMakeLists.txt` 里加一行

```cmake
mcpp_add_demo(NAME my_demo SOURCES demos/my_demo.cpp)
# 需要不同 C++ 标准时:
# mcpp_add_demo(NAME uses_print SOURCES demos/uses_print.cpp STANDARD 23)
```

单元测试同理：

```cmake
mcpp_add_test(NAME test_my_topic SOURCES tests/test_my_topic.cpp)
```

新增模块：在 `modules/` 下建一个 `NN_shortname/` 文件夹并放一份 `CMakeLists.txt`，
顶层会自动 `add_subdirectory`，无需改顶层文件。

---

## 代码风格 / Code Style

仓库根的 `.clangd` 与 `.clang-format` 由 clangd 自动识别，支持 clangd 的编辑器
（VSCode + clangd 扩展、Neovim + nvim-lspconfig、CLion 内置、Emacs lsp-mode 等）
开箱即用。无需额外 IDE 配置。

---

## Module Index / 模块索引

| # | 模块 | 主题 |
| - | --- | --- |
| 01 | [`modules/01_basics`](modules/01_basics/) | 基础复习与扩展 / Basics Review & Extensions |
| 02 | [`modules/02_lifetime_type_safety`](modules/02_lifetime_type_safety/) | 生命周期与类型安全 / Lifetime & Type Safety |
| 03 | [`modules/03_multi_file`](modules/03_multi_file/) | 多文件编程 / Multi-file Programming |
| 04 | [`modules/04_streams_strings`](modules/04_streams_strings/) | 流与字符串 / Streams & Strings |
| 05 | [`modules/05_containers_ranges_p1`](modules/05_containers_ranges_p1/) | 容器、ranges 与算法 Part 1 |
| 06 | [`modules/06_containers_ranges_p2`](modules/06_containers_ranges_p2/) | 容器、ranges 与算法 Part 2 |
| 07 | [`modules/07_value_categories`](modules/07_value_categories/) | 值分类与移动语义 / Value Categories & Move Semantics |
| 08 | [`modules/08_templates_basics`](modules/08_templates_basics/) | 模板基础与移动语义 / Templates Basics & Move Semantics |
| 09 | [`modules/09_templates_advanced`](modules/09_templates_advanced/) | 模板进阶 / Advanced Templates |
| 10 | [`modules/10_memory`](modules/10_memory/) | 内存管理 / Memory Management |
| 11 | [`modules/11_error_handling`](modules/11_error_handling/) | 错误处理 / Error Handling |
| 12 | [`modules/12_threading`](modules/12_threading/) | 多线程（新）/ Threading (Modern) |
| 13 | [`modules/13_concurrency_advanced`](modules/13_concurrency_advanced/) | 并发进阶 / Advanced Concurrency |
| 14 | [`modules/14_move_semantics_basics`](modules/14_move_semantics_basics/) | Move Semantics Basics |
| 15 | [`modules/15_summary`](modules/15_summary/) | 补充与总结 / Supplements & Summary |
