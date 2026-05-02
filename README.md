# ModernCpp

现代 C++ 学习仓库：每篇笔记对应一个可编译、可测试的模块，可在 **GCC / Clang / MSVC /
clang-cl / MinGW GCC / MinGW Clang** 这一族编译器下验证（vcpkg 与 Conan 双轨支持，
Linux / Windows / macOS 全平台）。

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

# 2) vcpkg（一次性，手动 clone）
#    注：Homebrew 的 vcpkg formula 只装了可执行文件，不含 scripts/ports/triplets
#    完整源码树（CMake toolchain 文件就在源码树里），所以这里必须 git clone。
git clone https://github.com/microsoft/vcpkg ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
echo 'export VCPKG_ROOT=$HOME/vcpkg' >> ~/.zshrc
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
cmake --preset msvc                                  # 多配置：configure 一次
cmake --build --preset msvc-relwithdebinfo --parallel
ctest   --preset msvc-relwithdebinfo
```

> `msvc` 是多配置 preset：configure 一次后，`msvc-{debug,release,relwithdebinfo,minsizerel}`
> 四个 buildPreset 共享同一份工程，按需切换无需重 configure。

clang-cl 同上，preset 改成 `clang-cl-relwithdebinfo`（单配置 Ninja，每个 build type
一个 preset）。

### Windows (MinGW UCRT64)

```bash
# 在 MSYS2 UCRT64 shell 里（或者把 ucrt64 bin 目录放进 PATH）：
export VCPKG_ROOT=/d/path/to/vcpkg
cmake --preset mingw-gcc-relwithdebinfo
cmake --build --preset mingw-gcc-relwithdebinfo --parallel
ctest   --preset mingw-gcc-relwithdebinfo
```

切 MinGW Clang：把 `mingw-gcc-` 换成 `mingw-clang-` 即可（同样要求 ucrt64 的
`clang/clang++` 在 PATH 中）。这两组 preset 已经把 `VCPKG_TARGET_TRIPLET` 固化为
`x64-mingw-dynamic`，所以不需要再写 `CMakeUserPresets.json` 叠层。

---

## 编译器 × 平台 × triplet 速查

仓库一份 `CMakePresets.json` 同时支持五个单配置编译器家族（GCC / Clang / clang-cl /
MinGW GCC / MinGW Clang）和两个多配置 family（MSVC VS / Ninja Multi-Config）；与之配套的
vcpkg triplet **必须**与编译器 ABI 匹配，否则会出现 GoogleTest 链接失败。

策略：**Linux / macOS 上 vcpkg 会按主机自动检测 triplet**（一种 OS 一种主流 ABI，
检测可靠），preset 不固化；**Windows 上必须固化**，因为 MSVC ABI 与 MinGW ABI 共存，
vcpkg 默认猜 `x64-windows`，MinGW 用户需手动覆盖。

| 编译器 | 平台 | vcpkg triplet | preset 前缀 | 备注 |
| --- | --- | --- | --- | --- |
| **GCC**          | Linux x64 / ARM64 | `x64-linux` / `arm64-linux` | `gcc-*` | vcpkg 自动检测；preset 仅在 Linux 主机上可见 |
| **GCC**          | macOS             | —                     | —           | macOS 上 `gcc` 是 clang shim，请用 `clang-*` preset |
| **GCC**          | Windows (MinGW)  | `x64-mingw-dynamic`   | `mingw-gcc-*`  | preset 内已固化 triplet；需在 MSYS2 UCRT64 shell 中跑或把 ucrt64 bin 放进 PATH |
| **Clang**        | Linux x64 / ARM64 | `x64-linux` / `arm64-linux` | `clang-*` | vcpkg 自动检测；需要 libstdc++-13 提供 C++23 库 |
| **Clang**        | macOS Intel / Apple Silicon | `x64-osx` / `arm64-osx` | `clang-*` | vcpkg 自动检测；preset 在 Linux / macOS 主机上可见 |
| **Clang**        | Windows (MinGW)  | `x64-mingw-dynamic`   | `mingw-clang-*`| preset 内已固化 triplet；同样需要 MSYS2 UCRT64 环境 |
| **clang-cl**     | Windows (LLVM)   | `x64-windows`         | `clang-cl-*`| preset 内已固化；LLVM 官方分发，需在 VS Developer Prompt 中跑 |
| **MSVC**         | Windows          | `x64-windows`         | `msvc-*`    | preset 内已固化；VS 2022 multi-config |

> **为什么这样设计**：详见 [`docs/vcpkg-guide.md §5 triplet：ABI 的命名空间`](docs/vcpkg-guide.md#5-tripletabi-的命名空间)。

Windows 上四组 preset（`msvc-*` / `clang-cl-*` / `mingw-gcc-*` / `mingw-clang-*`）都
已经把 triplet 固化在 `cacheVariables.VCPKG_TARGET_TRIPLET`，不需要任何环境变量或 `-D`
覆盖；按编译器选 preset 即可：

```bash
cmake --preset mingw-gcc-debug      # MinGW GCC
cmake --preset mingw-clang-debug    # MinGW Clang
cmake --preset clang-cl-debug       # LLVM clang-cl（MSVC ABI）
cmake --preset msvc                 # MSVC（多配置）
```

如需切换到 `x64-mingw-static` 等其他 triplet，可用本地 `CMakeUserPresets.json` 叠一层
`cacheVariables.VCPKG_TARGET_TRIPLET` 覆盖，或临时加 `-DVCPKG_TARGET_TRIPLET=...`。

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

> **Windows + MinGW GCC/Clang**：仓库已内置 `mingw-gcc-*` 与 `mingw-clang-*` 两组
> preset（仅在 `${hostSystemName} == Windows` 时可见），triplet 已固化为
> `x64-mingw-dynamic`，开箱即用，无需 `CMakeUserPresets.json` 叠层。背景与设计动机
> 详见 [`docs/vcpkg-guide.md §6 MinGW 专题`](docs/vcpkg-guide.md#6-mingw-专题)。

> **完整文档**：vcpkg 端到端使用、binary cache、CRT 对齐、排查 FAQ 详见
> [`docs/vcpkg-guide.md`](docs/vcpkg-guide.md)；preset 设计原理详见
> [`docs/cmake-presets-guide.md`](docs/cmake-presets-guide.md)。

### Option B：Conan

仓库提供与 vcpkg 平行的一组 `*-conan` preset，直接读 Conan 生成的 toolchain，
无需手动 `-DCMAKE_TOOLCHAIN_FILE=`。仓库还内置了 `conan/profiles/`：把每个常见
平台 / 工具链的 ABI（compiler / version / libcxx / runtime …）固化成 profile 文件，
让"哪个 preset 用哪份 profile"在仓库里就是 reproducible 的，不依赖 `conan profile detect`
在不同机器上嗅探的结果。

```bash
# 1) 安装 Conan 2.x
pip install --upgrade conan

