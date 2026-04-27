# vcpkg 使用与配置完整指南

> 本文档系统讲解 vcpkg 的工作原理、安装与首次配置、manifest 模式、triplet 与 ABI、
> Windows MinGW 专题、binary cache 加速，以及本仓库 `vcpkg.json` 是如何与 CMake Presets
> 衔接的。
>
> 配套文件：[`vcpkg.json`](../vcpkg.json) / [`cmake/Dependencies.cmake`](../cmake/Dependencies.cmake)
>
> 配套阅读：
> - [`docs/cmake-presets-guide.md`](cmake-presets-guide.md) —— preset 与 toolchain hook 机制
> - [`docs/conan-guide.md`](conan-guide.md) —— 平行的另一种方案
> - [`docs/ci-guide.md`](ci-guide.md) —— CI 中 vcpkg binary cache 配置

---

## 目录

1. [什么是 vcpkg，它解决什么问题](#1-什么是-vcpkg它解决什么问题)
2. [安装与首次配置](#2-安装与首次配置)
3. [manifest 模式与 vcpkg.json 详解](#3-manifest-模式与-vcpkgjson-详解)
4. [toolchain 文件的工作机制](#4-toolchain-文件的工作机制)
5. [triplet：ABI 的命名空间](#5-tripletabi-的命名空间)
6. [MinGW 专题](#6-mingw-专题)
7. [binary cache：加速重复构建](#7-binary-cache加速重复构建)
8. [MSVC CRT 对齐](#8-msvc-crt-对齐)
9. [常用命令速查](#9-常用命令速查)
10. [常见问题与排查](#10-常见问题与排查)

---

## 1. 什么是 vcpkg，它解决什么问题

**vcpkg** 是微软开源的 C++ 包管理器：用一份清单文件（`vcpkg.json`）声明依赖，vcpkg
负责下载、编译、按 ABI 配置安装到本地，并通过 CMake 的 toolchain 文件让你的项目
`find_package` 能找到。

### C++ 没有 npm/pip 的痛点

Python 装个 `requests` 是 `pip install requests`，不用关心：

- 编译？不用，纯 Python
- ABI？不用，只有一个 CPython
- 链接？不用，import 即可

C++ 装一个 GoogleTest 要回答一连串问题：

- 用什么编译器？（GCC / Clang / MSVC，版本？）
- 静态还是动态？
- 链接 MSVC 的哪个 CRT？（/MD / /MT）
- 装到哪？怎么让 `find_package(GTest)` 找到？

vcpkg 把这些问题打包成一个概念叫 **triplet**（详见 [§5](#5-tripletabi-的命名空间)），
让你只需要声明依赖名，vcpkg 替你把 ABI 拼对。

### vcpkg 的两种工作模式

| 模式 | 入口 | 安装位置 | 适用 |
| --- | --- | --- | --- |
| **classic** | `vcpkg install gtest` | `<vcpkg-root>/installed/` 全局共享 | 个人随手玩 |
| **manifest** | `vcpkg.json` + CMake | per-project 的 `vcpkg_installed/` | 团队/CI/可复现 |

**本仓库用 manifest 模式**：每个项目一份 `vcpkg.json`，依赖随源码一起进版本管理，谁
clone 都装得出一样的版本。下文专讲 manifest 模式。

### 与 Conan 的高层对比

完整对比表见 [conan-guide.md §1](conan-guide.md#1-什么是-conan它解决什么问题)。一句话：
**vcpkg 像 apt（编译型）；Conan 像 maven（recipe + 二进制分发）**。本仓库同时支持两种，
让用户体验不同生态。

---

## 2. 安装与首次配置

vcpkg 的官方推荐安装方式是 **git clone 源码树**，不是包管理器（包管理器版本只装可执行
文件，不含 `scripts/ports/triplets`，无法走 manifest 模式）。

### Linux / macOS

```bash
# 1) 克隆 vcpkg
git clone https://github.com/microsoft/vcpkg ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh

# 2) 设置 VCPKG_ROOT 环境变量（一次性）
echo 'export VCPKG_ROOT=$HOME/vcpkg' >> ~/.bashrc   # 或 ~/.zshrc
source ~/.bashrc
```

> **macOS 注意**：不要 `brew install vcpkg` —— Homebrew 的 formula 只装 vcpkg 可执行
> 文件，不含完整源码树。后续 `cmake --preset` 会因为找不到
> `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake` 而失败。

### Windows

```powershell
# 1) 克隆 vcpkg（建议放短路径，避免 MAX_PATH 问题）
git clone https://github.com/microsoft/vcpkg D:\vcpkg
D:\vcpkg\bootstrap-vcpkg.bat

# 2) 持久化 VCPKG_ROOT 到用户级环境变量
setx VCPKG_ROOT "D:\vcpkg"
# 注意：setx 设置的环境变量在新开的 shell 才生效，当前 shell 仍未生效
```

### 为什么本仓库用 `$env{VCPKG_ROOT}` 而不是固定路径

```json
"CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
```

把路径写成环境变量展开有几个好处：

- **机器无关**：每个开发者把 vcpkg 装在自己喜欢的位置
- **多项目共享同一份 vcpkg root**：vcpkg 的 binary cache 能跨项目复用
- **CI 与本地一致**：CI 也只是设 `VCPKG_ROOT` 环境变量，preset 不需要改

代价：用户必须先设环境变量。第一次 configure 报错 "Could not find toolchain file" 时，
99% 是 `VCPKG_ROOT` 没设或没在当前 shell 里生效。

---

## 3. manifest 模式与 vcpkg.json 详解

本仓库的 `vcpkg.json` 全文：

```json
{
    "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
    "name": "moderncpp",
    "version-string": "0.1.0",
    "description": "Modern C++ learning modules",
    "dependencies": [
        "gtest"
    ]
}
```

### 字段解释

| 字段 | 必需 | 含义 |
| --- | --- | --- |
| `$schema` | 否 | JSON Schema URL，让 IDE 能补全/校验 |
| `name` | 是 | 项目名（小写、可含 `-`），vcpkg 内部唯一标识 |
| `version-string` | 否（建议有） | 项目版本，对 vcpkg 决策无影响，仅元数据 |
| `description` | 否 | 一行描述 |
| `dependencies` | 是 | 依赖列表，可以是字符串或对象 |

### dependencies 的两种写法

**简单字符串**（本仓库用法）：

```json
"dependencies": [ "gtest" ]
```

vcpkg 装最新 baseline 版本。

**对象写法**（带 features / 平台限定）：

```json
"dependencies": [
    {
        "name": "boost-asio",
        "features": ["ssl"],
        "platform": "!windows"
    },
    {
        "name": "fmt",
        "version>=": "10.0.0"
    }
]
```

字段：

- `features`：启用包的可选特性
- `platform`：仅在某平台安装（vcpkg 自己的平台表达式语法）
- `version>=`：最低版本约束（需要配合 baseline，详见下节）

### 锁文件：baseline 与 vcpkg-configuration.json

vcpkg 默认按 vcpkg root 当前 commit 决定包版本（commit 即 baseline）。如果想锁版本：

```json
{
    "name": "moderncpp",
    "dependencies": [ "gtest" ],
    "builtin-baseline": "5ee5eee0d3e9c6098b24d263e9099edcdcef6631"
}
```

或单独建 `vcpkg-configuration.json` 指定 registry baseline。

**本仓库未启用 baseline**，原因：教学项目用最新版有助于体验新特性；CI 通过
[`lukka/run-vcpkg@v11`](../.github/workflows/ci.yml#L86) 的 `vcpkgGitCommitId` 已经固定
vcpkg 版本，等效于 baseline。

### 与 classic 模式的差异

```bash
# classic 模式：手动安装到 vcpkg root（全局共享）
vcpkg install gtest

# manifest 模式：cmake configure 时自动按 vcpkg.json 装到本项目 build 目录
cmake --preset gcc-debug
# vcpkg 自动安装到 build/gcc-debug/vcpkg_installed/x64-linux/
```

manifest 模式的优势：

- 依赖随源码进版本管理（`vcpkg.json` 提交，`vcpkg_installed/` 不提交）
- 不同项目装不同版本不互相污染
- CI 与本地装的就是 `vcpkg.json` 声明的版本

---

## 4. toolchain 文件的工作机制

vcpkg 通过 CMake 的 `CMAKE_TOOLCHAIN_FILE` 钩子接入项目。机制详见
[cmake-presets-guide.md §8 toolchain hook 机制](cmake-presets-guide.md#8-toolchain-hook-机制)。
本节聚焦 vcpkg 的具体行为。

### `vcpkg.cmake` 在 configure 时干什么

依次执行：

1. **检测模式**：当前目录是否有 `vcpkg.json` → manifest 模式
2. **决定 triplet**：从 `VCPKG_TARGET_TRIPLET` cache 变量、`VCPKG_DEFAULT_TRIPLET`
   环境变量、主机自动检测三处依次取（详见 [§5](#5-tripletabi-的命名空间)）
3. **解析 vcpkg.json**：列出所有 dependencies
4. **安装缺失依赖**：调用 `vcpkg install`，输出装到
   `<binaryDir>/vcpkg_installed/<triplet>/`
5. **注入 find_package 路径**：把上述目录加进 `CMAKE_PREFIX_PATH`，让后续
   `find_package(GTest CONFIG)` 能找到

第 4 步是首次 configure 慢的原因 —— 第一次跑 `cmake --preset gcc-debug` 可能等几分钟，
就是 vcpkg 在编译 gtest。第二次起，binary 已经在 `vcpkg_installed/` 里，秒过。

### `find_package(GTest CONFIG)` 是怎么命中的

本仓库 [`cmake/Dependencies.cmake`](../cmake/Dependencies.cmake)：

```cmake
find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
    find_package(GTest MODULE QUIET)
endif()
```

- `CONFIG` 模式查找 `<triplet>/share/gtest/GTestConfig.cmake` —— vcpkg 装的包附带这个
- `MODULE` 模式找 `FindGTest.cmake` —— 系统包管理器装的 gtest 走这个
- 两个都找不到，FATAL_ERROR 提示用户装 vcpkg 或 Conan

vcpkg 装的 gtest 走 `CONFIG` 路径，靠 toolchain 文件把
`build/<preset>/vcpkg_installed/<triplet>/share/gtest/` 加进 `CMAKE_PREFIX_PATH` 来命中。

---

## 5. triplet：ABI 的命名空间

### 什么是 triplet

triplet 是 vcpkg 用一个字符串描述的"目标 ABI"。命名通常是
`<架构>-<平台>[-<链接方式>][-<CRT>]`：

| triplet | 架构 | 平台 | 链接 | CRT |
| --- | --- | --- | --- | --- |
| `x64-linux` | x86_64 | Linux (glibc) | dynamic | — |
| `arm64-linux` | aarch64 | Linux | dynamic | — |
| `x64-osx` | x86_64 | macOS | dynamic | — |
| `arm64-osx` | aarch64 | macOS (Apple Silicon) | dynamic | — |
| `x64-windows` | x86_64 | Windows MSVC | dynamic | /MD（动态 CRT） |
| `x64-windows-static` | x86_64 | Windows MSVC | static | /MT（静态 CRT） |
| `x64-mingw-dynamic` | x86_64 | Windows MinGW | dynamic | UCRT |
| `x64-mingw-static` | x86_64 | Windows MinGW | static | UCRT |

triplet 决定了 vcpkg 装出来的二进制是什么样的。**triplet 与你将要链接它的代码必须 ABI
一致**，否则一堆 `undefined reference` 或链接错乱。

### 本仓库的 triplet 策略（PR #5 后）

| 主机 | preset | VCPKG_TARGET_TRIPLET 来源 |
| --- | --- | --- |
| Linux x64 / ARM64 | `gcc-*` / `clang-*` | **vcpkg 主机自动检测** |
| macOS Intel / Apple Silicon | `clang-*` | **vcpkg 主机自动检测** |
| Windows MSVC | `msvc` | preset 固化为 `x64-windows` |
| Windows clang-cl | `clang-cl-*` | preset 固化为 `x64-windows` |
| Windows MinGW | `mingw-gcc-*` / `mingw-clang-*` | preset 内固化为 `x64-mingw-dynamic` |

### 为什么 Linux/macOS 不固化 triplet

每个 OS 上只有一种主流 ABI：

- Linux：glibc + libstdc++（对应 `x64-linux` / `arm64-linux`）
- macOS：libc++（对应 `x64-osx` / `arm64-osx`）

vcpkg 看主机 OS + `CMAKE_HOST_SYSTEM_PROCESSOR` 就能正确决定。**硬编码反而会让 ARM64
Linux、Apple Silicon 等用户拿到错的二进制**。

历史上仓库曾经在 `_gcc` / `_clang` 上硬编码 `x64-linux`（PR #3），后被代码评审指出
退化了主机自动检测，PR #5 改回让 vcpkg 自己决定。

### 为什么 Windows 必须固化

Windows 上 ABI 不唯一：MSVC ABI 和 MinGW ABI 共存。vcpkg 默认猜 `x64-windows`（MSVC
ABI），对 MSVC / clang-cl 用户是对的，对 MinGW 用户是错的。**所以 Windows 上每一组
preset 都把自己的 triplet 显式固化在 `cacheVariables` 里**：`msvc-*` / `clang-cl-*`
固化 `x64-windows`，`mingw-gcc-*` / `mingw-clang-*` 固化 `x64-mingw-dynamic`，按编译器
选 preset 即可（详见 [§6](#6-mingw-专题)）。

### 自定义 triplet

vcpkg 允许在仓库里放自定义 triplet（`.cmake` 文件），通过
`VCPKG_OVERLAY_TRIPLETS` 启用。本仓库未使用此机制 —— 内置 triplet 已经够。

---

## 6. MinGW 专题

### 问题陈述

MinGW（msys2 ucrt64 / mingw64 环境）在 Windows 上提供 GNU 工具链：

- 用 GNU 的 name mangling 与 ABI
- 链接 libstdc++（不是 MSVC 的 STL）
- 异常处理走 SEH/Dwarf（MinGW 实现，不是 MSVC 的）

而 vcpkg 在 Windows 上默认用 `x64-windows` triplet，装出 **MSVC ABI + MSVC STL** 的
二进制。把这种 gtest 拿去给 MinGW g++ 链接：

```
undefined reference to `testing::AssertionResult::AssertionResult(...)'
undefined reference to `testing::Test::SetUp()'
... (一长串)
```

符号 mangling 完全对不上、STL 类型布局不一样。链接必然失败。

### 为什么 vcpkg 不"看编译器再决定 triplet"

vcpkg 的 triplet 决策发生在 **CMake 知道编译器之前**：你跑 `cmake --preset` 时，CMake
先加载 toolchain 文件，vcpkg 这时只有环境变量和命令行参数能看，还没进到
`project()` 检测编译器那一步。所以它只能按主机 OS 给个保守默认。

### 解决方案：内置 mingw-* preset

仓库 `_gcc` / `_clang` 这些 Linux/macOS preset 在 Windows 上被 condition 门控（详见
[cmake-presets-guide.md §5](cmake-presets-guide.md#5-condition让-preset-跨平台共存)），
直接用不了。所以仓库为 Windows 上的 GNU 工具链单独提供了两组 preset：

- `mingw-gcc-{debug,release,relwithdebinfo,minsizerel}`
- `mingw-clang-{debug,release,relwithdebinfo,minsizerel}`

它们与 Linux 侧的 `gcc-*` / `clang-*` 互不干扰：

- 都用 Ninja 单配置生成器
- 都把 `VCPKG_TARGET_TRIPLET` 固化为 `x64-mingw-dynamic`
- 都用 `${hostSystemName} == Windows` 条件门控（macOS / Linux 上不会出现在
  `cmake --list-presets` 里）
- 编译器分别用 `gcc`/`g++` 与 `clang`/`clang++`（依赖你在 MSYS2 UCRT64 shell 里跑，
  或把 `<msys2>/ucrt64/bin` 放进 PATH，让这些短名字解析到 ucrt64 工具链）

### 用法

```bash
# 在 MSYS2 UCRT64 shell 里：
export VCPKG_ROOT=/d/path/to/vcpkg
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug
ctest --preset mingw-gcc-debug
```

切到 MinGW Clang：把 `mingw-gcc-` 换成 `mingw-clang-`。

### 备选方案 1：configure 时手动 -D

如果你已经在用别的"基础" preset（例如自定义的 `_my-base`），可以临时 `-D` 覆盖：

```bash
cmake --preset mingw-gcc-debug -DVCPKG_TARGET_TRIPLET=x64-mingw-static
```

### 备选方案 2：CMakeUserPresets.json 叠层

要长期使用非 `x64-mingw-dynamic` 的 triplet（例如 `x64-mingw-static`，或自定义的
overlay triplet），可以在仓库根建一份 `CMakeUserPresets.json`（已 .gitignored），
继承 `mingw-gcc-debug` 等 leaf 并覆盖 `cacheVariables.VCPKG_TARGET_TRIPLET`：

```json
{
    "version": 6,
    "cmakeMinimumRequired": { "major": 3, "minor": 25, "patch": 0 },
    "configurePresets": [
        {
            "name": "my-mingw-static-debug",
            "inherits": "mingw-gcc-debug",
            "cacheVariables": { "VCPKG_TARGET_TRIPLET": "x64-mingw-static" }
        }
    ]
}
```

UserPresets 的"同名字段浅合并、不同名 preset 直接追加"规则见
[cmake-presets-guide.md §9](cmake-presets-guide.md#9-cmakeuserpresetsjson本地叠层)。

---

## 7. binary cache：加速重复构建

### 为什么 vcpkg 慢

vcpkg 默认从源码编译。装一个 boost 可能要 20 分钟。CI 跑一次就要 20 分钟全用在 vcpkg
上，本地切 preset 也是 20 分钟一次。

**解决方案：binary cache** —— 把编译结果按 ABI hash 存起来，下次同 ABI 的依赖直接
解压，秒过。

### binary cache 的 hash 来源

vcpkg 用以下因子计算 hash：

- triplet
- 包版本 + features
- 编译器版本
- vcpkg 自身版本（commit）
- 关键工具链选项

只要 hash 一致，就直接用 cache。

### 几种 binary cache 后端

| 后端 | 适用 |
| --- | --- |
| **本地目录** | 单机多项目共享（默认在 `~/.cache/vcpkg/archives/`） |
| **NuGet** | 团队内网共享（含 GitHub Packages、Azure Artifacts） |
| **`x-gha`** | GitHub Actions 内置 cache |
| **HTTP** | 自建 HTTP server |

### 本仓库 CI 的配置

```yaml
env:
  VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"
```

字段含义：

- `clear`：清空默认源（避免回退到本地 cache 浪费时间）
- `x-gha,readwrite`：用 GitHub Actions cache，可读可写

每个 job 跑完会把新 cache 写入 GHA cache，后续 PR 命中就秒过。

详细配置（GHA cache token 注入、与 `lukka/run-vcpkg@v11` 的衔接）见
[ci-guide.md §6 缓存与加速](ci-guide.md#6-缓存与加速)。

### 本地启用 binary cache

```bash
export VCPKG_BINARY_SOURCES="default,readwrite"
```

`default` 即本地目录后端，路径默认 `~/.cache/vcpkg/archives/`。多项目共享同一份缓存。

---

## 8. MSVC CRT 对齐

### 问题

MSVC 的 CRT（C 运行时）有两类：

- **/MD**（动态 CRT，msvcrt.dll）：本仓库默认
- **/MT**（静态 CRT，链进 .exe）

**gtest 默认用静态 CRT**（`gtest_force_shared_crt: OFF`）。如果你的项目用动态 CRT 链
gtest 的静态 CRT 版本，会报 LNK2038 错乱：

```
error LNK2038: mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease'
doesn't match value 'MD_DynamicRelease' in main.obj
```

### 本仓库的解决方案

[`cmake/Dependencies.cmake`](../cmake/Dependencies.cmake)：

```cmake
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
```

强制 gtest 用动态 CRT，与本项目的 `/MD` 默认对齐。

### 与 triplet 的关系

不同 triplet 自带不同 CRT 默认：

- `x64-windows`：`/MD`（动态 CRT，与本仓库默认一致）
- `x64-windows-static`：`/MT`（静态 CRT，需要项目把 `CMAKE_MSVC_RUNTIME_LIBRARY` 改为
  `MultiThreaded`）
- `x64-windows-static-md`：`/MD` 但其他依赖静态 —— 中间方案

本仓库选 `x64-windows`，所以 `gtest_force_shared_crt: ON` 是正确选择。如果哪天换
`x64-windows-static`，要把这一行改为 `OFF` 并同步设 `CMAKE_MSVC_RUNTIME_LIBRARY`。

### Conan 下的等价问题

Conan 用 `compiler.runtime` 字段表达同样的概念。详见
[conan-guide.md §10](conan-guide.md#10-常见问题与排查)。

---

## 9. 常用命令速查

### manifest 模式（本仓库工作流）

```bash
# configure 时 vcpkg 自动安装依赖
cmake --preset gcc-debug

# 强制重新解析 vcpkg.json（修改了依赖列表后）
cmake --preset gcc-debug --fresh

# 看 vcpkg 装到了哪
ls build/gcc-debug/vcpkg_installed/x64-linux/share/
```

### 直接调用 vcpkg

```bash
# 看仓库依赖树
cd build/gcc-debug && ../../../$VCPKG_ROOT/vcpkg list

# 搜索某个包
$VCPKG_ROOT/vcpkg search fmt

# 升级 vcpkg 自身（影响包版本）
cd $VCPKG_ROOT && git pull && ./bootstrap-vcpkg.sh
```

### 排查时的 verbose 选项

```bash
# vcpkg 操作的详细日志
cmake --preset gcc-debug -DVCPKG_INSTALL_OPTIONS="--debug"

# vcpkg 装某个包失败时，build log 在
ls build/gcc-debug/vcpkg-bootstrap.log
ls $VCPKG_ROOT/buildtrees/<port-name>/
```

---

## 10. 常见问题与排查

### "Could not find toolchain file: vcpkg.cmake"

原因：

1. **`VCPKG_ROOT` 未设**：跑 `echo $VCPKG_ROOT`（Linux/Mac）或 `echo %VCPKG_ROOT%`
   （Windows）确认
2. **设了但未在当前 shell 生效**：Linux 要 `source ~/.bashrc`；Windows 的 `setx`
   只对新开 shell 生效，重开终端
3. **路径里有空格未 quote**：用环境变量持久化，避免 inline 路径

### MinGW 链接失败：`undefined reference to testing::...`

100% 是 triplet 不匹配 —— vcpkg 装的是 `x64-windows`（MSVC ABI），但你用 MinGW gcc
链接。修法见 [§6 MinGW 专题](#6-mingw-专题)。

### vcpkg 拉源码失败（HTTP 502 / git clone failed）

GitHub 偶尔抖动。重新跑一次 `cmake --preset` 通常就好。CI 上可以用
`gh run rerun --failed` 重试。如果反复失败，检查 `$VCPKG_ROOT/buildtrees/` 下的具体
port log，看是不是某个 port 的上游源不稳定。

### 切了 preset 后 vcpkg 重新装一遍

不同 preset 的 `binaryDir` 不同，`vcpkg_installed/` 也不同。第一次切是会重装。开了
binary cache（[§7](#7-binary-cache加速重复构建)）后，重装是从 cache 解压，秒过。

### 升级 vcpkg 后版本意外变化

vcpkg 的"包版本"由 vcpkg root 的 commit 决定。`git pull` 一下 vcpkg root，gtest 可能
就从 1.15.0 跳到 1.16.0。要锁版本：

- 设 `builtin-baseline` 在 `vcpkg.json` 里
- 或 CI 用 `lukka/run-vcpkg@v11` 的 `vcpkgGitCommitId` 锁 vcpkg commit

### MSVC 链接 LNK2038（CRT 不匹配）

详见 [§8 MSVC CRT 对齐](#8-msvc-crt-对齐)。本仓库 `gtest_force_shared_crt: ON` 已经
解决，触发这个错通常是你换了 `x64-windows-static` triplet 但没同步改 CMake 的
runtime library 设置。

### `vcpkg_installed/` 占空间太大

每个 preset 的 `build/<preset>/vcpkg_installed/` 都是一份完整的依赖。多 preset 切来切
去会越攒越多。清理：删掉 `build/` 下不再用的 preset 目录即可，不影响 vcpkg root 与
binary cache。
