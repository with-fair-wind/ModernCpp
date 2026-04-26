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

## 快速开始 / Quickstart

### Linux (Ubuntu 24.04 / Debian 13)

```bash
# 1) 工具链
sudo apt update
sudo apt install -y build-essential gcc-13 g++-13 ninja-build cmake git

# 2) vcpkg（一次性）
git clone https://github.com/microsoft/vcpkg ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
echo 'export VCPKG_ROOT=$HOME/vcpkg' >> ~/.bashrc
source ~/.bashrc

# 3) 构建 + 测试
cmake --preset gcc-relwithdebinfo
cmake --build --preset gcc-relwithdebinfo --parallel
ctest   --preset gcc-relwithdebinfo
```

切 Clang：把 `gcc-` 换成 `clang-`，并 `apt install -y clang-18 lld-18`（C++23 库依赖
`libstdc++-13`，已被 g++-13 包带入）。

ARM64 Linux（Raspberry Pi、AWS Graviton 等）同样可用 —— vcpkg 会自动选 `arm64-linux`
triplet，无需额外配置。

### macOS (Intel / Apple Silicon)

```bash
# 1) 工具链
brew install cmake ninja llvm    # llvm 提供较新的 clang；Apple Clang 也可用，但 C++23 滞后
brew install vcpkg               # 或手动 git clone

# 2) 设置 VCPKG_ROOT
echo 'export VCPKG_ROOT=$(brew --prefix)/share/vcpkg' >> ~/.zshrc  # brew 安装的话
source ~/.zshrc

# 3) 构建 + 测试（macOS 上用 clang-* preset；gcc 在 mac 上是 clang shim，不要用 gcc-*）
cmake --preset clang-relwithdebinfo
cmake --build --preset clang-relwithdebinfo --parallel
ctest   --preset clang-relwithdebinfo
```

vcpkg 会根据主机自动选 `x64-osx`（Intel）或 `arm64-osx`（Apple Silicon），无需手动指定。
若 Apple Clang 的 C++23 支持不全，请用 `brew install llvm` 后让 PATH 优先指向 LLVM clang。

### Windows (MSVC / clang-cl)

```powershell
# 在 "x64 Native Tools Command Prompt for VS 2022" 或 Developer PowerShell 中：
$env:VCPKG_ROOT = "D:\path\to\vcpkg"
cmake --preset msvc
cmake --build --preset msvc-relwithdebinfo --parallel
ctest   --preset msvc-relwithdebinfo
```

clang-cl 同上，preset 改成 `clang-cl-relwithdebinfo`。

### Windows (MinGW UCRT64)