# 2) 首次：初始化 Conan home 的默认 build profile（构建工具用，与 host profile 不同）
conan profile detect --force

# 3) 用仓库内置的 host profile 装依赖到与 preset 同名的目录
conan install . -pr=./conan/profiles/linux-gcc \
    -s build_type=Debug \
    --output-folder=build/gcc-debug-conan \
    --build=missing

# 4) 直接用 conan preset 配置（toolchain 自动取自 build/<preset>/conan_toolchain.cmake）
cmake --preset gcc-debug-conan
cmake --build --preset gcc-debug-conan
ctest --preset gcc-debug-conan
```

> ⚠️ `-s build_type=` 必须与 preset 后缀一致（`*-debug-conan` ↔ `Debug`、
> `*-relwithdebinfo-conan` ↔ `RelWithDebInfo`，类推）。CMakeDeps 只为请求的
> build_type 生成 per-config 目标文件；mismatch 会 configure 通过、构建炸
> `gtest/gtest.h: No such file`。`cmake/Dependencies.cmake` 已加 configure 期
> 防御检查，撞上时会直接 FATAL_ERROR 提示重跑。

#### 内置 profile × preset 对照表

| profile | 目标场景 | 配套 preset 前缀 |
| --- | --- | --- |
| `conan/profiles/linux-gcc`     | Linux + GCC 13（Ubuntu 24.04 / Debian 13）         | `gcc-*-conan` |
| `conan/profiles/linux-clang`   | Linux + Clang 18 + libstdc++（Ubuntu 24.04）       | `clang-*-conan` |
| `conan/profiles/macos-clang`   | macOS + apple-clang 16+ / libc++（Xcode 16+）      | `clang-*-conan` |
| `conan/profiles/msvc`          | Windows + MSVC ABI（cl.exe 或 clang-cl）           | `msvc-conan` / `ninja-mc-msvc-conan` / `clang-cl-*-conan` |
| `conan/profiles/mingw-ucrt64`  | Windows + MSYS2 **UCRT64** GCC（仓库官方支持的 MinGW 入口） | `mingw-gcc-*-conan` |
| `conan/profiles/mingw-clang64` | Windows + MSYS2 **CLANG64** Clang + libc++          | `mingw-clang-*-conan` |

每份 profile 顶部都有用法注释、版本固化说明、以及"如何在 CLI 上覆盖 compiler.version"
的指导。MSYS2 多环境共存时的 cache aliasing 注意事项见 `mingw-ucrt64` profile 内说明。

#### Windows + MSYS2 完整命令示例

```bash
# 在 MSYS2 UCRT64 shell 里：
conan profile detect --force      # 仅首次
conan install . -pr=./conan/profiles/mingw-ucrt64 \
    -s build_type=RelWithDebInfo \
    --output-folder=build/mingw-gcc-relwithdebinfo-conan \
    --build=missing
