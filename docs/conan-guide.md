# Conan 使用与配置完整指南

> 本文档系统讲解 Conan 2.x 的设计理念、profile 概念、conanfile.txt 配置、CMakeDeps 与
> CMakeToolchain generator、cmake_layout，以及本仓库 `conanfile.txt` 是如何与 CMake
> Presets 衔接的。
>
> 配套文件：[`conanfile.txt`](../conanfile.txt) / [`cmake/Dependencies.cmake`](../cmake/Dependencies.cmake)
>
> 配套阅读：
> - [`docs/cmake-presets-guide.md`](cmake-presets-guide.md) —— preset 与 toolchain hook 机制
> - [`docs/vcpkg-guide.md`](vcpkg-guide.md) —— 平行的另一种方案
> - [`docs/ci-guide.md`](ci-guide.md) —— CI 当前未集成 Conan，本文 §9 说明原因

---

## 目录

1. [什么是 Conan，它解决什么问题](#1-什么是-conan它解决什么问题)
2. [安装与 profile 初始化](#2-安装与-profile-初始化)
3. [conanfile.txt 详解](#3-conanfiletxt-详解)
4. [CMakeDeps 与 CMakeToolchain 两个 generator](#4-cmakedeps-与-cmaketoolchain-两个-generator)
5. [cmake_layout 与 per-preset 输出目录](#5-cmake_layout-与-per-preset-输出目录)
6. [本仓库工作流](#6-本仓库工作流)
7. [编译器矩阵下的 preset 设计](#7-编译器矩阵下的-preset-设计)
8. [profile 进阶](#8-profile-进阶)
9. [与 CI 的集成（前瞻）](#9-与-ci-的集成前瞻)
10. [常见问题与排查](#10-常见问题与排查)

---

## 1. 什么是 Conan，它解决什么问题

**Conan** 是 JFrog 主导的 C/C++ 包管理器。Conan 2.x 与 1.x 是不兼容的两套，本仓库
（与本文档）只讲 **Conan 2.x**。

Conan 的核心理念：用 Python 写的 **recipe**（`conanfile.py`）描述如何构建一个包，用
**profile** 描述构建环境（编译器、build_type、libcxx ABI 等），二者结合产生一个
**package_id** 索引到具体的二进制。Conan 默认会先去远端 binary repo 拉对应
package_id 的二进制，没有才本地 build。

### vcpkg vs Conan 对比表

> 这是本仓库选 vcpkg+Conan 双轨支持的对照表，[vcpkg-guide.md §1](vcpkg-guide.md#1-什么是-vcpkg它解决什么问题)
> 反向引用此处。

| 维度 | vcpkg | Conan |
| --- | --- | --- |
| 包格式 | port（CMake/PowerShell 脚本） | recipe（Python，`conanfile.py`） |
| 二进制分发 | 默认无（源码编译，可选 binary cache） | **默认有**（远端有 prebuilt 直接拉） |
| ABI 描述 | triplet 字符串（`x64-windows-static`） | profile（多字段：compiler/version/libcxx/build_type/...） |
| 项目清单 | `vcpkg.json`（manifest 模式） | `conanfile.txt` 或 `conanfile.py` |
| 与 CMake 衔接 | 单一 toolchain `vcpkg.cmake`，cmake configure 一步触发 | 两个 generator（toolchain + deps），需先 `conan install` |
| 版本范围语法 | `version>=` 等基础约束 | 完整范围语法（`[>=1.10 <2.0]` 等） |
| 多版本共存 | 通过 overlay ports | 内置（同一包多 version 可共存于 cache） |
| 学习曲线 | 较低（manifest + 一个 toolchain 就懂） | 较高（profile/recipe/cache 三个新概念） |

一句话定位：**vcpkg 像 apt（编译型）；Conan 像 Maven（recipe + 二进制分发）**。

### 为什么本仓库同时支持两种

教学项目，目的是让读者**体验两种生态**。日常工作选一个就好：

- 团队内 C++ 项目、Windows 居多 → vcpkg 更顺
- 跨语言（含 C++）大型项目、有 binary repo → Conan 更顺

---

## 2. 安装与 profile 初始化

### 安装 Conan 2.x

```bash
# pip 是官方推荐方式
pip install --upgrade conan

# 验证版本（必须 ≥ 2.0）
conan --version
# Conan version 2.x.x
```

> 不要用 `conan` 1.x 的资料 —— 1.x 与 2.x 命令、配置、generator 全不兼容。判定标准：
> 命令 `conan profile detect` 在 1.x 不存在。

### 首次 profile 初始化

```bash
conan profile detect
# Found gcc 13
# gcc>=11 && libstdc++ -> setting libstdc++11
# Detected profile:
# [settings]
# arch=x86_64
# build_type=Release
# compiler=gcc
# compiler.cppstd=gnu17
# compiler.libcxx=libstdc++11
# compiler.version=13
# os=Linux
```

`conan profile detect` 检测当前主机环境，写入默认 profile（位置见
`conan profile path default`）。这个 profile 会作为后续 `conan install` 的默认
**host profile**。

### profile 文件结构

profile 是 INI 格式：

```ini
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=gnu17
compiler.libcxx=libstdc++11
compiler.version=13
os=Linux

[options]
# 包级 options 覆盖

[buildenv]
# 构建期环境变量

[runenv]
# 运行期环境变量

[conf]
# 配置项（tools.cmake.cmaketoolchain:generator 等）
```

### profile 与 vcpkg triplet 的对照

| vcpkg triplet | Conan settings 等价 |
| --- | --- |
| `x64-linux` | `arch=x86_64`, `os=Linux`, `compiler.libcxx=libstdc++11` |
| `arm64-linux` | `arch=armv8`, `os=Linux`, `compiler.libcxx=libstdc++11` |
| `x64-windows` | `arch=x86_64`, `os=Windows`, `compiler=msvc`, `compiler.runtime=dynamic` |
| `x64-mingw-dynamic` | `arch=x86_64`, `os=Windows`, `compiler=gcc`, `compiler.libcxx=libstdc++11` |
| `x64-osx` | `arch=x86_64`, `os=Macos`, `compiler.libcxx=libc++` |

triplet 用一个字符串编码，profile 用多字段表达 —— **profile 表达力更强**（可分
host/build profile 做交叉编译，可分 compiler.libcxx 与 libcxx ABI 等），代价是新概念
更多。

---

## 3. conanfile.txt 详解

本仓库的 `conanfile.txt` 全文：

```ini
[requires]
gtest/1.15.0

[generators]
CMakeDeps
CMakeToolchain

[layout]
cmake_layout
```

### 字段解释

#### `[requires]`：依赖列表

```ini
[requires]
gtest/1.15.0
fmt/[>=10.0 <11.0]
boost/1.84.0
```

- 一行一个依赖，格式 `name/version`
- 版本可以是精确版本（`1.15.0`）或范围（`[>=10.0 <11.0]`、`[*]`）
- 不写 version Conan 会拒绝（与 vcpkg 不同，vcpkg 不写就用 baseline 默认）

#### `[generators]`：CMake 集成 generator

| Generator | 产物 | 作用 |
| --- | --- | --- |
| `CMakeDeps` | `<pkg>-config.cmake` 等 | 让 `find_package(GTest CONFIG)` 找到 |
| `CMakeToolchain` | `conan_toolchain.cmake` | 把 profile 翻译成 CMake 变量 |

详见 [§4](#4-cmakedeps-与-cmaketoolchain-两个-generator)。

#### `[layout]`：输出目录布局

`cmake_layout` 是 Conan 内置的标准布局，详见 [§5](#5-cmake_layout-与-per-preset-输出目录)。

#### 其他字段（本仓库未用）

```ini
[options]
boost*:shared=True

[build_requires]
cmake/3.27.0

[tool_requires]
ninja/1.11.0

[test_requires]
catch2/3.4.0
```

- `[options]`：覆盖包的 options
- `[build_requires]` / `[tool_requires]`：构建时才需要的工具（cmake / ninja 等）
- `[test_requires]`：测试时才用的依赖

### conanfile.py vs conanfile.txt

Conan 支持两种格式：

- **conanfile.txt**：INI 格式，简单声明，适合"我只是消费别人的包"
- **conanfile.py**：Python 类，能写自定义 build 逻辑，适合"我要自己产出包"

**本仓库选 `.txt`**，因为它是消费方（不发布包），能少一个 Python 类的学习成本。

---

## 4. CMakeDeps 与 CMakeToolchain 两个 generator

vcpkg 用一份 `vcpkg.cmake` 同时干"toolchain 注入"和"find_package 路径"两件事。Conan
拆成两个 generator，关注点分离更清晰。

### CMakeToolchain → `conan_toolchain.cmake`

`conan install` 会按 profile 生成一份 `conan_toolchain.cmake`，内容大致：

```cmake
# 生成的，不是手写
set(CMAKE_C_COMPILER "/usr/bin/gcc-13" CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-13" CACHE FILEPATH "")
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
set(CMAKE_CXX_STANDARD 23)
# 把 Conan 的 install 目录加进 CMAKE_PREFIX_PATH
list(PREPEND CMAKE_PREFIX_PATH "/path/to/build/Release/generators")
# ...
```

它的作用：**把 profile 里 `[settings]` 翻译成 CMake 的对应变量**，再把 Conan 安装的
依赖目录注册到 `CMAKE_PREFIX_PATH`。

通过 preset 的 `CMAKE_TOOLCHAIN_FILE` 指向它（详见
[cmake-presets-guide.md §8](cmake-presets-guide.md#8-toolchain-hook-机制)）：

```json
"CMAKE_TOOLCHAIN_FILE": "${sourceDir}/build/${presetName}/conan_toolchain.cmake"
```

**本仓库 `conanfile.txt` 故意不写 `[layout] cmake_layout`**：cmake_layout 会把生成
的 toolchain 路径变成 `<output-folder>/build/<BuildType>/generators/conan_toolchain.cmake`，
带"build_type 子目录"——而 CMake preset 的 cache 变量是固定字符串、不能按 build_type
动态变化（一个 leaf preset 已经隐含一个 build_type，但路径表达式没法用）。去掉
`cmake_layout` 后 Conan 默认行为是把 toolchain 直接写在 `--output-folder` 根，
preset 路径 `${sourceDir}/build/${presetName}/conan_toolchain.cmake` 一行到位。

### CMakeDeps → `<pkg>-config.cmake`

每个 require 包会生成一个 `<pkg>-config.cmake` 文件，让 `find_package(<Pkg> CONFIG)`
能找到。例如 gtest 会生成：

```
build/<preset>/generators/
  GTestConfig.cmake
  GTestConfigVersion.cmake
  GTestTargets.cmake
  GTestTargets-release.cmake
```

`find_package(GTest CONFIG)` 看到这些文件，就能创建 `GTest::gtest` 等 imported target。

本仓库 [`cmake/Dependencies.cmake`](../cmake/Dependencies.cmake) 的查找逻辑：

```cmake
find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
    find_package(GTest MODULE QUIET)
endif()
```

vcpkg 与 Conan 都生成 `CONFIG` 文件，**同一份 `find_package` 代码二者都能命中**，这正
是双轨支持的基础。

### 与 vcpkg 单 toolchain 的设计差异

| | vcpkg | Conan |
| --- | --- | --- |
| 文件数 | 1（`vcpkg.cmake`） | 2+（`conan_toolchain.cmake` + 每个包一个 config） |
| 职责 | toolchain + 依赖搜索 | toolchain（CMakeToolchain）+ 依赖搜索（CMakeDeps），两者拆开 |
| 触发安装 | configure 时 | **必须先 `conan install` 命令** |
| 修改时机 | 改 `vcpkg.json` 后下次 configure 即生效 | 改 `conanfile.txt` 后必须重跑 `conan install` |

---

## 5. cmake_layout 与 per-preset 输出目录

### `cmake_layout` 的默认布局

Conan 提供几个内置 layout 函数。`cmake_layout` 是最常见的：

```
build/
  <build_type>/
    generators/        ← CMakeDeps + CMakeToolchain 输出在这里
      conan_toolchain.cmake
      GTestConfig.cmake
      ...
```

构建类型在路径里，所以同一项目多 build_type 不冲突。

### 本仓库的选择：per-preset 输出目录 + 不用 cmake_layout

本仓库希望每个 preset 一个独立 build 目录、且 toolchain 路径**不带 build_type 子层**
（preset cache 变量没法按 build_type 动态展开），所以 `conanfile.txt` 不写 `[layout]`，
配合 `--output-folder=build/<preset>` 让 Conan 把 toolchain 直接写在 preset 同根：

```bash
conan install . --output-folder=build/gcc-debug-conan --build=missing
```

`--output-folder` 让 Conan 把 toolchain / `<Pkg>Config.cmake` 等 generators 直接写到
`build/gcc-debug-conan/`（无 `generators/` 子目录，因为本仓库的 `conanfile.txt`
不带 `[layout] cmake_layout`），与 preset 的 `binaryDir` 完全对齐。

然后 preset 把 toolchain 路径写死成：

```json
"CMAKE_TOOLCHAIN_FILE": "${sourceDir}/build/${presetName}/conan_toolchain.cmake"
```

`${presetName}` 会展开成 `gcc-debug-conan`，正好命中 `conan install` 写入的
`generators/` 子目录。

### 为什么 `_conan` preset 的 toolchain 路径写死

```json
{
    "name": "_conan",
    "hidden": true,
    "cacheVariables": {
        "CMAKE_TOOLCHAIN_FILE": "${sourceDir}/build/${presetName}/conan_toolchain.cmake"
    }
}
```

写死带来一个**强制约定**：用户必须先跑 `conan install . --output-folder=build/<presetName>`
才能跑 `cmake --preset <preset>`。文件不存在 cmake 报错很直白：

```
CMake Error: Could not find toolchain file:
.../build/gcc-debug-conan/generators/conan_toolchain.cmake
```

错误信息里直接告诉你少了 `conan install` 这一步。这比 vcpkg 的"configure 慢慢卡住装包"
更显式。

详细机制见 [cmake-presets-guide.md §8 toolchain hook 机制](cmake-presets-guide.md#8-toolchain-hook-机制)。

---

## 6. 本仓库工作流

Conan 的核心工作流是**三步走**：

```
┌──────────────────────────────────┐
│ ① conan install                  │  ← 按 profile 生成 toolchain + 装依赖
│   --output-folder=build/<preset> │
│   --build=missing                │
└────────────┬─────────────────────┘
             │ 产物：conan_toolchain.cmake / GTestConfig.cmake
             ▼
┌──────────────────────────────────┐
│ ② cmake --preset <preset>        │  ← preset 读上一步生成的 toolchain
└────────────┬─────────────────────┘
             │ 产物：CMakeCache.txt / build.ninja / *.sln
             ▼
┌──────────────────────────────────┐
│ ③ cmake --build --preset         │  ← 编译
│   ctest --preset                 │  ← 测试
└──────────────────────────────────┘
```

### 完整命令示例（GCC Debug）

```bash
# 第一步：让 Conan 装依赖到 build/gcc-debug-conan/
conan install . \
    --output-folder=build/gcc-debug-conan \
    --build=missing \
    -s build_type=Debug \
    -s compiler.cppstd=23

# 第二步：cmake 用上一步生成的 toolchain
cmake --preset gcc-debug-conan

# 第三步：构建 + 测试
cmake --build --preset gcc-debug-conan
ctest --preset gcc-debug-conan
```

### 关键参数

#### `--build=missing`

Conan 默认只用远端 prebuilt 二进制。如果远端没有当前 profile 对应的二进制（例如你的
`compiler.version=13` 但远端只有 12 的），会失败。`--build=missing` 表示"远端没就本地
build"，是日常开发的安全默认。

#### `-s build_type=Debug`

覆盖 profile 里的 `build_type`。本仓库需要这一行，因为 profile 默认 Release。

> ⚠️ **必须与 preset 的 `CMAKE_BUILD_TYPE` 完全一致**。`gcc-debug-conan` 用
> `-s build_type=Debug`，`gcc-relwithdebinfo-conan` 用 `-s build_type=RelWithDebInfo`，
> 类推。CMakeDeps 只为请求的 build_type 生成 per-config 目标文件
> （`GTest-Target-<lower>.cmake` + `GTest-<lower>-x86_64-data.cmake`）；mismatch
> 时 `find_package(GTest)` 仍然返回成功（`GTest::gtest_main` target 在
> `GTestTargets.cmake` 里被无条件 `add_library(... INTERFACE IMPORTED)`），但
> `INTERFACE_INCLUDE_DIRECTORIES` 被 `$<$<CONFIG:OtherType>:...>` 包起来，对当前
> build_type 求值为空 —— configure 通过、build 时炸 `gtest/gtest.h: No such file`。
> `cmake/Dependencies.cmake` 已经加了一道 configure 期检查捕获这种情况，撞上时会
> 直接 FATAL_ERROR 提示重跑命令。

#### `-s compiler.cppstd=23`

覆盖 C++ 标准。本仓库要 C++23，profile 默认可能是 17。

### 为什么 vcpkg 一步、Conan 必须两步

| 阶段 | vcpkg | Conan |
| --- | --- | --- |
| 决定要装啥 | `vcpkg.json` | `conanfile.txt` + profile + 命令行 settings |
| 触发安装 | CMake configure 时 toolchain 自动跑 | **`conan install` 命令显式跑** |
| 安装完产物 | `vcpkg_installed/` | `build/<preset>/conan_toolchain.cmake` + 同目录下的 `<Pkg>Config.cmake` 等 |
| CMake 使用 | toolchain 注册路径 | toolchain 注册路径 + Deps 文件被 `find_package` 命中 |

vcpkg 把"装包"塞进 toolchain 的 side effect；Conan 把它独立成命令。Conan 的方式让"装
依赖"与"配置 CMake"两个动作显式分离，便于排查。

### per-build-type 还是 per-preset 输出目录的权衡

Conan 默认布局是 `build/<build_type>/`，本仓库覆盖成 `build/<presetName>/`。原因：

- 默认布局：Debug / Release 各一份，不区分编译器；切编译器要小心
- per-preset：每个 preset 完全隔离，gcc-debug 和 clang-debug 互不影响

代价：每个 preset 都要单独 `conan install`（无法复用），但 Conan 有 cache，重复装不会
真的重新编译。

---

## 7. 编译器矩阵下的 preset 设计

仓库 `_gcc-conan` / `_clang-conan` 与 vcpkg 版的 `_gcc` / `_clang` 一一对应，区别只有
toolchain：

```json
{
    "name": "_gcc-conan",
    "hidden": true,
    "inherits": ["_conan", "_base"],
    "generator": "Ninja",
    "cacheVariables": {
        "CMAKE_C_COMPILER": "gcc",
        "CMAKE_CXX_COMPILER": "g++"
    },
    "condition": {
        "type": "equals",
        "lhs": "${hostSystemName}",
        "rhs": "Linux"
    }
}
```

继承链与 vcpkg 完全镜像（详见
[cmake-presets-guide.md §4 本仓库 preset 全景图](cmake-presets-guide.md#4-本仓库-preset-全景图)）：

```
_base + _conan → _gcc-conan         → gcc-{debug,release,relwithdebinfo,minsizerel}-conan
              → _clang-conan        → clang-{debug,release,relwithdebinfo,minsizerel}-conan
              → _clang-cl-conan     → clang-cl-{debug,release,relwithdebinfo,minsizerel}-conan
              → _mingw-gcc-conan    → mingw-gcc-{debug,release,relwithdebinfo,minsizerel}-conan
              → _mingw-clang-conan  → mingw-clang-{debug,release,relwithdebinfo,minsizerel}-conan
              → msvc-conan          (VS 2022 multi-config)
                  build/test: msvc-{debug,release,relwithdebinfo,minsizerel}-conan
              → ninja-mc-msvc-conan (Ninja Multi-Config + cl.exe)
                  build/test: ninja-mc-msvc-{debug,release,relwithdebinfo,minsizerel}-conan
```

`*-conan` preset 与 vcpkg 一侧（无 `-conan` 后缀）一一对应、四种 build type 全覆盖。

### 为什么 `*-conan` preset 不固化 triplet

Conan **完全不读** `VCPKG_TARGET_TRIPLET`。Conan 用 `[settings]` 表达 ABI，从 profile
里读，与 cmake cache 变量无关。所以 conan preset 即使写了 triplet 也没用，**本仓库
干脆不写**。

### MinGW 在 Conan 下的处理

vcpkg 下仓库为 MinGW 提供专门的 `mingw-{gcc,clang}-*` preset 固化 triplet（详见
[vcpkg-guide.md §6](vcpkg-guide.md#6-mingw-专题)）。Conan 下同样有平行的
`mingw-{gcc,clang}-*-conan` preset，但 ABI 不靠 triplet 表达，而是靠 profile：

```bash
# 一个针对 MinGW 的 profile（保存为 ~/.conan2/profiles/mingw-ucrt64）
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=gnu23
compiler.libcxx=libstdc++11
compiler.version=13
os=Windows
```

```bash
conan install . -pr=mingw-ucrt64 --output-folder=build/mingw-gcc-debug-conan --build=missing
cmake --preset mingw-gcc-debug-conan
```

`-pr=` 指定 profile，覆盖默认；`--output-folder` 必须与目标 preset 名一致。

### Ninja Multi-Config 是什么

`Ninja Multi-Config` 是 CMake 自 3.17 起内置的**多配置生成器**，与 VS generator 同级，
但底层用 ninja 文件而非 .sln/.vcxproj。一次 configure 后会在 build 目录下生成：

```
build/ninja-mc-msvc/
├── build.ninja              # 默认 config（一般是 Debug）
├── build-Debug.ninja        # Debug 专用
├── build-Release.ninja
├── build-RelWithDebInfo.ninja
└── build-MinSizeRel.ninja
```

`cmake --build <dir> --config <Type>` 通过选 `build-<Type>.ninja` 切换；CMake preset 的
buildPreset 也支持 `"configuration": "Debug"` 字段，所以使用方式与 VS preset 完全相同。

**优势**：

- ninja 速度（远快于 VS 的 MSBuild）
- 跨平台可用（gcc / clang / clang-cl / cl 都能配，本仓库只为 cl 提供 `ninja-mc-msvc`，
  其他单配置 preset 已够用）
- 不依赖 VS 工程文件，CI / 命令行场景更轻量

**劣势**：与 VS generator 共享同一个**多配置 + Conan 摩擦**（见下）。

### 多配置 + Conan 的固有摩擦

`msvc-conan` 与 `ninja-mc-msvc-conan` 都是多配置 preset，本应"一次 configure 后随便切
build_type"，但 **Conan 的 `conan_toolchain.cmake` 是 per-build-type 的**——每跑一次
`conan install` 会按当前 profile 的 `build_type` 重新生成 toolchain，**覆盖**前一次的
内容。所以这两组 preset 在实际使用中**同一时刻只能跑一种 build_type**。

工作流的现实：

```bash
# 想跑 Debug：
conan install . -s build_type=Debug --output-folder=build/msvc-conan --build=missing
cmake --preset msvc-conan       # 此时 toolchain 是 Debug 版本
cmake --build --preset msvc-debug-conan
ctest --preset msvc-debug-conan

# 想换成 Release：
conan install . -s build_type=Release --output-folder=build/msvc-conan --build=missing
                                 # ↑ 这一步会覆盖 build/msvc-conan/conan_toolchain.cmake
cmake --preset msvc-conan       # 重新 configure 让 CMake 拿新的 toolchain
cmake --build --preset msvc-release-conan
```

也就是 4 个 buildPreset 是为了"切到其中一种"提供入口，**不是真的能并存 4 种 build_type
共享一份 configure**。

### 想要 Windows MSVC ABI + Conan 同时保留 4 种 build type？

改用 **`clang-cl-{debug,release,relwithdebinfo,minsizerel}-conan`**：

- 单配置 Ninja，每个 build type 各自有独立的 `build/clang-cl-<type>-conan/` 目录与
  `conan_toolchain.cmake`，互不覆盖
- clang-cl 与 cl.exe **使用同一套 MSVC ABI**（都链接 MSVC STL、用 MSVC 的 name mangling），
  二进制层面与 cl.exe 互通，对 99% 的代码而言行为等价
- 唯一差别：clang-cl 额外支持一些 GCC/Clang 风格的 builtin/attribute；想要"绝对纯
  cl.exe"才需要继续用 msvc-conan

### 为什么仓库还保留 msvc-conan / ninja-mc-msvc-conan

虽然有摩擦，仍保留是因为：

1. **教学价值**：让读者亲眼看到"VS generator + Conan"与"Ninja MC + Conan"的实际行为
2. **想跑纯 cl.exe + VS .sln 调试**的用户仍可用 `msvc-conan` 接 VS IDE
3. **对称性**：preset 全景图保持"vcpkg ↔ conan 一一对应"

---

## 8. profile 进阶

### host profile vs build profile

交叉编译场景（在 x64 Linux 上构建 ARM Linux 二进制）：

```bash
conan install . \
    --profile:host=arm-linux \    # 目标 ABI
    --profile:build=default \     # 构建机 ABI（用于 build-tool 包，如 cmake/ninja）
    --output-folder=build/arm-linux \
    --build=missing
```

- `--profile:host`：目标平台 profile
- `--profile:build`：构建机平台 profile（默认与 host 一致，非交叉时不用写）

vcpkg 用 host triplet + build triplet 表达类似概念，但 Conan 的 profile 比 triplet 表达
力更强。

### profile 模板与 include

profile 可以 include 其他 profile：

```ini
include(common-cpp23)

[settings]
build_type=Debug
```

`common-cpp23` 在同一 profile 目录下另一个文件，定义共享 settings。便于"基础 profile +
变体"模式。

### 命令行覆盖

任何 profile 字段都能在命令行临时覆盖：

```bash
conan install . \
    -s build_type=Debug \
    -s compiler.version=14 \
    -o boost*:shared=True \
    -c tools.cmake.cmaketoolchain:generator=Ninja
```

- `-s`：覆盖 settings
- `-o`：覆盖 options（包级开关）
- `-c`：覆盖 conf（Conan 行为配置）

适合"基础 profile + 偶尔需要的变种"场景，不用为每个变种建 profile 文件。

---

## 9. 与 CI 的集成（前瞻）

### 当前状态

本仓库 CI（[.github/workflows/ci.yml](../.github/workflows/ci.yml)）**只跑 vcpkg
preset，未集成 Conan**。Conan preset 仅本地可用。

### 为什么 CI 没集成 Conan

- **避免双倍 matrix**：当前已有 `linux-gcc` / `linux-clang` / `windows-msvc` 三个 build
  job，加 Conan 等于 ×2 = 6 个 job，CI 时间和 GHA 配额翻倍
- **vcpkg 已能验证依赖管理是工作的**：Conan 与 vcpkg 都走 `find_package(GTest CONFIG)`，
  vcpkg 跑通基本能保证 Conan 也跑通
- **教学项目优先级**：维护 vcpkg + Conan 双 CI 不如把精力放在多模块覆盖上

### 如果未来要加

设计草图（不是已实现，仅供参考）：

```yaml
- name: Install Conan
  run: pip install conan && conan profile detect

- name: Conan install
  run: |
    conan install . \
      --output-folder=build/${{ matrix.preset }}-conan \
      --build=missing \
      -s build_type=RelWithDebInfo \
      -s compiler.cppstd=23

- name: CMake configure (Conan)
  run: cmake --preset ${{ matrix.preset }}-conan

- name: Build (Conan)
  run: cmake --build --preset ${{ matrix.preset }}-conan

- name: Test (Conan)
  run: ctest --preset ${{ matrix.preset }}-conan
```

需要解决的问题：

- **Conan cache**：`~/.conan2/p/` 应该走 GHA cache（`actions/cache@v4`）
- **profile 一致性**：CI 的 `conan profile detect` 默认 cppstd 可能跟本地不同，需要
  `-s` 覆盖统一
- **matrix 扩展**：是新增 `*-conan` job 还是用 step 在同一 job 里跑两遍

详见 [ci-guide.md §11 扩展功能](ci-guide.md#11-扩展功能)。

---

## 10. 常见问题与排查

### "Could not find toolchain file: conan_toolchain.cmake"

99% 是忘了第一步 `conan install`。本仓库 `_conan` preset 把 toolchain 路径写死，文件
不存在 cmake 直接报错。修法：

```bash
conan install . --output-folder=build/<presetName> --build=missing
cmake --preset <presetName>
```

剩下 1% 是 `--output-folder` 跟 preset 名拼错。preset 名是 `gcc-debug-conan`，
output-folder 必须也是 `build/gcc-debug-conan`，不是 `build/gcc-debug`。

### profile 与编译器版本不匹配

```
ERROR: Missing prebuilt package for 'gtest/1.15.0'
... compiler.version=14 ...
You can try:
    'conan install ... --build=missing'
```

远端 binary repo 没有当前 profile 对应的二进制。两个方向：

- 加 `--build=missing` 让 Conan 本地 build（最常见）
- 改 profile 的 `compiler.version` 与远端有的对齐（节省 build 时间）

### libcxx ABI 链接错乱（GCC 5+ 的双 ABI）

GCC 5 引入了 dual ABI，老代码用 `libstdc++`（_GLIBCXX_USE_CXX11_ABI=0），新代码用
`libstdc++11`（_GLIBCXX_USE_CXX11_ABI=1）。混用会出现 `std::string` / `std::list` 这
些类型链接错乱：

```
undefined reference to `f(std::__cxx11::basic_string<...>)'
```

修法：profile 里 `compiler.libcxx=libstdc++11`（现代默认），并保证项目不显式 `-D_GLIBCXX_USE_CXX11_ABI=0`。

### MSVC `compiler.runtime` 与 `gtest_force_shared_crt`

vcpkg 用 `triplet` 决定 CRT，Conan 用 `compiler.runtime` 字段：

```ini
[settings]
compiler=msvc
compiler.runtime=dynamic       # /MD
compiler.runtime_type=Release  # /MD vs /MDd
```

本仓库 [`cmake/Dependencies.cmake`](../cmake/Dependencies.cmake) 强制
`gtest_force_shared_crt: ON`，配合 `compiler.runtime=dynamic`（默认）就对齐了。如果
profile 改成 `runtime=static`，记得同步把 `gtest_force_shared_crt` 改 OFF 并设
`CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded`。

详见 [vcpkg-guide.md §8 MSVC CRT 对齐](vcpkg-guide.md#8-msvc-crt-对齐)。

### Conan cache 占空间太大

`~/.conan2/p/`（packages cache）可能涨到几十 GB。清理：

```bash
# 清理 90 天没用的包
conan cache clean --temp --download --source --build "*"

# 完全重置（核选项）
rm -rf ~/.conan2/p/
```

cache 删了下次 install 会重新下载/编译。

### 修改 conanfile.txt 后没生效

记住 Conan 是**两步走**：改 `conanfile.txt` 必须**重跑** `conan install`，光跑
`cmake --preset` 没用 —— preset 读的是上次 install 生成的旧 toolchain 文件。

### `--build=missing` 总是 build 同一个包

通常意味着你的 profile 在多次 install 间发生了细微变化（package_id 一直变，cache 命中
不了）。检查：

- `conan profile show` 是否每次都一样
- 是否有命令行 `-s` / `-o` 不一致
- cppstd 是否被某些工具偷偷改过

确认 profile 稳定后，`--build=missing` 第一次会 build，第二次起命中 cache，秒过。