见下方 [vcpkg 段](#option-avcpkgmanifest-模式) 中关于 `CMakeUserPresets.json`
叠层的说明。

---

## 编译器 × 平台 × triplet 速查

仓库一份 `CMakePresets.json` 同时支持三种主流编译器；与之配套的 vcpkg triplet
**必须**与编译器 ABI 匹配，否则会出现 GoogleTest 链接失败。

策略：**Linux / macOS 上 vcpkg 会按主机自动检测 triplet**（一种 OS 一种主流 ABI，
检测可靠），preset 不固化；**Windows 上必须固化**，因为 MSVC ABI 与 MinGW ABI 共存，
vcpkg 默认猜 `x64-windows`，MinGW 用户需手动覆盖。

| 编译器 | 平台 | vcpkg triplet | preset 前缀 | 备注 |
| --- | --- | --- | --- | --- |
| **GCC**          | Linux x64 / ARM64 | `x64-linux` / `arm64-linux` | `gcc-*` | vcpkg 自动检测；preset 仅在 Linux 主机上可见 |
| **GCC**          | macOS             | —                     | —           | macOS 上 `gcc` 是 clang shim，请用 `clang-*` preset |
| **GCC**          | Windows (MinGW)  | `x64-mingw-dynamic`   | `my-gcc-*`  | 需要 `CMakeUserPresets.json` 叠层并 `"condition": null`（见下方 vcpkg 段） |
| **Clang**        | Linux x64 / ARM64 | `x64-linux` / `arm64-linux` | `clang-*` | vcpkg 自动检测；需要 libstdc++-13 提供 C++23 库 |
| **Clang**        | macOS Intel / Apple Silicon | `x64-osx` / `arm64-osx` | `clang-*` | vcpkg 自动检测；preset 在所有非 Windows 主机上可见 |
| **Clang**        | Windows (MinGW)  | `x64-mingw-dynamic`   | `my-clang-*`| msys2 ucrt64/clang++，方法同 MinGW GCC |
| **clang-cl**     | Windows (LLVM)   | `x64-windows`         | `clang-cl-*`| preset 内已固化；LLVM 官方分发，需在 VS Developer Prompt 中跑 |
| **MSVC**         | Windows          | `x64-windows`         | `msvc-*`    | preset 内已固化；VS 2022 multi-config |

Windows 上的 MSVC / clang-cl preset 已经把 triplet 固化在
`cacheVariables.VCPKG_TARGET_TRIPLET`，正常情况下你不用关心；只有 Windows MinGW 这种
"需要 override"的场景才要叠层或加 `-D` 覆盖：

```bash
# 方式 A：用本地 CMakeUserPresets.json 叠层（推荐，零环境变量）
cmake --preset my-gcc-debug

# 方式 B：configure 时一次性传入（绕过 condition 也只能用 -D 覆盖到已显示的 preset）
cmake --preset gcc-debug -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic
```

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
> 仓库内的 `gcc-*` / `clang-*` preset 已加 `condition: hostSystemName != Windows`
> （避免 Windows 用户误用 Linux triplet），所以 Windows 上的 MinGW 叠层 preset
> 必须用 `"condition": null` 显式解除：
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
>         { "name": "my-gcc-debug",   "inherits": ["_mingw-triplet", "gcc-debug"],   "condition": null },
>         { "name": "my-clang-debug", "inherits": ["_mingw-triplet", "clang-debug"], "condition": null }
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

仓库提供与 vcpkg 平行的一组 `*-conan` preset，直接读 Conan 生成的 toolchain，
无需手动 `-DCMAKE_TOOLCHAIN_FILE=`：

```bash
# 1) 安装 Conan 2.x
pip install --upgrade conan

# 2) 让 Conan 把依赖装到与 preset 同名的 build 目录
conan install . --output-folder=build/gcc-debug-conan --build=missing \
    -s compiler.cppstd=23

# 3) 直接用 conan preset 配置（toolchain 自动取自 build/<preset>/conan_toolchain.cmake）
cmake --preset gcc-debug-conan
cmake --build --preset gcc-debug-conan
ctest --preset gcc-debug-conan
```

可用的 conan preset：

| 编译器 | preset |
| --- | --- |
| GCC   | `gcc-debug-conan` / `gcc-release-conan` / `gcc-relwithdebinfo-conan` |
| Clang | `clang-debug-conan` / `clang-release-conan` / `clang-relwithdebinfo-conan` |
| MSVC  | `msvc-conan`（多配置）+ build/test preset `msvc-{debug,release,relwithdebinfo}-conan` |

---

## 构建与测试 / Build & Test

项目支持 CMake 的四种构建类型：`Debug` / `Release` / `RelWithDebInfo` / `MinSizeRel`，
并在三种编译器上都可用。

| 编译器 | 生成器 / Generator | 选择 build type 的方式 |
| --- | --- | --- |
| GCC      | Ninja（单配置）                 | 每种 build type 一个 preset：`gcc-debug` / `gcc-release` / `gcc-relwithdebinfo` / `gcc-minsizerel` |
| Clang    | Ninja（单配置）                 | 同上：`clang-debug` / `clang-release` / `clang-relwithdebinfo` / `clang-minsizerel` |
| clang-cl | Ninja（单配置，仅 Windows）     | 同上：`clang-cl-debug` / `clang-cl-release` / `clang-cl-relwithdebinfo` / `clang-cl-minsizerel` |
| MSVC     | Visual Studio 17 2022（多配置） | 单一 configurePreset `msvc`，构建时通过 buildPreset 选 `msvc-debug` / `msvc-release` / `msvc-relwithdebinfo` / `msvc-minsizerel` |

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

详细配置说明：

- [`docs/clangd-toolchain-overview.md`](docs/clangd-toolchain-overview.md) —— 工具链整体框架
- [`docs/clangd-config-guide.md`](docs/clangd-config-guide.md) —— `.clangd` 完整参考
- [`docs/clang-format-config-guide.md`](docs/clang-format-config-guide.md) —— `.clang-format` 完整参考
- [`docs/clang-tidy-config-guide.md`](docs/clang-tidy-config-guide.md) —— `.clang-tidy` 完整参考

---

## 持续集成 / CI

GitHub Actions（`.github/workflows/ci.yml`）会在每次 push 与 PR 上验证：

| Job | 平台 | 内容 |
| --- | --- | --- |
| `linux-gcc`     | ubuntu-24.04 | GCC 13 + vcpkg + `gcc-relwithdebinfo` preset，跑 ctest |
| `linux-clang`   | ubuntu-24.04 | Clang 18 + libstdc++-13 + `clang-relwithdebinfo` preset，跑 ctest |
| `windows-msvc`  | windows-2022 | MSVC (VS 2022) + `msvc-relwithdebinfo`，跑 ctest |
| `lint`          | ubuntu-24.04 | `clang-format-18 --dry-run --Werror` 检查所有 .cpp/.hpp/.h |

任一 job 失败即阻止 PR 合并，确保三编译器始终可编译且代码风格一致。

详细使用与配置说明：[`docs/ci-guide.md`](docs/ci-guide.md) —— CI 完整指南（触发、matrix、缓存、调试、分支保护、扩展功能）。

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