cmake --preset mingw-gcc-relwithdebinfo-conan
cmake --build --preset mingw-gcc-relwithdebinfo-conan --parallel
ctest   --preset mingw-gcc-relwithdebinfo-conan
```

CLANG64 把 `mingw-ucrt64` profile 名 + `mingw-gcc-*-conan` preset 名同时换成
`mingw-clang64` / `mingw-clang-*-conan`。

可用的 conan preset（与 vcpkg 一侧完全对称，同样四种 build type）：

| 编译器 | preset |
| --- | --- |
| GCC                | `gcc-{debug,release,relwithdebinfo,minsizerel}-conan` |
| Clang              | `clang-{debug,release,relwithdebinfo,minsizerel}-conan` |
| clang-cl           | `clang-cl-{debug,release,relwithdebinfo,minsizerel}-conan` |
| MinGW GCC          | `mingw-gcc-{debug,release,relwithdebinfo,minsizerel}-conan` |
| MinGW Clang        | `mingw-clang-{debug,release,relwithdebinfo,minsizerel}-conan` |
| MSVC (VS)          | `msvc-conan`（多配置）+ build/test `msvc-{debug,release,relwithdebinfo,minsizerel}-conan` ⚠️ |
| MSVC (Ninja MC)    | `ninja-mc-msvc-conan`（多配置）+ build/test `ninja-mc-msvc-{...}-conan` ⚠️ |

> ⚠️ `msvc-conan` 与 `ninja-mc-msvc-conan` **多配置 + Conan 有固有摩擦**：Conan 的 `conan_toolchain.cmake` 是
> per-build-type 的，每跑一次 `conan install` 会**覆盖**前一次的 toolchain，所以这两组 preset 在实际使用中
> **同一时刻只能跑一种 build_type**（4 个 buildPreset 是为了"切到这一种"提供入口，不是真的能并存 4 种）。
> 想要 Windows MSVC ABI + Conan **同时保留 4 种 build_type**，请改用 `clang-cl-{...}-conan`（单配置 Ninja，
> 4 个独立的 build dir 与 toolchain，零摩擦；clang-cl 与 cl.exe 是同一套 ABI）。详见
> [`docs/conan-guide.md` §"多配置 + Conan 的固有摩擦"](docs/conan-guide.md#多配置--conan-的固有摩擦)。

> **完整文档**：Conan 2.x 概念、profile、CMakeDeps/CMakeToolchain、三步走工作流、
> 排查 FAQ 详见 [`docs/conan-guide.md`](docs/conan-guide.md)。

---

## 构建与测试 / Build & Test

项目支持 CMake 的四种构建类型：`Debug` / `Release` / `RelWithDebInfo` / `MinSizeRel`，
在所有受支持的编译器家族上都可用。

| 编译器 | 生成器 / Generator | 选择 build type 的方式 |
| --- | --- | --- |
| GCC          | Ninja（单配置）                  | 每种 build type 一个 preset：`gcc-debug` / `gcc-release` / `gcc-relwithdebinfo` / `gcc-minsizerel` |
| Clang        | Ninja（单配置）                  | 同上：`clang-debug` / `clang-release` / `clang-relwithdebinfo` / `clang-minsizerel` |
| clang-cl     | Ninja（单配置，仅 Windows）      | 同上：`clang-cl-debug` / `clang-cl-release` / `clang-cl-relwithdebinfo` / `clang-cl-minsizerel` |
| MinGW GCC    | Ninja（单配置，仅 Windows）      | 同上：`mingw-gcc-{debug,release,relwithdebinfo,minsizerel}` |
| MinGW Clang  | Ninja（单配置，仅 Windows）      | 同上：`mingw-clang-{debug,release,relwithdebinfo,minsizerel}` |
| MSVC (VS)    | Visual Studio 17 2022（多配置）  | 单一 configurePreset `msvc`，buildPreset 选 `msvc-{debug,release,relwithdebinfo,minsizerel}` |
| MSVC (NMC)   | Ninja Multi-Config（多配置）     | 单一 configurePreset `ninja-mc-msvc`，buildPreset 选 `ninja-mc-msvc-{debug,release,...}` |

> **Ninja Multi-Config 是什么**：CMake 4 自带的多配置生成器，跟 VS 一样能"一次 configure 出 4 种 build
> type 共享的工程"，但底层是几份 `build-Debug.ninja` / `build-Release.ninja` ... 通过
> `cmake --build <dir> --config <Type>` 切换，**速度近似 Ninja 单配置 + 体验近似 VS 多配置**。优势：
> 比 VS 快、跨平台可用、不依赖 .sln/.vcxproj；劣势：与 Conan 配合时仍有 per-build-type toolchain 摩擦
> （见上方 ⚠️）。日常推荐：单平台单 build type 选单配置 Ninja preset；想"一次 configure 切多种 build
> type"且不在乎 IDE 集成的，用 `ninja-mc-msvc`。

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
| `MCPP_ENABLE_SANITIZERS` | OFF | 开启 AddressSanitizer（GCC/Clang Unix 驱动 + UBSan；MSVC / clang-cl 仅 ASan，UBSan 不可用），仅作用于 Debug / RelWithDebInfo。MSVC ABI 下会去掉 `/RTC1`（与 ASan 不兼容） |

例：`cmake --preset gcc-debug -DMCPP_BUILD_TESTS=OFF`。
开 sanitizer 的常用组合：`cmake --preset gcc-debug -DMCPP_ENABLE_SANITIZERS=ON`。

### 格式化 / Formatting

仓库根的 `.clang-format` 控制 C/C++ 风格，CI 用 `clang-format-18 --dry-run --Werror` 校验。
本地一键修复 / 校验：

```bash
cmake --build --preset gcc-debug --target format        # 原地修复全部 .cpp/.hpp
cmake --build --preset gcc-debug --target format-check  # 仅 dry-run，与 CI 行为一致
```

需要 `clang-format`（推荐 18+）在 PATH 中；CMake 配置时 `find_program` 检测不到就跳过这两个 target。
非 C/C++ 文件（CMake、JSON、YAML、Markdown）的缩进由根目录 `.editorconfig` 兜底，VS Code / JetBrains
等主流 IDE 会自动识别。

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

GitHub Actions（`.github/workflows/ci.yml`）在每次 push 与 PR 上跑一个 9 路 build/test 矩阵 + 一个 lint job：

| Job | 平台 | 内容 |
| --- | --- | --- |
| `linux-gcc`        | ubuntu-24.04 | GCC 13 + vcpkg + `gcc-relwithdebinfo` |
| `linux-clang`      | ubuntu-24.04 | Clang 18 + libstdc++-13 + `clang-relwithdebinfo` |
| `linux-gcc-asan`   | ubuntu-24.04 | GCC 13 + `gcc-debug` + `MCPP_ENABLE_SANITIZERS=ON`（ASan + UBSan） |
| `linux-gcc-conan`  | ubuntu-24.04 | GCC 13 + Conan 2.x + `gcc-relwithdebinfo-conan` |
| `windows-msvc`     | windows-2022 | MSVC (VS 2022) + `msvc-relwithdebinfo` |
| `windows-clang-cl` | windows-2022 | LLVM clang-cl + `clang-cl-relwithdebinfo` |
| `windows-msvc-asan`| windows-2022 | MSVC + `msvc-debug` + `MCPP_ENABLE_SANITIZERS=ON`（MSVC ASan） |
| `windows-mingw-gcc`| windows-2022 | MSYS2 UCRT64 GCC + `mingw-gcc-relwithdebinfo`（验证非-MSVC ABI 路径 + `x64-mingw-dynamic` triplet） |
| `macos-clang`      | macos-14     | Apple Clang + `clang-relwithdebinfo`（验证 `_clang` preset 的 Darwin 分支） |
| `lint`             | ubuntu-24.04 | `clang-format-18 --dry-run --Werror` + `clang-tidy-18`（target `format-check` / `tidy-check`） |

任一 job 失败即阻止 PR 合并。该矩阵不仅覆盖三种主流编译器，还把 sanitizer、Conan、MinGW、clang-cl、macOS 都纳入主线，避免某条路径"名义支持但长期未跑"。

详细使用与配置说明：[`docs/ci-guide.md`](docs/ci-guide.md) —— CI 完整指南（触发、matrix、缓存、调试、分支保护、扩展功能）。

---

## Module Index / 模块索引

图例：✅ 已落地 demos/tests · ⚠️ WIP（仅文档，无 demos/tests）。

| # | 模块 | 主题 | 状态 |
| - | --- | --- | --- |
| 01 | [`modules/01_basics`](modules/01_basics/) | 基础复习与扩展 / Basics Review & Extensions | ✅ |
| 02 | [`modules/02_lifetime_type_safety`](modules/02_lifetime_type_safety/) | 生命周期与类型安全 / Lifetime & Type Safety | ✅ |
| 03 | [`modules/03_multi_file`](modules/03_multi_file/) | 多文件编程 / Multi-file Programming | ⚠️ WIP |
| 04 | [`modules/04_streams_strings`](modules/04_streams_strings/) | 流与字符串 / Streams & Strings | ⚠️ WIP |
| 05 | [`modules/05_containers_ranges_p1`](modules/05_containers_ranges_p1/) | 容器、ranges 与算法 Part 1 | ⚠️ WIP |
| 06 | [`modules/06_containers_ranges_p2`](modules/06_containers_ranges_p2/) | 容器、ranges 与算法 Part 2 | ⚠️ WIP |
| 07 | [`modules/07_value_categories`](modules/07_value_categories/) | 值分类与移动语义 / Value Categories & Move Semantics | ⚠️ WIP |
| 08 | [`modules/08_templates_basics`](modules/08_templates_basics/) | 模板基础与移动语义 / Templates Basics & Move Semantics | ⚠️ WIP |
| 09 | [`modules/09_templates_advanced`](modules/09_templates_advanced/) | 模板进阶 / Advanced Templates | ⚠️ WIP |
| 10 | [`modules/10_memory`](modules/10_memory/) | 内存管理 / Memory Management | ⚠️ WIP |
| 11 | [`modules/11_error_handling`](modules/11_error_handling/) | 错误处理 / Error Handling | ✅ |
| 12 | [`modules/12_threading`](modules/12_threading/) | 多线程（新）/ Threading (Modern) | ✅ |
| 13 | [`modules/13_concurrency_advanced`](modules/13_concurrency_advanced/) | 并发进阶 / Advanced Concurrency | ⚠️ WIP |
| 14 | [`modules/14_move_semantics_basics`](modules/14_move_semantics_basics/) | Move Semantics Basics | ⚠️ WIP |
| 15 | [`modules/15_summary`](modules/15_summary/) | 补充与总结 / Supplements & Summary | ⚠️ WIP |
