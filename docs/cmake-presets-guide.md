# CMake Presets 使用与配置完整指南

> 本文档系统讲解 CMake Presets 的设计理念、文件结构、继承机制、condition 系统、与
> toolchain 的衔接，以及本仓库的 `CMakePresets.json` 是如何把多编译器（gcc / clang /
> clang-cl / MSVC）+ 多构建类型 + 多依赖管理器（vcpkg / Conan）组织起来的。
>
> 配套文件：[`CMakePresets.json`](../CMakePresets.json)
>
> 配套阅读：
> - [`docs/vcpkg-guide.md`](vcpkg-guide.md) —— vcpkg 端到端
> - [`docs/conan-guide.md`](conan-guide.md) —— Conan 端到端
> - [`docs/ci-guide.md`](ci-guide.md) —— CI 怎么按 preset 名展开 matrix

---

## 目录

1. [什么是 CMake Presets，为什么需要它](#1-什么是-cmake-presets为什么需要它)
2. [文件结构与 schema 速览](#2-文件结构与-schema-速览)
3. [hidden + inherits：组合优于复制](#3-hidden--inherits组合优于复制)
4. [本仓库 preset 全景图](#4-本仓库-preset-全景图)
5. [condition：让 preset 跨平台共存](#5-condition让-preset-跨平台共存)
6. [多配置 vs 单配置生成器](#6-多配置-vs-单配置生成器)
7. [testPresets 与 _test-base 的设计](#7-testpresets-与-_test-base-的设计)
8. [toolchain hook 机制](#8-toolchain-hook-机制)
9. [CMakeUserPresets.json：本地叠层](#9-cmakeuserpresetsjson本地叠层)
10. [常见问题与排查](#10-常见问题与排查)

---

## 1. 什么是 CMake Presets，为什么需要它

**CMake Presets** 是 CMake 3.19 引入的"命令固化机制"：把原本要写在命令行里的一长串
`-G`、`-D`、环境变量、toolchain 路径等，全部写进一份 `CMakePresets.json`，开发者用一行
`cmake --preset <名字>` 就能复现完整配置。

### 没有 preset 的世界

```bash
# 每个开发者要记住自己平台的完整命令
cmake -S . -B build/gcc-debug \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-linux
```

问题一大堆：

- 命令贴错一个字符就能整天 debug
- 团队里每个人/每个 IDE 写法不同，CI 又是另一套
- 切编译器要重抄一遍
- 新人入门门槛高

### 有 preset 的世界

```bash
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
```

三行命令在所有平台、所有 IDE、CI 全部一样。配置细节都进了版本管理的 JSON。

### preset 在本项目的价值

本项目同时支持 **GCC / Clang / clang-cl / MSVC** 四套编译器、**Debug / Release /
RelWithDebInfo / MinSizeRel** 四种构建类型、**vcpkg / Conan** 两种依赖管理器，组合爆炸。
preset 把每个组合凝固成一个名字，开发者只需要记编译器+构建类型，例如
`clang-relwithdebinfo`、`msvc-debug-conan`。

CI（[ci.yml](../.github/workflows/ci.yml)）也直接复用这套 preset 名 —— **本地与 CI
使用完全一样的配置**，本地通过的代码 CI 不会因配置差异而失败。

---

## 2. 文件结构与 schema 速览

`CMakePresets.json` 的最小骨架：

```json
{
    "version": 6,
    "cmakeMinimumRequired": { "major": 3, "minor": 25, "patch": 0 },
    "configurePresets": [ ... ],
    "buildPresets":     [ ... ],
    "testPresets":      [ ... ]
}
```

### 顶层字段

| 字段 | 含义 | 本仓库取值 |
| --- | --- | --- |
| `version` | preset schema 版本，决定能用哪些字段 | `6`（支持 condition、$env 等较新特性） |
| `cmakeMinimumRequired` | 最低 CMake 版本，不满足直接报错 | `3.25` |
| `configurePresets` | configure 阶段的 preset 数组 | 7 个 hidden + 17 个 leaf |
| `buildPresets` | build 阶段的 preset 数组 | 22 个 |
| `testPresets` | ctest 的 preset 数组 | 1 个 hidden + 22 个 |

> schema 版本越高功能越多，但需要更新的 CMake。`version: 6` 要求 CMake ≥ 3.25。本仓库
> `cmakeMinimumRequired` 已经卡到 3.25，所以可以放心用 `condition`、`$env` 这些 v3+
> 才有的字段。

### 三类 preset 各自管什么

```
configurePreset  →  cmake --preset <name>
                    决定：generator / compiler / toolchain / cacheVariables

buildPreset      →  cmake --build --preset <name>
                    引用一个 configurePreset，可指定 configuration / target

testPreset       →  ctest --preset <name>
                    引用一个 configurePreset，控制 ctest 行为
```

三类 preset **可以同名**（CMake 按使用阶段区分）。本仓库 `gcc-debug` 既是
configurePreset 也是 buildPreset 也是 testPreset，名字共用，开发者一个名字打通三步。

---

## 3. hidden + inherits：组合优于复制

如果每个 leaf preset 都把 generator、compiler、toolchain、cacheVariables 全写一遍，
24 个 preset 就要重复 24 次相同的字段。**`hidden` + `inherits` 让你把公共字段抽出来当
积木**。

### `hidden: true` 的作用

```json
{
    "name": "_base",
    "hidden": true,
    "binaryDir": "${sourceDir}/build/${presetName}",
    "cacheVariables": { "CMAKE_EXPORT_COMPILE_COMMANDS": "ON" }
}
```

`hidden: true` 表示这个 preset 是**积木，不是入口**。它：

- **不会**出现在 `cmake --list-presets`
- **不能**直接 `cmake --preset _base`（CMake 会报错说 preset 是 hidden 的）
- **可以**被其他 preset `inherits`

约定俗成用下划线开头（`_base` / `_vcpkg` / `_gcc`），跟普通 leaf 视觉区分。

### `inherits` 的合并语义

```json
{
    "name": "gcc-debug",
    "inherits": "_gcc",
    "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" }
}
```

`inherits` 可以是字符串或数组：

- **字符串**：单继承，从一个 hidden preset 拿字段
- **数组**：多继承，按顺序合并；后者覆盖前者

合并规则（重要）：

| 字段类型 | 合并方式 |
| --- | --- |
| `cacheVariables`（对象） | **浅合并**，子 preset 同名 key 覆盖父的 |
| `environment`（对象） | 浅合并 |
| `generator` / `binaryDir` 等标量 | 子有则覆盖，子无则继承 |
| `condition` | **不继承**！每个 preset 自己判断 |

> `condition` 不继承这条很关键 —— 父 preset 的 condition 不会自动传到子 preset。本仓库
> 的做法是：**hidden 中间层带 condition**，leaf 不写 condition，而 CMake 实际行为是把
> hidden 父级的 condition 视为 leaf 是否可见的前置条件（v3.21+ 表现一致）。如果将来
> CMake 规则收紧，需要把 condition 往 leaf 上抄。

### 本仓库的三层结构

```
                       ┌─────────────────────────┐
                       │       _base             │  ← binaryDir + compile_commands
                       └─────────────────────────┘
                                  │
                ┌─────────────────┼─────────────────┐
                │                                   │
        ┌───────▼──────┐                    ┌──────▼──────┐
        │   _vcpkg     │                    │   _conan    │   ← toolchain 注入
        └───────┬──────┘                    └──────┬──────┘
                │                                  │
       ┌────────┼────────┐                ┌────────┼────────┐
       │        │        │                │        │        │
   ┌───▼──┐ ┌──▼──┐ ┌────▼─────┐    ┌────▼────┐ ┌─▼──────┐ │
   │ _gcc │ │_clang│ │_clang-cl │    │_gcc-conan│ │_clang- │ ...
   └───┬──┘ └──┬──┘ └────┬─────┘    └────┬────┘ │ conan  │
       │       │         │               │      └────┬───┘
   ┌───▼───┐   …         …               …           …
   │gcc-   │
   │debug  │  ← leaf：只填 CMAKE_BUILD_TYPE
   └───────┘
```

第一层（`_base`）管"所有 preset 都需要"的设置；第二层（`_vcpkg` / `_conan`）管
toolchain；第三层（`_gcc` / `_clang` / ...）管编译器和 generator；leaf 只填构建类型。

**新增一个构建类型** 只要加一个 leaf；**新增一个编译器** 加一个第三层的 hidden + 4 个
leaf；**新增一种依赖管理器** 加一个第二层 hidden + 复制一组第三层。

---

## 4. 本仓库 preset 全景图

> 本节是仓库 preset JSON 的**唯一权威展示**位置，其他文档只展示片段并反向链接到这里。

### 第一层：`_base`

```json
{
    "name": "_base",
    "hidden": true,
    "binaryDir": "${sourceDir}/build/${presetName}",
    "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
    }
}
```

- `${sourceDir}` / `${presetName}` 是 CMake 内置宏，分别展开为源码目录与当前 preset 名
- `binaryDir` 模板让每个 preset 各占一个 `build/<presetName>/` 目录，互不干扰
- `compile_commands.json` 给 clangd 用（详见 [clangd-toolchain-overview.md](clangd-toolchain-overview.md)）

### 第二层：`_vcpkg` 与 `_conan`

```json
{
    "name": "_vcpkg",
    "hidden": true,
    "cacheVariables": {
        "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    }
}

{
    "name": "_conan",
    "hidden": true,
    "cacheVariables": {
        "CMAKE_TOOLCHAIN_FILE": "${sourceDir}/build/${presetName}/conan_toolchain.cmake"
    }
}
```

两个 toolchain 注入策略的差别：

- vcpkg 走 **环境变量** `$env{VCPKG_ROOT}` —— 一台机器只有一个 vcpkg root，每个用户
  自己设环境变量
- Conan 走 **per-preset 路径** —— Conan 的 toolchain 是 `conan install` 命令产物，按
  preset 隔离

详见 [§8 toolchain hook 机制](#8-toolchain-hook-机制)。

### 第三层：编译器 hidden 中间层

```json
{
    "name": "_gcc",
    "hidden": true,
    "inherits": ["_vcpkg", "_base"],
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

`inherits` 数组顺序：`_vcpkg` 先，`_base` 后 —— 后者覆盖前者，但二者无字段冲突，顺序
其实不影响。习惯把"主要功能"（toolchain）写前面，把"通用基础"（_base）写后面。

`condition` 把这个 hidden 限定在 Linux 主机上，详见 [§5](#5-condition让-preset-跨平台共存)。

类似地：

- `_clang`：与 `_gcc` 同模板，编译器换成 `clang/clang++`，condition 改为 `notEquals Windows`（覆盖 Linux + macOS）
- `_clang-cl`：condition 为 `equals Windows`，且固化 `VCPKG_TARGET_TRIPLET: x64-windows`
- `_gcc-conan` / `_clang-conan`：换 `inherits` 为 `["_conan", "_base"]`，condition 同上

### 第四层：leaf

```json
{
    "name": "gcc-debug",
    "displayName": "GCC Debug",
    "inherits": "_gcc",
    "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" }
}
```

leaf 的内容**只有构建类型**。其他全部从 `_gcc` 链上继承。这就是分层的回报。

### MSVC 是个特例：单 configurePreset + 多 buildPreset

详见 [§6 多配置 vs 单配置生成器](#6-多配置-vs-单配置生成器)。

### 完整 leaf 一览

| 编译器 | leaf 名 | 数量 |
| --- | --- | --- |
| GCC | `gcc-{debug,release,relwithdebinfo,minsizerel}` | 4 |
| Clang | `clang-{debug,release,relwithdebinfo,minsizerel}` | 4 |
| clang-cl | `clang-cl-{debug,release,relwithdebinfo,minsizerel}` | 4 |
| MSVC | `msvc`（多配置） | 1 |
| GCC + Conan | `gcc-{debug,release,relwithdebinfo}-conan` | 3 |
| Clang + Conan | `clang-{debug,release,relwithdebinfo}-conan` | 3 |
| MSVC + Conan | `msvc-conan`（多配置） | 1 |
| **合计 configurePreset** | | **20** |

### 命名约定

- vcpkg 版：`{compiler}-{buildtype}`（例：`gcc-debug`）
- Conan 版：`{compiler}-{buildtype}-conan`（例：`gcc-debug-conan`）
- 多配置：仅编译器名（例：`msvc`、`msvc-conan`）

---

## 5. condition：让 preset 跨平台共存

### 为什么需要 condition

仓库一份 `CMakePresets.json` 同时承载 Linux、macOS、Windows 三类主机的 preset。如果 Windows
用户能看到 `gcc-debug`（Linux 的 GCC preset），他可能误用 —— configure 出错事小，**用
错误的 vcpkg triplet 把 gtest 装出来再链接失败**事大。

`condition` 让 preset **在不满足条件的主机上不出现**：`cmake --list-presets` 不列出，
`cmake --preset` 报错说找不到。

### 语法

```json
"condition": {
    "type": "equals",
    "lhs": "${hostSystemName}",
    "rhs": "Linux"
}
```

`type` 常用：

- `equals` / `notEquals`：比较两个字符串
- `inList` / `notInList`：判断 lhs 是否在 rhs 列表里
- `matches`：lhs 匹配 rhs 正则
- `anyOf` / `allOf`：组合多个条件
- `not`：取反

`${hostSystemName}` 是 CMake 内置宏，展开为 `Linux` / `Darwin`（macOS）/ `Windows`。

### 本仓库的条件分布

| Hidden preset | condition | 含义 |
| --- | --- | --- |
| `_gcc` / `_gcc-conan` | `${hostSystemName}` equals `Linux` | 仅 Linux —— macOS 上 `gcc` 是 clang shim，避免误用 |
| `_clang` / `_clang-conan` | `${hostSystemName}` notEquals `Windows` | Linux + macOS（macOS 用 Apple Clang） |
| `msvc` / `_clang-cl` | `${hostSystemName}` equals `Windows` | 仅 Windows |

历史背景：早期版本只用 `notEquals Windows`，但这覆盖了 macOS，被代码评审指出 macOS
应该走 `clang-*` 而不是 `gcc-*`。详见 PR #5。

### condition 不满足时的表现

```bash
# 在 Windows 上跑：
$ cmake --preset gcc-debug
CMake Error: No such preset in CMakePresets.json: "gcc-debug"

$ cmake --list-presets
Available configure presets:
  "msvc"            - MSVC (VS 2022, multi-config)
  "clang-cl-debug"  - clang-cl Debug
  ...
# 注意：gcc-* 与 clang-* 都不会出现
```

### MinGW 用户怎么办

MinGW（msys2 ucrt64 环境）虽然在 Windows 上，但用 GNU 工具链 + GNU ABI。本仓库的
`_gcc` 在 Windows 上不可见，这正是 condition 想避免的"误用"。仓库为 MinGW 单独提供了
两组一等公民 preset：`mingw-gcc-*` 与 `mingw-clang-*`，门控为
`hostSystemName == Windows`，triplet 已固化为 `x64-mingw-dynamic`，开箱即用。详见
[vcpkg-guide.md §6 MinGW 专题](vcpkg-guide.md#6-mingw-专题)。

---

## 6. 多配置 vs 单配置生成器

CMake 的 generator 分两类：

- **单配置**（Ninja、Makefile）：configure 时就要决定 `CMAKE_BUILD_TYPE`，每个构建类型
  一份独立的 build 目录
- **多配置**（Visual Studio、Xcode）：configure 一次生成"工程文件"，**构建时**通过
  `--config Debug/Release` 选构建类型

本仓库都用到了。

### 单配置（gcc / clang / clang-cl）

```bash
cmake --preset gcc-debug              # configure 出 build/gcc-debug/
cmake --build --preset gcc-debug      # 构建 Debug
cmake --preset gcc-release            # 单独 configure 出 build/gcc-release/
cmake --build --preset gcc-release    # 构建 Release
```

每个构建类型一个 leaf preset，互不干扰。优点：不同构建类型的 build artifact 不会冲突。
缺点：preset 数量多（4 build types × 3 编译器 = 12 个）。

### 多配置（MSVC / Visual Studio）

```json
{
    "name": "msvc",
    "displayName": "MSVC (VS 2022, multi-config)",
    "generator": "Visual Studio 17 2022",
    "architecture": { "value": "x64", "strategy": "set" },
    "cacheVariables": { "VCPKG_TARGET_TRIPLET": "x64-windows" }
}
```

```bash
cmake --preset msvc                          # configure 一次（生成 .sln 工程）
cmake --build --preset msvc-debug            # 构建 Debug
cmake --build --preset msvc-release          # 构建 Release
cmake --build --preset msvc-relwithdebinfo   # 构建 RelWithDebInfo
```

只有一个 configurePreset，但有四个 buildPreset：

```json
{ "name": "msvc-debug",          "configurePreset": "msvc", "configuration": "Debug" },
{ "name": "msvc-release",        "configurePreset": "msvc", "configuration": "Release" },
{ "name": "msvc-relwithdebinfo", "configurePreset": "msvc", "configuration": "RelWithDebInfo" },
{ "name": "msvc-minsizerel",     "configurePreset": "msvc", "configuration": "MinSizeRel" }
```

`configuration` 字段就是 `--config` 的等价物。

### 为什么不全用一种

- **Ninja 单配置**：构建快、跨平台、与 clangd 配合好（一个 `compile_commands.json` 不混淆）
- **VS 多配置**：能直接在 Visual Studio IDE 里切构建类型；MSVC 用户更习惯

仓库同时提供两套，让用户按自己的工作流选。

---

## 7. testPresets 与 _test-base 的设计

`testPresets` 控制 `ctest` 的行为。本仓库把所有 testPreset 共用的设置抽到一个
hidden base：

```json
{
    "name": "_test-base",
    "hidden": true,
    "output": { "outputOnFailure": true, "shortProgress": true },
    "execution": { "noTestsAction": "ignore", "stopOnFailure": false }
}
```

- `outputOnFailure`：测试失败时打印 stdout（默认只显示 PASS/FAIL）
- `shortProgress`：进度条简洁形式
- `noTestsAction: ignore`：模块没注册测试时不报错（demo-only 模块常见）
- `stopOnFailure: false`：单个测试挂了继续跑后面的

每个 leaf testPreset 只需写：

```json
{ "name": "gcc-debug", "inherits": "_test-base", "configurePreset": "gcc-debug" }
```

### `ctest --preset` vs `cmake --build --preset && ctest`

```bash
# 方式 A（推荐）：直接 ctest --preset
ctest --preset gcc-debug

# 方式 B：手动 ctest，需要 cd 到 build 目录
cd build/gcc-debug && ctest --output-on-failure
```

方式 A 的好处：所有 ctest 选项已经在 preset 里固化，开发者不需要记。CI 也用方式 A。

---

## 8. toolchain hook 机制

> 这是 preset 与 vcpkg/Conan 衔接的核心机制。vcpkg-guide 与 conan-guide 都会反向引用
> 这一节。

### `CMAKE_TOOLCHAIN_FILE` 的本质

CMake 在第一次 `project()` 调用之前，会**先加载** `CMAKE_TOOLCHAIN_FILE` 指向的 .cmake
文件。这个文件可以：

- 设置编译器路径
- 注入 `find_package` 的搜索路径
- 设置目标架构、CRT、运行时库
- 触发依赖安装（vcpkg manifest 模式就是利用这一点）

这是一个 **"在编译还没开始之前就改造 CMake"** 的钩子。包管理器全靠它接入 CMake。

### vcpkg 的 hook：`vcpkg.cmake`

```json
"CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
```

- `$env{VCPKG_ROOT}` 是 preset 的环境变量展开宏，读取 shell 里的 `VCPKG_ROOT`
- `vcpkg.cmake` 在 configure 时会：
  1. 读 `vcpkg.json`（manifest 模式）
  2. 决定 triplet（来自 `VCPKG_TARGET_TRIPLET` 或主机自动检测）
  3. 安装缺失依赖（首次 configure 慢就是因为这一步）
  4. 把 vcpkg 的 install 目录加进 `CMAKE_PREFIX_PATH`，让 `find_package` 能找到

详见 [vcpkg-guide.md §4](vcpkg-guide.md#4-toolchain-文件的工作机制)。

### Conan 的 hook：`conan_toolchain.cmake`

```json
"CMAKE_TOOLCHAIN_FILE": "${sourceDir}/build/${presetName}/conan_toolchain.cmake"
```

- 路径硬编码到 `build/<presetName>/` —— 因为 Conan 的 toolchain 是 **`conan install`
  命令的产物**，必须先跑 `conan install` 再跑 `cmake --preset`
- `conan_toolchain.cmake` 由 Conan 的 `CMakeToolchain` generator 写出
- 它把编译器、build_type、libcxx ABI 等翻译成 CMake 变量，并把 Conan 的 install
  目录加进 `CMAKE_PREFIX_PATH`

详见 [conan-guide.md §4](conan-guide.md#4-cmakedeps-与-cmaketoolchain-两个-generator)。

### 两种策略对比

| 维度 | vcpkg | Conan |
| --- | --- | --- |
| toolchain 来源 | 仓库自带（git clone） | 命令产物（`conan install` 生成） |
| preset 路径写法 | `$env{VCPKG_ROOT}/...` | `${sourceDir}/build/${presetName}/...` |
| 是否需要预步骤 | 否（`cmake --preset` 一步触发） | 是（必须先 `conan install`） |
| 多 preset 共享 | 一份 toolchain 文件 | per-preset 各一份 |

---

## 9. CMakeUserPresets.json：本地叠层

`CMakeUserPresets.json` 是 CMake 给个人用户准备的"叠层文件"：

- **不入库**（本仓库 `.gitignore` 已忽略）
- 与 `CMakePresets.json` 同目录
- CMake 加载时会把两份文件**合并**，叠层 preset 可以 `inherits` 仓库 preset

### 典型用途

- **本地编译器路径不在 PATH 里**：继承 `mingw-gcc-debug` 等 leaf 后覆盖
  `CMAKE_C_COMPILER` 为绝对路径（如 `F:/scoop/apps/msys2/current/ucrt64/bin/gcc.exe`）
- **想用非默认 triplet**：例如继承 `mingw-gcc-debug` 改成 `x64-mingw-static`，详见
  [vcpkg-guide.md §6 备选方案 2](vcpkg-guide.md#6-mingw-专题)
- **个人偏好的 build 目录布局**：覆盖 `binaryDir`
- **添加只对自己有用的 leaf**：例如带特殊 sanitizer 配置的 `my-debug-asan`

> 注：MinGW UCRT64 的 vcpkg triplet 已经在仓库的 `mingw-gcc-*` / `mingw-clang-*`
> preset 里固化为 `x64-mingw-dynamic`，**不再需要 UserPresets 叠层**。

### 与仓库 preset 的合并规则

- 同名 preset：UserPresets 里的字段**覆盖**仓库版（按 cacheVariables 浅合并）
- 不同名 preset：直接追加，能 `inherits` 仓库的 hidden 或公开 preset

### 叠层模板示例

```json
{
    "version": 6,
    "cmakeMinimumRequired": { "major": 3, "minor": 25, "patch": 0 },
    "configurePresets": [
        {
            "name": "my-mingw-abs-path-debug",
            "inherits": "mingw-gcc-debug",
            "cacheVariables": {
                "CMAKE_C_COMPILER":   "F:/scoop/apps/msys2/current/ucrt64/bin/gcc.exe",
                "CMAKE_CXX_COMPILER": "F:/scoop/apps/msys2/current/ucrt64/bin/g++.exe"
            }
        }
    ]
}
```

---

## 10. 常见问题与排查

### "No such preset in CMakePresets.json"

可能原因：

1. **拼错 preset 名**：跑 `cmake --list-presets` 核对
2. **condition 不满足**：你在 Windows 上找 `gcc-debug`，但它被门控为 Linux-only。
   `--list-presets` 不会列出来，但 `--list-presets all` 会列出**含 hidden 与不满足
   condition 的所有 preset**，可用来确认是被 condition 挡住了
3. **CMake 版本太低**：仓库要求 ≥ 3.25，旧版本不认 `version: 6` 直接失败
4. **`hidden: true` 的 preset 想直接用**：hidden 只能被 inherits，不能 `--preset`

### inherits 顺序导致 cacheVariables 被覆盖

```json
"inherits": ["_a", "_b"]
```

后者（`_b`）的同名 cacheVariable 会覆盖前者（`_a`）。本仓库习惯：通用基础（`_base`）
放后面，主要功能（`_vcpkg` / `_conan`）放前面。如果 `_base` 不小心定义了 toolchain，
就会把 `_vcpkg` 的 toolchain 覆盖掉。**修改 inherits 顺序前先确认两个 preset 没有
字段冲突**。

### IDE 识别 preset 的注意事项

- **VSCode + CMake Tools 扩展**：右下角能切 preset；如果改了 JSON 没生效，
  Ctrl+Shift+P → "CMake: Reset CMake Tools Extension State"
- **CLion**：File → Settings → Build, Execution, Deployment → CMake，启用 "CMakePresets.json"
- **Visual Studio**：原生支持 preset，但 condition 解析有差异，可能比命令行多/少几个

### `${env}` vs `$env{}` 不一样

- `$env{NAME}`：preset 字段里的环境变量展开，**configure 时一次性展开为字符串**
- `$ENV{NAME}`（CMake script 里）：CMake 脚本里的运行时读取，是另一个语境

写 preset 时统一用 `$env{}`。

### 改 preset 后必须重新 configure

```bash
# 改 CMakePresets.json 后：
rm -rf build/gcc-debug   # 旧的 CMakeCache.txt 会缓存上次 configure 的结果
cmake --preset gcc-debug
```

`CMakeCache.txt` 一旦生成，CMake 不会重新读 toolchain 文件、不会重算 condition。改
preset 不重新 configure 是 CMake 用户最常踩的坑。

### CI 与本地 preset 怎么对应

CI 直接用 preset 名作 matrix entry：

```yaml
matrix:
  include:
    - { name: linux-gcc, preset: gcc-relwithdebinfo, ... }
    - { name: linux-clang, preset: clang-relwithdebinfo, ... }
```

CI 跑的就是 `cmake --preset $preset` —— 与你本地命令一字不差。详见
[ci-guide.md §5 Matrix 并行策略](ci-guide.md#5-matrix-并行策略)。
