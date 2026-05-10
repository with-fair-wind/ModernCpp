# C++ 编译流程与 ABI 概念指南

> 本文档系统讲解 C++ 程序从源码到可执行文件的完整流程，以及 ABI（Application Binary
> Interface）在其中的核心角色。作为本仓库工具链文档的"概念基底"，其他指南可引用此处来
> 解释"为什么需要这样配置"。
>
> 配套阅读：
> - [`vcpkg-guide.md`](vcpkg-guide.md) — triplet 实操与 MinGW 专题
> - [`cmake-presets-guide.md`](cmake-presets-guide.md) — preset 与 toolchain 配置
> - [`clangd-toolchain-overview.md`](clangd-toolchain-overview.md) — clangd 如何使用编译命令

---

## 目录

1. [编译流程全景](#1-编译流程全景)
2. [目标文件与符号](#2-目标文件与符号)
3. [链接：静态与动态](#3-链接静态与动态)
4. [ABI 定义与组成](#4-abi-定义与组成)
5. [C Runtime（CRT）](#5-c-runtimecrt)
6. [C++ 标准库](#6-c-标准库)
7. [平台 ABI 全景对比](#7-平台-abi-全景对比)
8. [Clang 的多目标能力与 LLVM](#8-clang-的多目标能力与-llvm)
9. [MSYS2 子系统与 ABI 对应关系](#9-msys2-子系统与-abi-对应关系)
10. [本项目的 ABI 决策](#10-本项目的-abi-决策)

---

## 1. 编译流程全景

C++ 源码到可执行文件经历四个阶段（外加运行时加载）：

```
┌────────┐    ┌────────┐    ┌────────┐    ┌────────┐    ┌────────────┐
│ 预处理  │───▶│  编译   │───▶│  汇编   │───▶│  链接   │───▶│ 加载 & 运行 │
│        │    │        │    │        │    │        │    │            │
│ .cpp   │    │ .i     │    │ .s     │    │ .o/.obj│    │ .exe/.so   │
│ .h     │    │ (展开后)│    │ (汇编)  │    │ + .a   │    │ + .dll     │
└────────┘    └────────┘    └────────┘    └────────┘    └────────────┘
  #include      语法分析       指令编码       符号解析        动态链接器
  宏展开        语义分析       生成目标文件    重定位          加载依赖 DLL/SO
  条件编译      IR 优化                      合并段
```

### 1.1 预处理（Preprocessing）

- **输入**：`.cpp` + 被 `#include` 的所有头文件
- **输出**：单一的翻译单元（translation unit，`.i` 文件）
- **做的事**：展开 `#include`、替换宏、处理 `#if` / `#ifdef`
- **工具**：`gcc -E`、`clang -E`、`cl /E`

一个 `.cpp` 文件加上它递归 include 的所有头文件，合在一起就是一个 **translation unit
（TU）**。编译器一次处理一个 TU，TU 之间互相不可见——这就是为什么需要头文件声明。

### 1.2 编译（Compilation）

- **输入**：预处理后的 TU
- **输出**：汇编代码（`.s`）或直接生成目标文件（现代编译器通常跳过 `.s` 阶段）
- **做的事**：
  - 词法分析 → 语法分析 → AST（抽象语法树）
  - 语义分析（类型检查、重载决议、模板实例化）
  - 优化（在 LLVM 中是对 IR 的各种 pass）
  - 代码生成（从 IR 到目标机器的指令）

这一阶段决定了**名字修饰（name mangling）**和**调用约定（calling convention）**——
这些是 ABI 的核心。

### 1.3 汇编（Assembly）

- **输入**：汇编代码
- **输出**：目标文件（`.o` on Linux/macOS，`.obj` on Windows）
- **做的事**：把人类可读的汇编指令编码成二进制机器码

### 1.4 链接（Linking）

- **输入**：一组 `.o` / `.obj` + 库文件（`.a` / `.lib` / `.so` / `.dll`）
- **输出**：可执行文件（`.exe`、ELF 可执行）或共享库
- **做的事**：
  - **符号解析**：把一个 TU 里对外部函数/变量的引用，绑定到另一个 TU 的定义
  - **重定位**：把各目标文件的代码/数据段拼到一起，修正地址
  - **生成最终二进制**：填入 ELF/PE 头部、节表、导入导出表

**链接失败**是 ABI 不匹配最常见的表现——`undefined reference` / `undefined symbol`。

### 1.5 加载与运行

- **动态链接器**（`ld-linux.so` / Windows loader）在程序启动时：
  - 加载依赖的共享库（`.so` / `.dll`）
  - 解析动态符号（PLT/GOT on Linux，IAT on Windows）
- **CRT 初始化**：在 `main()` 之前执行全局对象构造、`atexit` 注册等

---

## 2. 目标文件与符号

### 2.1 目标文件格式

| 平台 | 格式 | 工具 |
|------|------|------|
| Linux | ELF (Executable and Linkable Format) | `readelf`、`objdump`、`nm` |
| macOS | Mach-O | `otool`、`nm` |
| Windows | PE/COFF (Portable Executable) | `dumpbin`、`llvm-readobj` |

三种格式的设计哲学相似：都把二进制分成若干"段/节"（section），每段有不同用途。

### 2.2 常见段（Section）

| 段名 | 内容 |
|------|------|
| `.text` | 可执行代码（机器指令） |
| `.data` | 已初始化的全局/静态变量 |
| `.bss` | 未初始化的全局/静态变量（不占文件空间，加载时清零） |
| `.rodata` | 只读数据（字符串常量、`const` 全局） |
| `.symtab` / `.strtab` | 符号表和字符串表 |
| `.rel.text` / `.rela.text` | 重定位条目 |

### 2.3 符号（Symbol）

符号是链接器工作的基本单位。每个函数、全局变量在目标文件里都是一个符号条目，包含：

- **名字**：经过 mangling 后的字符串（如 `_ZN7testing8internal19MakeAndRegisterTestInfoE...`）
- **类型**：函数 / 对象 / 未定义（需外部提供）
- **绑定**：全局（global）/ 局部（local）/ 弱（weak）
- **节索引**：符号定义在哪个段，或 `UND`（undefined，需要链接器解析）

### 2.4 查看符号的常用命令

```bash
# Linux — 列出目标文件的所有符号
nm -C my_file.o          # -C 自动 demangle

# Linux — 查看共享库的动态符号
nm -D libgtest.so

# Windows MSVC — dumpbin
dumpbin /symbols my_file.obj
dumpbin /exports gtest.dll

# 跨平台 LLVM 工具
llvm-nm -C my_file.o
llvm-readobj --symbols my_file.obj

# demangle 工具
echo "_ZN7testing14AssertionResultD1Ev" | c++filt
# → testing::AssertionResult::~AssertionResult()

# MSVC mangling demangle
undname "?SetUp@Test@testing@@MEAAXXZ"
# → protected: virtual void testing::Test::SetUp(void)
```

---

## 3. 链接：静态与动态

### 3.1 静态链接

- **库文件**：`.a`（Linux/MinGW）、`.lib`（MSVC 静态库）
- `.a` 只是一组 `.o` 文件的打包（`ar` 命令创建）
- 链接时把库中被引用的目标文件**直接合并**到最终二进制
- 优点：无运行时依赖，部署简单
- 缺点：二进制体积大，安全更新需重新编译

### 3.2 动态链接

- **库文件**：`.so`（Linux）、`.dylib`（macOS）、`.dll`（Windows）
- 链接时只记录"我需要这个符号来自某个共享库"，不复制代码
- 运行时由动态链接器加载并绑定
- 优点：节省内存（多进程共享一份代码），可独立更新
- 缺点：DLL hell / soname 版本管理

### 3.3 Windows DLL 的特殊性

Windows DLL 与 Linux `.so` 有本质区别：

1. **默认不导出**：DLL 中的符号默认不可见，必须显式标记导出：
   ```cpp
   __declspec(dllexport) void my_func();  // 导出
   __declspec(dllimport) void my_func();  // 导入声明
   ```

2. **Import Library**：链接 DLL 时不直接链接 `.dll`，而是链接一个"导入库"：
   - MSVC：`.lib`（同名但和静态 `.lib` 是不同东西）
   - MinGW：`.dll.a`（GNU 格式导入库）

3. 本项目 GTest 用的是 `libgtest.dll.a`（MinGW 导入库），运行时需要 `libgtest.dll`

### 3.4 符号可见性

Linux/macOS 的共享库默认导出所有全局符号（和 Windows 相反）。最佳实践是限制可见性：

```bash
# 编译时默认隐藏，按需显式导出
-fvisibility=hidden
```

```cpp
__attribute__((visibility("default"))) void exported_func();
```

---

## 4. ABI 定义与组成

**ABI（Application Binary Interface）** 定义了已编译代码之间如何互相调用的二进制级约定。
如果两段代码遵循相同 ABI，它们可以链接在一起正确工作；否则会出现链接错误或运行时崩溃。

ABI 由以下几个维度组成：

### 4.1 调用约定（Calling Convention）

规定函数调用时参数和返回值如何传递：

| 约定 | 平台 | 参数传递 | 栈清理 |
|------|------|---------|--------|
| System V AMD64 ABI | Linux / macOS x86_64 | 前 6 个整数参数用 RDI, RSI, RDX, RCX, R8, R9；浮点用 XMM0-7 | 调用者 |
| Microsoft x64 | Windows x86_64 | 前 4 个参数用 RCX, RDX, R8, R9（不管整数/浮点）；要求 32 字节 shadow space | 调用者 |
| cdecl | x86 32-bit | 栈传参，从右到左 | 调用者 |
| stdcall | Win32 API | 栈传参，从右到左 | 被调用者 |
| vectorcall | MSVC 优化 | 前 6 向量参数用 XMM/YMM 寄存器 | 调用者 |

注意：**Linux 和 Windows 的 64 位调用约定不同**——即使都是 x86_64，参数传递规则也不一样。
MinGW 虽然用的是 GNU 工具链，但在 Windows 上遵循 Microsoft x64 调用约定（这是为了能调用
Windows API）。

### 4.2 名字修饰（Name Mangling）

C++ 支持重载、命名空间、模板——但链接器只认扁平的符号名。编译器把函数签名编码成唯一的
字符串，这个过程叫 name mangling。

**两大方案完全不兼容**：

| | Itanium C++ ABI | MSVC |
|---|---|---|
| 使用者 | GCC、Clang（GNU 模式）| MSVC、Clang（MSVC 模式）|
| 前缀 | `_Z` | `?` |
| 示例 | `_ZN7testing14AssertionResultD1Ev` | `??1AssertionResult@testing@@QEAA@XZ` |
| demangle 工具 | `c++filt` | `undname` |

同一个函数 `testing::AssertionResult::~AssertionResult()` 在两种方案下的修饰名完全不同。
这就是为什么 **MSVC 编译的库不能给 MinGW 用，反之亦然**。

### 4.3 数据类型布局

ABI 规定了每种类型的大小、对齐和内存布局：

```cpp
struct Example {
    char a;     // offset 0, size 1
    // 3 bytes padding (alignment of int)
    int b;      // offset 4, size 4
    short c;    // offset 8, size 2
    // 2 bytes padding (struct alignment = max member alignment = 4)
};
// sizeof(Example) = 12 on most platforms
```

不同编译器对于以下内容的布局可能不同：
- `long` 的大小（Windows: 4 字节；Linux x64: 8 字节）
- `bool` 的表示
- 位域（bitfield）的分配策略
- 空基类优化（EBO）的实现细节

### 4.4 虚表（vtable）布局

多态类的虚表是 ABI 的核心部分：

**Itanium ABI**（GCC / Clang GNU 模式）：
- vtable 指针位于对象头部（offset 0）
- vtable 布局：RTTI 指针在 offset -1，虚函数从 offset 0 开始
- 多重继承时每个基类有自己的 vtable 段（VTT）

**MSVC ABI**：
- vtable 指针同样在对象头部
- 但布局、RTTI 结构、多重继承时的 thunk 机制都和 Itanium 不同
- `dynamic_cast` / `typeid` 的实现完全不同

### 4.5 异常处理模型

| 模型 | 平台 | 原理 |
|------|------|------|
| DWARF | Linux x86_64 | 用 `.eh_frame` 段记录 unwind 信息，零开销（无异常时不影响性能） |
| SjLj (setjmp/longjmp) | 旧 MinGW 32-bit | 用 setjmp 保存上下文，有运行时开销 |
| SEH (Structured Exception Handling) | Windows 64-bit | 操作系统级异常表（`.pdata` / `.xdata`），零开销 |
| Compact Unwind | macOS | DWARF 的优化版本 |

MinGW 64-bit 的现代版本（包括 MSYS2 ucrt64/clang64）使用 SEH，与 Windows 原生一致。

---

## 5. C Runtime（CRT）

CRT 是每个 C/C++ 程序都链接的最底层库，提供：

- 程序入口点（`_start` → `__libc_start_main` → `main`）
- 标准 C 库函数（`malloc`、`printf`、`fopen`...）
- 全局对象的构造/析构调度
- `atexit` 和进程退出处理
- 线程本地存储（TLS）初始化

### 5.1 Linux 的 CRT

| 实现 | 说明 |
|------|------|
| **glibc** | GNU C Library，绝大多数桌面/服务器 Linux 发行版的标配 |
| **musl** | 轻量级实现，Alpine Linux 用，静态链接友好 |

Linux 上通常不需要关心 CRT 选择——一个系统只有一种 libc。

### 5.2 Windows 的 CRT

Windows 的复杂性在于有**两代 CRT 共存**：

| | MSVCRT (`msvcrt.dll`) | UCRT (`ucrtbase.dll`) |
|---|---|---|
| 年代 | Windows 95 时代 | Windows 10+ / VS 2015+ |
| 维护状态 | 功能冻结，不再更新 | 持续更新 |
| C 标准 | C89 为主，部分 C99 | 完整 C11/C17 |
| 谁用 | MSYS2 mingw64 环境 | MSVC、MSYS2 ucrt64/clang64 |
| 问题 | `snprintf` 行为不标准、locale 问题多 | 无 |

**为什么不能混用**：每个 CRT 实现维护自己的：
- 内存堆（`malloc` 分配的内存只能由同一个 CRT 的 `free` 释放）
- `FILE*` 结构体（一个 CRT 的 `fopen` 返回的指针传给另一个 CRT 的 `fread` 会崩溃）
- `errno` 线程本地变量

如果一个可执行文件链接了 UCRT，但加载了一个链接 MSVCRT 的 DLL，两者之间不能传递
`FILE*` 指针或让一方 `free` 另一方 `malloc` 的内存。

### 5.3 CRT 链接方式（MSVC）

| 选项 | 含义 | 运行时依赖 |
|------|------|-----------|
| `/MD` | 动态链接 UCRT（`ucrtbase.dll` + `vcruntime140.dll`） | 需要目标机器有 VC Redistributable |
| `/MT` | 静态链接 CRT 到可执行文件 | 无外部依赖，但二进制更大 |
| `/MDd` / `/MTd` | Debug 版本（额外检查） | 开发机 |

vcpkg 的 triplet 通过 `VCPKG_CRT_LINKAGE` 控制：
- `x64-windows`：`dynamic`（对应 `/MD`）
- `x64-windows-static`：`static`（对应 `/MT`）

---

## 6. C++ 标准库

C++ 标准库是在 CRT 之上的一层，提供 `std::string`、`std::vector`、`<iostream>`、
`<algorithm>` 等。**它不是操作系统的一部分，是编译器/工具链附带的**。

### 6.1 三大实现

| 实现 | 维护者 | 配合使用 | 内联命名空间 |
|------|--------|---------|-------------|
| **libstdc++** | GNU | GCC（也可被 Clang 使用） | `std::__cxx11`（ABI breaking 类型） |
| **libc++** | LLVM | Clang（也可被 GCC 使用） | `std::__1`（所有类型） |
| **MSVC STL** | Microsoft | MSVC、clang-cl | `std`（无额外命名空间） |

### 6.2 为什么互不兼容

即使两个库都实现了 `std::string`，它们的**内存布局完全不同**：

```
libstdc++ std::string (SSO, GCC 5+):
┌──────────────────────────────────────────┐
│ pointer (8) │ size (8) │ union {         │
│             │          │   buffer[16]    │  ← SSO（短字符串优化）
│             │          │   capacity (8)  │
│             │          │ }               │
└──────────────────────────────────────────┘
sizeof = 32 bytes

libc++ std::string (SSO):
┌──────────────────────────────────────────┐
│ union {                                  │
│   short: is_long(1 bit) + size(7) + buf[23] │
│   long:  is_long(1 bit) + cap + size + ptr  │
│ }                                        │
└──────────────────────────────────────────┘
sizeof = 24 bytes

MSVC STL std::string (SSO):
┌──────────────────────────────────────────┐
│ union {                                  │
│   buf[16] (SSO)                          │
│   ptr + ...                              │
│ } │ size (8) │ capacity (8)             │
└──────────────────────────────────────────┘
sizeof = 32 bytes (x64)
```

如果库 A 用 libstdc++ 编译，传出一个 `std::string`，库 B 用 libc++ 编译去读它——
两者对同一块内存的解读不同，**必然崩溃或数据错乱**。

### 6.3 名字修饰中的体现

libc++ 用内联命名空间 `std::__1` 包裹所有类型。所以链接时你会看到：

```
# libstdc++ 编译的代码期望找到：
testing::internal::MakeAndRegisterTestInfo(std::basic_string<char, ...>, ...)

# libc++ 编译的代码产生的符号：
testing::internal::MakeAndRegisterTestInfo(std::__1::basic_string<char, ...>, ...)
```

这两个符号名不同（mangling 不同），链接器报 `undefined symbol`——就是本仓库
`mingw-clang` preset 最初遇到的第二个报错。

### 6.4 一个进程能否混用？

- **Linux**：理论上可以让不同 SO 各自用不同 stdlib，但跨 SO 边界传递 `std::string`
  等类型就会出问题。实践中应统一。
- **Windows**：不可以。DLL 边界传递 C++ 对象本身就是危险操作（涉及内存分配器归属），
  混 stdlib 更是雪上加霜。
- **结论**：一个项目内所有编译单元必须用同一个 C++ stdlib。

---

## 7. 平台 ABI 全景对比

| 维度 | Linux x86_64 | macOS (ARM64) | Windows MSVC | Windows MinGW |
|------|-------------|---------------|-------------|---------------|
| 目标文件格式 | ELF | Mach-O | PE/COFF | PE/COFF |
| C++ ABI 标准 | Itanium | Itanium | MSVC (proprietary) | Itanium |
| 调用约定 | System V AMD64 | AAPCS64 | Microsoft x64 | Microsoft x64 |
| 名字修饰 | Itanium mangling | Itanium mangling | MSVC mangling | Itanium mangling |
| C++ 标准库 | libstdc++ | libc++ | MSVC STL | libstdc++ 或 libc++ |
| C Runtime | glibc (动态) | libSystem | UCRT | UCRT 或 MSVCRT |
| 异常处理 | DWARF | Compact Unwind | SEH | SEH |
| DLL 导出 | 默认全部导出 | 默认全部导出 | 必须 `__declspec` | 默认全部导出 |
| 调试信息 | DWARF | DWARF | PDB (CodeView) | DWARF 或 CodeView |

### 关键观察

1. **Linux/macOS 上 ABI 基本统一**：GCC 和 Clang 都遵循 Itanium C++ ABI，用同一套
   mangling 规则，产出的 `.o` 可以互链。唯一需注意的是 stdlib 选择。

2. **Windows 上存在两个 ABI 世界**：
   - MSVC ABI：`cl.exe`、`clang-cl`、MSVC STL
   - MinGW/GNU ABI：`x86_64-w64-mingw32-gcc`、`x86_64-w64-mingw32-clang++`、libstdc++/libc++

   两者**完全不能互链**（mangling 不同、vtable 不同、异常机制实现不同）。

3. **MinGW 的特殊位置**：用 Itanium ABI + PE/COFF 格式 + Microsoft x64 调用约定。
   调用约定和 MSVC 相同（能调 Windows API），但 C++ 层面的 mangling 和 vtable 遵循 Itanium。

---

## 8. Clang 的多目标能力与 LLVM

### 8.1 LLVM 架构

```
┌────────────────────────────────────────────────────────────────┐
│                        LLVM 项目                                │
├────────────┬──────────────────┬───────────────────────────────┤
│  前端      │    优化器         │         后端                   │
│            │                  │                               │
│  Clang     │  LLVM IR Passes  │  x86 / ARM / RISC-V / ...    │
│  (C/C++/ObjC)                 │                               │
│            │                  │  目标代码生成                   │
└────────────┴──────────────────┴───────────────────────────────┘
```

- **Clang** 是 LLVM 的 C/C++/Objective-C 前端
- Clang 把源码编译成 **LLVM IR**（中间表示）
- LLVM 后端把 IR 翻译成目标平台的机器码
- 同一个 Clang 二进制文件可以针对多个目标生成代码（通过 `--target`）

### 8.2 Target Triple

Target triple 是 Clang 选择目标 ABI 的核心参数，格式为：

```
<arch>-<vendor>-<os>-<environment>
```

常见示例：

| Triple | 含义 |
|--------|------|
| `x86_64-pc-linux-gnu` | Linux x64, GNU 工具链 |
| `x86_64-apple-darwin` | macOS x64 |
| `aarch64-apple-darwin` | macOS ARM64 (Apple Silicon) |
| `x86_64-pc-windows-msvc` | Windows x64, MSVC ABI |
| `x86_64-w64-windows-gnu` | Windows x64, MinGW/GNU ABI |

Triple 决定了：
- 使用哪种调用约定
- 使用哪种名字修饰方案
- 搜索哪个 sysroot 的头文件和库
- 用哪种异常处理模型
- 默认链接哪个 C++ stdlib

### 8.3 为什么同一个 Clang 在不同环境下行为不同

Clang 的默认 target 由**编译 Clang 时的配置**决定：

| 安装来源 | 默认 target | 默认 C++ stdlib |
|---------|-------------|----------------|
| scoop `llvm` 包 | `x86_64-pc-windows-msvc` | MSVC STL |
| MSYS2 clang64 (`mingw-w64-clang-x86_64-clang`) | `x86_64-w64-windows-gnu` | libc++ |
| MSYS2 ucrt64 (`mingw-w64-ucrt-x86_64-clang`)* | `x86_64-w64-windows-gnu` | libstdc++ |
| Ubuntu `apt install clang` | `x86_64-pc-linux-gnu` | libstdc++ |
| macOS Xcode (`Apple Clang`) | `arm64-apple-darwin` | libc++ |

*ucrt64 的 clang 需要额外安装 `mingw-w64-ucrt-x86_64-clang` 包。

这就是本仓库最初遇到的问题根源：`mingw-clang` preset 写了 `CMAKE_CXX_COMPILER=clang++`，
但 scoop 的 clang++ 默认 target 是 `x86_64-pc-windows-msvc`，生成 MSVC ABI 的代码，
与 vcpkg 用 MinGW GCC 编译的 GTest（Itanium ABI + libstdc++）不兼容。

### 8.4 手动切换 ABI 的关键参数

```bash
# 切换 target ABI
clang++ --target=x86_64-w64-windows-gnu  # 产出 MinGW ABI
clang++ --target=x86_64-pc-windows-msvc  # 产出 MSVC ABI

# 切换 C++ stdlib
clang++ -stdlib=libstdc++  # 使用 GNU libstdc++
clang++ -stdlib=libc++     # 使用 LLVM libc++

# 切换链接器
clang++ -fuse-ld=lld       # LLVM linker (GNU 兼容模式)
clang++ -fuse-ld=lld-link  # LLVM linker (MSVC 兼容模式)
```

### 8.5 与 clangd 的关系

clangd（C++ Language Server）复用 Clang 的前端来分析代码。它通过
`compile_commands.json` 读取每个文件的编译命令（包括 `--target`、`-stdlib` 等），
按完全相同的 ABI 规则来做语义分析。

如果 clangd 读到的编译命令和实际构建时用的不一致，就会出现"编辑器里没报错但构建失败"
或反之的情况。详见 [`clangd-toolchain-overview.md`](clangd-toolchain-overview.md)。

---

## 9. MSYS2 子系统与 ABI 对应关系

MSYS2 提供了多个**相互独立**的 MinGW 工具链子系统，每个都是自洽的生态：

| 子系统 | 编译器 | C++ stdlib | C Runtime | 包前缀 |
|--------|--------|-----------|-----------|--------|
| **ucrt64** | GCC | libstdc++ | UCRT | `mingw-w64-ucrt-x86_64-` |
| **mingw64** | GCC | libstdc++ | MSVCRT | `mingw-w64-x86_64-` |
| **clang64** | Clang | libc++ | UCRT | `mingw-w64-clang-x86_64-` |
| clang32 | Clang (32-bit) | libc++ | UCRT | `mingw-w64-clang-i686-` |
| mingw32 | GCC (32-bit) | libstdc++ | MSVCRT | `mingw-w64-i686-` |
| clangarm64 | Clang (ARM64) | libc++ | UCRT | `mingw-w64-clang-aarch64-` |

### 9.1 为什么不能跨子系统链接

每个子系统有自己独立的：
- 编译器二进制（`ucrt64/bin/gcc.exe` vs `clang64/bin/clang.exe`）
- 头文件（`ucrt64/include/c++/` vs `clang64/include/c++/`）
- 运行时库（`ucrt64/lib/libstdc++.dll.a` vs `clang64/lib/libc++.dll.a`）
- 包管理的二进制包（MSYS2 pacman 按子系统分别打包）

跨子系统链接会导致：
- **C++ stdlib ABI 不兼容**（libstdc++ vs libc++，如 §6 所述）
- **CRT 可能不兼容**（ucrt64 用 UCRT vs mingw64 用 MSVCRT）
- **头文件路径混乱**（include 到错误子系统的头文件）

### 9.2 vcpkg triplet 与子系统的映射

vcpkg 的内置 `x64-mingw-dynamic` triplet 不指定编译器，CMake 自动搜索到 PATH 上的
GCC。这意味着：

| PATH 上的子系统 | vcpkg 找到的编译器 | 构建产物的 ABI |
|----------------|-------------------|---------------|
| ucrt64 | `x86_64-w64-mingw32-g++` (ucrt64) | GCC + libstdc++ + UCRT |
| mingw64 | `x86_64-w64-mingw32-g++` (mingw64) | GCC + libstdc++ + MSVCRT |
| clang64 | 仍然找 GCC（CMake 不搜索 clang）→ 如果 ucrt64 也在 PATH 就找到 ucrt64 的 | 不确定 |

对于 clang64 用户，vcpkg 的自动检测逻辑会"越界"找到其他子系统的 GCC，产出 ABI 不匹配
的库。这就是为什么需要自定义 triplet（见 [§10](#10-本项目的-abi-决策)）。

### 9.3 推荐选择

| 场景 | 推荐子系统 | 理由 |
|------|-----------|------|
| 通用 MinGW 开发 | ucrt64 | 现代 UCRT，GCC 生态成熟，vcpkg 开箱即用 |
| 需要 Clang 特性（概念、模块等） | clang64 | Clang 对新标准支持更快，但需自定义 vcpkg triplet |
| 兼容老 Windows / 遗留库 | mingw64 | MSVCRT 兼容 XP/Vista 时代的 DLL |

---

## 10. 本项目的 ABI 决策

### 10.1 整体策略

```
┌─────────────────────────────────────────────────────────────┐
│                    本项目 preset 与 ABI 映射                  │
├─────────────┬──────────────┬────────────────────────────────┤
│ Preset 族   │ vcpkg triplet │ ABI 特征                       │
├─────────────┼──────────────┼────────────────────────────────┤
│ gcc-*       │ (自动检测)    │ Itanium + libstdc++ + glibc    │
│ clang-*     │ (自动检测)    │ Itanium + libstdc++ + glibc    │
│ msvc-*      │ x64-windows  │ MSVC + MSVC STL + UCRT         │
│ clang-cl-*  │ x64-windows  │ MSVC + MSVC STL + UCRT         │
│ mingw-gcc-* │ x64-mingw-dynamic        │ Itanium + libstdc++ + UCRT │
│ mingw-clang-*│ x64-mingw-clang-dynamic │ Itanium + libc++ + UCRT    │
└─────────────┴──────────────┴────────────────────────────────┘
```

### 10.2 为什么 Linux/macOS 不固化 triplet

每个 OS 上只有一种主流 ABI：
- Linux：Itanium + libstdc++ + glibc（不论用 GCC 还是 Clang）
- macOS：Itanium + libc++ + libSystem

vcpkg 按主机 OS 自动检测即可（`x64-linux` / `arm64-osx`），硬编码反而会阻碍 ARM64
等架构的用户。

### 10.3 为什么 Windows 必须显式固化 triplet

Windows 上存在两个互不兼容的 ABI 世界（MSVC vs MinGW），vcpkg 默认猜 `x64-windows`
（MSVC ABI）。对 MinGW 用户来说这是错误的，必须在 preset 中显式声明。

### 10.4 mingw-clang 为什么需要自定义 vcpkg triplet

三层问题叠加：

1. **CMake 不搜索 Clang**：内置 `x64-mingw-dynamic` triplet 不指定编译器，CMake 对
   MinGW 系统名只搜索 `gcc` / `g++` → 永远找到 ucrt64 的 GCC

2. **GCC 用 libstdc++，Clang64 用 libc++**：即使链接成功，C++ 类型的内存布局也不同

3. **vcpkg 不继承项目的 CMAKE_CXX_COMPILER**：vcpkg 内部构建是独立的 CMake 进程

解决方案是自定义 overlay triplet（`triplets/x64-mingw-clang-dynamic.cmake`），通过
`VCPKG_CHAINLOAD_TOOLCHAIN_FILE` 强制 vcpkg 内部也用 clang64 的 Clang 编译包：

```cmake
# triplets/x64-mingw-clang-dynamic.cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_ENV_PASSTHROUGH PATH)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_LIST_DIR}/clang64-toolchain.cmake")
```

这样整条链路都在 clang64 生态内闭合（Clang + libc++ + UCRT），ABI 一致。

详细的 vcpkg triplet 操作见 [`vcpkg-guide.md` §5-6](vcpkg-guide.md#5-tripletabi-的命名空间)。

### 10.5 ABI 不匹配的典型报错

供排查参考——如果你在构建中看到这些错误，大概率是 ABI 问题：

| 报错特征 | 可能原因 |
|---------|---------|
| `__declspec(dllimport) ... undefined symbol` | MSVC ABI vs MinGW ABI 混用 |
| `std::__1::basic_string` undefined | libc++ 编译的代码链接 libstdc++ 编译的库 |
| `std::__cxx11::basic_string` undefined | libstdc++ 新 ABI 链接旧 ABI（`_GLIBCXX_USE_CXX11_ABI`） |
| 大量 GTest 符号 undefined（`testing::Test::SetUp` 等） | 测试框架和测试代码 ABI 不匹配 |
| 运行时 `malloc`/`free` 崩溃 | CRT 混用（UCRT 分配的内存被 MSVCRT 释放） |
| `ld.lld: error: undefined symbol: vtable for ...` | vtable 布局不兼容（跨 ABI 继承） |

---

## 附录：术语速查

| 术语 | 英文 | 简释 |
|------|------|------|
| ABI | Application Binary Interface | 已编译代码的二进制级互操作约定 |
| API | Application Programming Interface | 源码级接口（头文件声明） |
| CRT | C Runtime | C 标准库运行时（malloc/printf 等的实现） |
| TU | Translation Unit | 一个 .cpp 经预处理后得到的完整编译单元 |
| mangling | Name Mangling / Name Decoration | 把 C++ 函数签名编码为链接器符号名 |
| triplet | — | vcpkg 中描述目标 ABI 的命名字符串 |
| target triple | — | Clang/LLVM 中描述目标平台的 `arch-vendor-os-env` 字符串 |
| SSO | Small String Optimization | std::string 在对象内部就地存储短字符串 |
| PLT/GOT | Procedure Linkage Table / Global Offset Table | ELF 动态链接的延迟绑定机制 |
| IAT | Import Address Table | PE 格式的动态符号导入表 |
| SEH | Structured Exception Handling | Windows 操作系统级异常处理机制 |
| DWARF | — | 调试信息和 unwind 信息的标准格式 |
| PDB | Program Database | MSVC 的调试信息文件格式 |
