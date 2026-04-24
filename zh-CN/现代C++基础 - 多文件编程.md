多文件编程（Programming in Multiple Files）

## 现代C++基础

## 现代 C++ 基础

梁鉴铭（Jiaming Liang），北京大学本科生

- 预处理器（Preprocessor）

- 声明与定义（Declaration and Definition）
- 头文件与源文件（Header files and source files）
- 命名空间（Namespace）

- 内联（Inline）

- 链接属性（Linkage）

- XMake 与如何制作库（XMake & how to make a library）

- 模块（Modules）

# 在那之前……

- C++20 已经引入了模块（modules），理论上可以显著加快编译时间。
- 不过，我们在课程里仍然使用头文件，原因是：
- 现实世界里大量既有 C++ 代码仍广泛使用头文件，你未来职业生涯里很可能仍需要维护它们。
- 各编译器对模块的支持仍在持续推进；编译器开发者日夜赶工，但这确实是一个复杂而庞大的工程，尚未彻底完成。
- 标准库要到 C++23 才完全“模块化”（这也仍在推进中）；在那之前，gcc 甚至不支持直接 import 标准库。

来源：https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Modules.html

# 在那之前……

- 模块里仍有不少问题，无论是缺陷还是性能问题。
- 例如在 Clang 中：模块反而增加了构建时间，等等。
- 顺便一提，Clang 中模块的一位主要开发者是中国人：许传奇。
- 最后，模块在 IDE 自动补全/提示方面依旧很糟糕；即便在 Visual Studio 里，模块也可能触发错误的波浪线并被错误分析，让你感觉像在没有任何插件的 Vim 里写代码。
- 总之，我相信模块在未来会非常重要……
- 像这样写 C++ 不是很酷吗：
- 但现在还不是大规模使用它们的时机。
- 你可以在小型实验项目里尝试。

# 多文件编程（Programming in multiple files）

预处理器（Preprocessor）

# 预处理器（Preprocessor）

- 还记得在 ICS 里学过的 C 语言编译流程吗？
- 1. 预处理：去掉注释并处理宏（这样你的 #include 才能把文件正确拼进来）。
- 2. 编译：每个源文件独立编译。
- 声明/定义（Def/Decl）会讲到！
- 3. 汇编：这里我们不太关心，因为它只是把汇编翻译成目标文件。
- 4. 链接：解析在其他翻译单元（TU）中被引用的符号。
- 链接（Linkage）会讲到！

# 预处理器（Preprocessor）

在 gcc 中由 –finput-charset=xx 指定，在 msvc 中由 /source-charset:xx 指定。
- 更具体地说，预处理可以分成 6 个阶段：
- 1. 读取文件并映射到翻译字符集（translation character set），从而保证支持 UTF-8（我们会在“字符串与流”里讲 UTF-8；它是 ASCII 的超集编码）。
- 在 C++23 之前 / 在 C 中，是否支持 Unicode 是实现定义的（implementation-defined）。
- 2. 处理行末的反斜杠，把两条物理行拼成一条逻辑行。
- 3. 抽取注释与宏，并解析整个文件。
- 你会在《编译原理》里学习如何解析一门语言。
- 随后注释会被剥离。
- 4. 预处理器运行并处理所有宏。
- 5. 字符串字面量按前缀指定的规则进行编码。
- 没有前缀的，由执行字符集（execution character set）决定。
- 6. 相邻字符串字面量会被拼接（例如 `"1" "2"` -> `"12"`）。

在 gcc 中由 –fexec-charset=xx 指定，在 msvc 中由 /execution-charset:xx 指定。

# 预处理器（Preprocessor）

- 预处理指令以 # 开头。
- 一共有四类指令。
- 1. `#include …`：把文件的全部内容复制到当前文件。
- `#include <…>` 会在系统路径或编译器选项指定的路径里查找文件（例如 `–Isome/path`）。
- `#include "…"` 会在本地查找文件（例如相对当前文件路径）；若失败，再回退到 `<…>` 的查找方式。
- 2. `#define`：也就是宏，做的是纯文本替换。
- 例如：
- 宏不应彼此递归引用，因为预处理器会在当前替换链上抑制（suppress）宏。
- 例如：`#define A B`，`#define B C`，`#define C A`，那么 A 会展开到 B，B 会展开到 C，但 C 永远不会再展开回 A，因为当前链是 A->B->C，于是 A/B/C 的任意进一步展开都会被抑制。

# 宏（Macros）

- 除此之外，宏还可以像“函数”那样工作。
- 例如：
- 仍然要记住：它是纯文本替换，所以：
- 通常你需要到处加括号，例如：

- 这也可能造成性能问题：

- 函数会被多调用一次！

# 宏（Macros）

- 注 1：宏的参数可以为空（例如 `SQR()` 会得到 `(()*())`，`Func(,)` 会得到 `+`）。
- 注 2：当参数没有被括号包裹时，宏参数会直接按逗号切分解析，因此例如 `FUNC(SomeFunc<int, double>())` -> `SomeFunc<int + double>()` 等。
- 你需要再加一层括号来帮助解析，例如 `FUNC((SomeFunc<int, double>()), 3)`。
- 注 3：你可以用 `…` 表示任意数量参数，并通过 `__VA_ARGS__` 引用它们。例如：

# 宏（Macros）

- 注 4：从 C++20 起，可以使用 `__VA_OPT__(content)`，它仅在 `__VA_ARGS__` 非空时启用。
- 例如：

- 注 5：`#` 可以把参数转成字符串，`##` 可以把参数拼接成一个完整记号（token）。
- 例如：

# 宏替换序列（Macro replacement sequence）*

- 首先，直到 VS2019 为止 MSVC 的预处理器都是非标准的；之后你需要用 `/Zc:preprocessor` 启用标准预处理器。
- 这是可选项，我真的建议你跳过这一部分；万一未来读到一些“味道很重”的代码，再回来看这一页也不迟。
- 展开可以分为四步：
- 1. 若宏是函数式（function-like）宏，则先解析并展开其参数。
- 若宏后面没有紧跟 `()`，则推迟到第 4 步再展开。
- 2. 把参数替换进宏体。
- 3. 把已展开的宏加入抑制链（suppress chain）。
- 4. 从替换后的文本重新扫描（rescan），看是否还能处理不在抑制链里的宏。

# 宏替换序列（Macro replacement sequence）*

- 例如：

- 那么 `EXAMPLE(5)` 会展开为：
- `EXAMPLE_ EMPTY()(5-1)(5)`。
- 然后把 `EXAMPLE` 加入抑制链。
- 再重新扫描替换结果：
- `EXAMPLE_` 后面没有 `()`，因此不会被替换（与定义冲突）。
- `EMPTY` 会被替换，得到 `EXAMPLE_()(5-1)(5)`。
- 重新扫描，但注意新替换出来的内容从空白处开始（也就是重新扫描 `()(5-1)(5)`），于是不会发生进一步变化。
- 那么 `SCAN(EXAMPLE(5))` 呢？

# 宏替换序列（Macro replacement sequence）*

- 由于 `SCAN` 是后面跟着 `()` 的函数式宏，因此先展开参数 `EXAMPLE(5)`。
- 得到 `EXAMPLE_ EMPTY()(5-1)(5)`。
- `EXAMPLE` 被加入抑制链。
- 用与之前相同的分析重新扫描，你只会得到 `EXAMPLE_()(5-1)(5)`。
- 当前展开结束，于是 `EXAMPLE` 从抑制链弹出。
- 然后把 `EXAMPLE_()(5-1)(5)` 替换进 `SCAN` 的宏体，并把 `SCAN` 加入抑制链（此时链里只有 `SCAN`）。
- 再重新扫描替换结果：
- `EXAMPLE_` 后面跟着 `()`，没有参数，于是展开为 `EXAMPLE`。
- 然后把 `EXAMPLE_` 加入抑制链。
- 重新扫描 `EXAMPLE(5-1)(5)`，于是把 `EXAMPLE(5-1)` 展开为 `EXAMPLE_ EMPTY()(5-1- 1)(5-1)`；由于 `EXAMPLE_` 在抑制链里，所以最终结果只是 `EXAMPLE_ ()(5-1-1)(5-1)(5)`。

更多例子见这里。

# 宏（Macros）

- 宏不受命名空间控制（都被视为全局），因此很容易污染作用域之外的名称。
- 例如，有些库可能间接包含了名为 `max` 的宏，从而干扰标准库里的 `std::max`。
- 例如 Windows 的 `<Windows.h>` / `<minwindef.h>`。
- 注意你的 IDE 配色；函数与宏通常颜色不同。
- 然后你需要 `#undef` 来取消宏的定义，也就是在当前作用域禁用它。
- 这里就是 `#undef max`。

# 宏（Macros）

- 关于函数式宏的最后一句：
- 真的非常不鼓励使用函数式宏；请用函数代替！
- 缺点很多：
- 1. 展开过程很晦涩，尤其当宏嵌套时。
- msvc 也不能直接完全支持标准行为。
- 2. 不支持递归。
- 3. 参数按逗号直接解析，需要额外括号辅助。
- 4. 可能造成性能损失，写定义时也需要额外括号，因为它是纯文本替换。
- 5. 可变参数相关操作能力有限（以后我们会讲普通函数里如何写可变参数，那要强大得多）。
- 6. 不受命名空间控制，可能冲突。

# 预处理器（Preprocessor）

- 第三类功能是按条件选择代码……
- 根据宏是否被定义：
- `#ifdef`、`（从 C++23 起还有 #elifdef）`、`#else`；
- `#ifndef`、`（从 C++23 起还有 #elifndef）`、`#else`；
- `#if defined xx` 等价于 `#ifdef xx`。
- 你需要用 `#endif` 结束它们。
- 经典用法是定义一个 `DEBUG` 宏，然后：
- 如果你不定义 `DEBUG`，那么这段代码会被省略并且不会被编译。
- 我们很快也会在头文件里用到它们。

# 预处理器（Preprocessor）

- 或者由“值”来控制。
- `#if`、`#elif`、`#else`；
- 它们只判断纯字面量，而不是“编译期可确定”的变量，因为编译发生在预处理之后。
- 例如：不要这样：

- 所有非宏名字都会变成 0。
- 还有一些特殊记号/宏：
- 从 C++17 起有 `__has_include(…)`，用于检测当前文件是否（直接或间接）包含了某个文件。

# 预处理器（Preprocessor）

更多预定义宏见这里。
- 属性测试宏：用 `__has_cpp_attribute(…)`；例如 `#if __has_cpp_attribute(nodiscard)`；
- 语言特性宏：例如 `#if __cpp_lambdas` 用于 lambda 表达式。
- 从 C++20 起，在 `<version>` 里定义了库特性宏，例如 `#if __cpp_lib_bit_cast` 用来检查是否实现了 `std::bit_cast`。
- `__cplusplus`：指定 C++ 版本（msvc 需要 `/Zc:__cplusplus` 才能启用，否则永远是 C++98 的宏值）。
- 不过这些宏通常不太用，因为只要你指定了标准，任何正常编译器都会实现 99% 的特性。
- 最后一件是用 `#error …` 或 `#warning …` 显式产生错误或警告。
- `#warning` 直到 C++23 才被正式支持，但此前很多编译器把它作为扩展支持。
- 例如：

# 预处理器（Preprocessor）

- 此外，你可能以前见过 `#pragma`。
- 例如，如果你上过 HPC 课程，可能知道 `#pragma unroll` 用来强迫编译器展开循环。
- `#pragma` 本质上是留给“实现定义用法”的宏式入口。
- 具体 pragma 请查编译器文档或工具文档。
- 例如：在 C++11 与 OpenMP 5.1 之前，你需要用 `#pragma` 而不是属性（attribute）来指定并行策略。

- 我们以后会讲 `#pragma once` 和 `#pragma pack`，因为这两个基本被所有编译器支持。
- 顺便，C++11 也允许写成 `_Pragma("…")`。

# 预处理器（Preprocessor）

- 最后，你可以用 `#line` 手动指定行号。

- 这在“由工具自动生成另一个文件”的场景里很常见：错误应当回到生成它的源文件去修。
- 例如你会在《编译原理》实验里用 Bison：用 `.y` 生成 `.cpp`，但错误提示仍指向 `.y`。

# 预处理器（Preprocessor）

- 最后一句：你经常在宏定义里看到 `do-while`。
- 例如：不要这样写：

- 这是为了避免反直觉情况，例如：
- 第二种情况只会把 `func(1)` 包进 `if`，`func(2)` 总会执行。

# 多文件编程（Programming in multiple files）

声明与定义（Declaration and Definition）

# 翻译单元（Translation unit）

- 现在进入编译期……
- 在 ICS 里我们知道：所有源文件分别翻译，最后链接起来消除未知符号。
- 这样一个单位叫做翻译单元（translation unit，TU）。
- 然而，由于 C++ 需要先声明后使用，每个 TU 都需要先看到声明。
- 这就是为什么你在使用 `std::vector` 之前要先 `#include <vector>`。
- 是的，头文件用来暴露公共声明，这样包含它的人就知道“如何使用它们”。

# 声明与定义（Declaration and Definition）

- 那为什么我们需要声明，而不是只用定义？
- C/C++ 有单一定义规则（One-Definition Rule，ODR）：每个实体在一个 TU 里、甚至在整份程序里，通常应当只有一个定义。
- 所以，我们可以尽可能多地暴露声明，但定义应当只有一个。
- 就像一个人可以发很多张名片（名片），但他/她始终是他/她自己，是独一无二的那一个。
- 几类典型的声明与定义：
- 函数原型 & 带函数体的定义，我们以前见过。
- 这也包括类中的成员函数。
- `class A;` 与 `class A{ … };`；`struct` 同理。
- `enum class A (: Type);` 以及带枚举项（enumerator）的完整定义。

严格来说，定义也是一种声明；下文所说的“声明”不包含这种“定义即声明”的情况。

# 声明与定义（Declaration and Definition）

- 先举个例子：• 如果我们把整个定义放进 `func.h` 会怎样？

# 声明与定义（Declaration and Definition）

- 原因：`func.cpp` 定义了一个 `Hello()`，而 `main.cpp` 也定义了一个（通过 `#include`，因为 `#include` 会把一切复制到当前文件）。
- 你可以用 msvc 的 `/P`，或 clang/gcc 的 `–E` 来验证。

msvc 预处理结果 g++ 预处理结果

# 声明与定义（Declaration and Definition）

- 我们说过类的声明类似 `class A;;` 那么是否该把声明放头文件、定义放源文件？
- 想一想：我们需要使用成员函数和数据成员，所以也应把它们的声明暴露给用户。
- 函数原型足够让别人知道如何调用；但仅有类声明远远不够！
- 因此，尽管 ODR 要求函数在整个程序里只能定义一次……
- 它只要求类在每个 TU 里只能定义一次。
- 并且，不同 TU 里对同一个类的定义必须相同。
- 把类定义放在头文件里并 `#include` 它，通常就能保证这一点。
- 对成员函数来说，如果拆到类外定义，仍然要求整个程序里只定义一次。

# 声明与定义（Declaration and Definition）

- 例如：

# 声明与定义（Declaration and Definition）

- 此外，类定义要求完整看到其所有成员与基类的定义。
- 你不能 `class Vector3; class A { Vector3 v; };`
- 形式化地说：类不能拥有不完整类型（incomplete type）的成员。
- 这是因为编译器需要确定类的布局（layout）；若某些成员的定义未知，其 `sizeof` 就不清楚。
- 否则在分别编译时，它怎么知道 `Vector3` 里是 3 个 `float` 还是 3 个 `double`？
- 那为什么还需要类前置声明？
- 你需要 `#include` 头文件才能看到定义。
- 于是当头文件变化时，所有间接 `#include` 它的头文件也会被牵连改变，也就是变更会到处传播。
- 然后就需要大规模重新编译……

# 声明与定义（Declaration and Definition）

- 但如果我们在头文件里只用 `class Vector3;`，那么对 `Vector3.h` 的修改就不会影响它们，从而阻断传播。
- 我们仍可能在源文件里 `#include "Vector3.h"`，但由于源文件分别编译，重新编译通常比“到处改动”快得多。
- 所以典型地，在这些情况下可以只用类前置声明：
- 1. 作为函数原型的参数类型：不会用到成员与布局。
- 2. 当你只需要指针或引用作为成员时。
- `Vector3` 是不完整类型，但 `Vector3*` 是确定的；它没有成员，布局就是一个指针。
- 你可以在源文件 `#include "Vector3.h"` 以进一步访问成员。
- 问题：如果函数体里不使用该类，还需要定义吗？

需要，因为参数会在栈上分配，所以布局必须已知。

那 `Vector3&` 呢？布局确定、成员不用，因此可以不需要定义。

# 声明与定义（Declaration and Definition）

- 真实案例：如果你需要两个类彼此把对方作为成员……
- 这往往暗示设计有问题。
- 在 `A.h` 里 `#include "B.h"` 同时在 `B.h` 里 `#include "A.h"`？
- 不行，例如在 `B.h` 里，`A` 需要看到 `B` 的完整定义，但由于 `#include` 顺序，`A` 会用到不完整类型的成员。
- 解决办法：在 `A.h` 里前置声明 `class B;`，在 `B.h` 里前置声明 `class A;`，然后用指针做成员。
- 也就是 `A` 含 `B*`，`B` 含 `A*`。
- 然后在 `B.cpp` `#include "A.h"`，在 `A.cpp` `#include "B.h"` 来使用函数与成员。
- 如果你遇到循环包含，拆分声明与定义就是解决之道。

# 声明与定义（Declaration and Definition）

- 注 1：允许把成员函数定义放进类定义里（我们以前经常这么做）。
- 注 2：成员函数声明与定义的返回类型、参数类型应当一致。
- 特别地，`final` 与 `override` 这类出现在原型前的说明符只需要出现在声明里；其他说明符在声明与定义里都要出现。
- 换句话说，这包括 cv 限定与 `noexcept` 说明。

# 声明与定义（Declaration and Definition）

- 注 3：`friend` 会隐式声明类或函数。
- 例如：
- 在 `main.cpp` 里不要 `#include "func.h"`！
- 还有其他隐式方式，例如 `void Hello2(class Vector3);` 会同时声明 `Vector3` 与 `Hello2`；但它们很少用。

- 注 4：类定义里的类成员事实上是定义，而不只是简单声明，所以不必在 `.cpp` 里再定义一次（例如 `float Vector3::x_`）。
- 但静态数据成员只是声明，因此需要定义。于是静态变量可以在头文件里是不完整类型，只在源文件里变完整…… `Vector3.cpp`

# 声明与定义（Declaration and Definition）

- 注 5：函数的默认参数应放在声明里，而不应放在定义里。
- 除非没有声明只有定义。
- 注 6：类型别名（例如 `using`）与 `static_assert` 也是声明，也就是放进头文件是安全的。
- 注 7：尽管不鼓励使用全局变量……
- 仍要提一句：它们也应拆分声明与定义。
- 例如在头文件里 `extern int x;`，在源文件里 `int x = 0;`。
- 原因：`int x` 可能是定义（也就是一个具有未定义初值的整数），所以要加 `extern` 表示这是声明。
- 这要求变量不能有初始化器（也就是不要 `extern int x = 1;`）；否则它仍会变成定义，并可能造成多重定义。
- 我们很快会再讲 `extern`……

# 声明与定义（Declaration and Definition）

- 还有一件事……
- 我们说过类定义在一个 TU 里只能出现一次；那如果我们既 `#include "a.h"` 又 `#include "b.h"`，而 `b.h` 又间接 `#include "a.h"` 呢？
- 那么 `a.h` 会被复制两次，于是类定义出现两次……
- 怎么解决？原因：第一次包含时，`VECTOR3_H`
- 靠头文件保护（header guard）！还没被定义，于是 `#ifndef` 与 `#endif` 之间的内容会保留。

后续再包含时，由于 `VECTOR3_H` 已经定义，所有内容都会被剥离，从而避免重复定义。

# 声明与定义（Declaration and Definition）

- 头文件保护通常由文件名或类名等组成“命名空间式”的宏名。
- 不过手动写保护宏可能麻烦，所以大多数编译器支持 `#pragma once`。
- 它会自动生成头文件保护，而且可能更快。
- 唯一缺点：头文件不能共享它们的保护；因此若同一份文件被复制到很多地方，头仍可能被多次包含。
- 但通常不鼓励到处复制文件，所以问题不大。
- 一般只用 `#pragma once` 就够了；但如果你真的要支持可能没有该特性的编译器，比如写一个非常基础的库，可以用传统头文件保护。

# 模板（Template）

- 最后还有模板；我们是否也应这样写：

在头文件里。在源文件里。
- 提示：模板不会把信息保留到目标文件里。
- 它只是指定了一套实例化代码的规则。
- 所以答案是不行，因为：
- 对其他包含头文件的文件来说，由于不知道实现，它们只会生成一个实例化符号并指望链接时找到它。
- 对有实现的文件来说，它只会实例化当前 TU 里用到的那些，因为编译是分开进行的！
- 特别地，如果你本地不用它，就不会生成任何实例。

对函数模板也可以不显式写 `<int>`（即 `template void Func(const int&)`）来实例化；

# 模板（Template）

模板参数会自动推导。

- 于是链接会失败，因为找不到符号。
- 因此我们需要把模板实现放进头文件，让使用者直接实例化，而不是推迟到链接阶段。
- 如果你真想把实现放进源文件，那么模板实例就仅限于你显式实例化过的那些。
- 也就是在源文件里写 `template void Func<int>(const int&);`，这个实例会保留在目标文件里。
- `template` 后面没有 `<>`，也没有函数体，因为函数体由模板生成。
- 然后你能在其他文件使用 `Func<int>`，但仍不能使用 `Func<float>`。

# 类中的模板（Template in class）

- 类里的函数模板也应放在头文件。
- 但若你愿意，也可以在头文件里拆分声明与定义，例如：

函数没有 `<…>`，而类有！

- 不同层次的模板参数不应混在一起。

当然，你也可以用例如 `template class B<void>;`、`template void B<int>::Func(int);` 来显式实例化。

# 小结（Summary）

- 总结如下：
- 函数声明（原型）、类定义与枚举定义应放在头文件。
- 函数定义应放在源文件。
- 成员函数定义可以写在类内，也可以拆到源文件；后者对未来修改更友好，因为修改头文件会传播。
- 有时若不需要成员与布局，类前置声明就够了。
- 模板通常应放在头文件，除非你希望限制模板代码的实例种类。
- 用头文件保护维护 ODR。

# 多文件编程（Programming in multiple files）

命名空间（Namespace）

# 命名空间（Namespace）

- 当代码库很大时，名字冲突很常见。
- 命名空间很像给变量加前缀，用来避免冲突。
- 所以你只需要 `namespace XXX { … }`，把原本在全局的内容放进 `…`。
- 例如：

# 命名空间（Namespace）

- 命名空间也可以嵌套：

- 从 C++17 起，你也可以直接这样嵌套：

- 这与 `namespace Test4 { namespace Test5{ … } }` 完全等价。

# 命名空间（Namespace）

- 注 1：你也可以例如 `namespace Test4 { void Test5::C::Output(){…} };`
- 总之，只要完全限定名相同，就会被视为同一实体。
- 注 2：命名空间内容可以扩展，也就是你可以再写 `namespace Test4::Test5 { class D{…}; }`，于是 `C`/`D` 被视为同一命名空间。
- 注 3：可以用命名空间别名，例如 `namespace Test45 = Test4::Test5;`，它类似于类型别名。
- 正如我们用过的 `namespace stdr = std::ranges;`
- 注 4：可以在 `namespace` 与名字之间加属性，例如：

但嵌套命名空间不支持直接加属性。

如果你学过一点 Python 的基础：`using namespace xx` 类似于 `from xx import *`，

`using yy::xx` 类似于 `from yy import xx`。

- 你也可以 `using namespace xxx`，从而不用写命名空间前缀就能用其中的名字。
- 例如 `using namespace std;`。
- 你也可以只引入部分实体：`using xx;` 例如 `using std::vector` 后可以直接写 `vector<int>`，但 `map` 仍要写 `std::map`。
- 注意：永远不要把 `using namespace xxx` 或 `using xxx` 写进头文件！
- 头文件会被到处包含，于是名字被污染……
- 例如我希望在 socket 库里用 `bind`；如果你在头文件里 `using namespace std`，又包含了 `<functional>`，就会出现 `bind` 与 `std::bind` 的二义性！
- 在源文件里这么做还可以，尽管我个人很少 `using namespace std`。

# 内联命名空间（Inline namespace）

- 最后介绍内联命名空间（inline namespace）。
- 内联命名空间与后面要讲的 `inline` 关键字没有直接关系；只是复用了 `inline` 这个关键字。
- 它会把内容“暴露”到外层命名空间，就好像写了 `using namespace xx;`。
- 例如：

不需要写 `Implv1::`！

# 内联命名空间（Inline namespace）

- 这可用于版本控制。
- 例如，如果 v2 更新了……你可以去掉 `namespace Implv1` 里的 `inline`，再给 `namespace Implv2` 加上 `inline`。
- 这样，只要 API 接口不变，用户代码可以保持不变。
- 否则用户想升级就得把所有 `Test6::Implv1` 改成 `Test6::Implv2`。
- 如果用户真想钉死版本，也可以写 `Test6::Implv2::Func`，这样未来更新不会影响它。
- 若内联命名空间与父命名空间产生同名冲突，会编译报错，这比直接 `using namespace xx` 更安全。

当然，这种版本更新需要重新编译；若不想重编译，只能走共享库路线。但这不是 `inline` 能解决的问题。

# 多文件编程（Programming in multiple files）

内联（Inline）

# `inline`

- 回忆函数调用过程：
- 调用者在栈上保存寄存器；
- 跳转到被调用位置；
- 被调用者保存寄存器、执行代码、恢复寄存器。
- 通过弹出返回地址跳回；
- 调用者在栈上恢复寄存器。
- 如果我们能把函数体融合进调用方并一起优化，那么保存/恢复寄存器与跳转几乎可以消除。
- 很久以前，`inline` 用来提示编译器对函数这么做。

# `inline`

- 为实现这一点，每个源文件都需要能看到函数体，因此：
- `inline` 函数应放在头文件，并且
- 每个 TU 都会为 `inline` 函数生成符号，最后链接器识别并合并为一个，从而不违反 ODR。
- 这与类定义类似：它把函数的 ODR 从“整份程序”放宽到“每个 TU”。
- 然而 `inline` 也有问题：
- 代码体积可能膨胀，因为函数体会被插入到很多地方。
- 指令缓存利用率可能下降，因为同一函数在不同地址出现多份。
- `inline` 函数不会单独形成稳定的“函数入口地址”，因此调试时不容易“单步跳进下一条”。

顺便：对函数取地址得到函数指针，会迫使编译器总是生成一个真实函数体地址。

编译器基本都保留某种“强制”内联的方言，例如 MSVC 的 `__forceinline` 和 GCC 的 `__attribute__((always_inline))`；然而，这只是更强的建议，编译器仍可能拒绝内联。

- 所以 `inline` 只是给编译器的建议，编译器可以选择不把函数体内联进去。
- 递归调用显然不能内联。
- 过于复杂的函数也不会被内联。
- 程序员很难判断该不该用 `inline`。
- 随着编译器优化快速发展，今天的编译器基本不太在乎你写不写 `inline`。
- 即便有些函数定义放在源文件里导致编译期无法内联……
- 链接期优化（Link-Time Optimization，LTO）已广泛实现，使得“链接时再内联”成为可能。
- 于是 `inline` 更多保留的含义是：你只需要在每个 TU 内遵守 ODR，而不像非 `inline` 函数那样必须在整份程序里唯一。程序员：建议内联，当然我只是建议。编译器：我知道，但没办法，宣布吧，我也不是猪，所以你的建议无效。

# `inline`

- 例如：

# `inline`

- 从 C++17 起引入了 `inline` 变量。
- 我们说过全局变量也应拆分声明与定义；
- 所以 `inline` 变量可以直接放进头文件，并且只需要在每个 TU 内满足 ODR。
- 这主要便于 header-only 库，因为它们希望在没有源文件的情况下也能用一些全局变量。
- `inline` 变量也可以出现在类定义里，于是静态变量就不必再拆分声明与定义。
- 因此这要求静态成员具有完整类型。

- 在 C++17 之前，你也许能在类定义里初始化 `static const` 数据成员，但仍需要在源文件里写例如 `const MemberClass A::a`（不带初始化器）。这里不再展开。

# `inline`

- 不过，`inline` 函数/变量在共享库里可能导致“双符号”问题。
- 我们说过每个 TU 都会为 `inline` 函数生成符号，链接时会合并为一个。
- 引入静态库需要完整重链接，于是符号能合并。
- 但对共享库：

在《生命周期与类型安全》里，RTTI 小节会涉及。

# `inline`

- 于是可能出现两个版本的函数/变量；
- 库内的代码调用其中一个……
- 库外的代码调用另一个。
- 这对非 `const` 的 `inline` 变量，或函数里的静态变量，是严重问题，因为不同地方的更新会不一致。
- 例如：Windows 需要额外方言才能把符号导出到共享库。

解决办法：若某些状态可能被修改，只在头文件声明，把定义放到源文件。

# 多文件编程（Programming in multiple files）

链接（Linkage）

# 链接属性（Linkage）

- 我们已经讲了很多拆分声明与定义；
- 但看起来我们总默认一件事——链接器总能在其他 TU 里找到符号！
- 这其实就是外部链接（external linkage）；下面这些东西默认都是外部链接：
- 类成员；
- 函数；
- `friend` 声明的函数；
- 枚举；
- 模板；
- 非 `const` 变量、`volatile` 变量与 `inline` 变量。
- 是的，只要不加额外关键字，几乎所有实体默认都是外部链接。

# 链接属性（Linkage）

- 但有时我们不想把实体暴露出去。
- 例如我们在源文件里实现了一个函数，但头文件里没有声明，意味着只想在当前 TU 使用。
- 但如果有人知道函数原型，他可以在自己的 TU 里声明原型，链接器仍然能找到它！
- 换句话说，链接器会“偷走”我们实现细节的私密性。
- 因此引入 `static` 来强制内部链接（internal linkage）。
- 注意：你仍可以在单个文件里拆分声明与定义；把所有声明放在开头（便于一眼看完所有函数）、实现放在后面，是良好的代码风格。
- 例如：与静态成员函数不同，表示链接的 `static` 在定义处是必要的。

# 链接属性（Linkage）

- 另一种做法是把东西定义在匿名命名空间里。
- 单个 TU 里的所有匿名命名空间被视为同一个；
- 不同 TU 的匿名命名空间则彼此不同。
- 即便你在匿名命名空间里再套一个有名字命名空间，它仍是内部链接。
- 例如：

# 链接属性（Linkage）

- 我们说过模板应放在头文件……
- 否则别人无法直接实例化代码。
- 这与“模板具有外部链接”矛盾吗？
- 不矛盾！例如：

实例化出来的代码具有外部链接。

# 链接属性（Linkage）

- 不过，有些特殊情况实体“天生”就是内部链接：
- `const` 全局变量；
- 匿名联合体（anonymous union）。
- 注意：这两种情况只在 C++ 如此；C 仍把它们视为外部链接。
- 所以若你希望它们具有外部链接，需要显式 `extern`。
- 任何带 `extern` 的实体都具有外部链接；
- 例如 `extern void Func()` 等价于 `void Func()`。
- 我个人若声明不在对应头文件里，会加 `extern` 作为提示：例如在 `a.h` 声明、`a.cpp` 定义，但我希望在 `b.cpp` 引用它却不包含头文件。这个 `extern` 是一种特别标注，表示“定义不在这里”。

# 链接属性（Linkage）

- 例如若你希望暴露 `const` 变量，你需要：
- 在头文件或引用处：`extern const A xx;`。
- 在源文件：`extern const A xx{…};`。
- 这与非 `const` 变量不同……
- 若定义处不显式写 `extern`，它会是内部链接，从而不会被视为头文件里那个外部声明的定义。
- 但 `const` 变量很少需要跨 TU，所以默认内部链接通常问题不大。
- 再提一句……
- 虽然我们教了很多全局变量的链接，但仍不鼓励使用全局变量。
- 这里的“全局”也包括那些包进命名空间里的。

# 单例（Singleton）

- 原因：
- 1. 跨 TU 初始化全局变量的顺序不确定。
- 只在同一 TU 内确定，保证从头到尾（你以前在单文件里用很多全局变量时就是这样）。
- 例如 `a.cpp` 里有一个全局内存分配器 `alloc`；`b.cpp` 里有全局变量 `Person person = alloc.Alloc(…);`。
- 也就是你假设 `alloc` 在 `person` 之前初始化。
- 这并不总是成立；例如若你新增 `c.cpp`，`person` 可能突然最先初始化。
- 但 `alloc` 还什么都没有，就会导致逻辑错误。
- 析构顺序同样不确定。
- 2. 全局变量带来的副作用可能不会执行。
- 例如你有一个类，构造函数会输出 `"Hello"`，并有一个该类型的全局变量 `var`。

# 单例（Singleton）

- 然后你把它做成库。
- 不久以后，你把另一个工程链接到这个库并期待看到 `"Hello"`……
- 结果它居然消失了！
- 这其实不属于 C++ 标准的一部分，而与操作系统和编译器约定有关。编译器可能在符号未被使用时选择不真正链接该库，于是全局变量的副作用不会发生。
- 解决办法：
- 仅在全局变量没有副作用、且不被其他 TU 使用（最好只是内部链接）时使用全局变量。
- 但有时我们确实需要单例模式（单例模式），它真的需要跨 TU。
- 那就只能我们自己手动规定顺序！

# 单例（Singleton）

- 于是我们在 `int main()` 开头手动依次 `GetInstance()`。
- 既然是全局实例，就不必释放内存。
- 程序退出时，操作系统会回收所有资源。
- 若你需要释放，就加一个 `void DestroyInstance() { delete &(GetInstance()); }` 并在结尾手动调用。
- 或者你也可以直接 `static Singleton instance{};`，析构函数会自动调用。
- 析构顺序是逆序；因此你不能在以下情况用它：
- 析构顺序很特殊。
- 或某些全局变量的析构函数仍需要这些单例。

# 链接属性（Linkage）

- 最后一种链接是无链接（no linkage）。
- 这发生在静态局部变量（函数里的 `static` 变量）与普通局部变量。
- 你甚至可以在块作用域里定义类，它也属于无链接（no linkage）。
- 总之，它们只能在当前作用域被引用。

# 多文件编程（Programming in multiple files）

XMake 与如何制作库（XMake & How to make a library）

# XMake

- 大项目会有一堆头文件和源文件……
- 我们可能需要把它们编译成多个可执行文件，
- 或者可能需要打包成库而不是可执行文件，
- 或者你需要指定选项来调试不同部分。
- 这些都需要更强大的构建工具。
- 在 ICS 里你们尝过 Makefile；除此之外……
- CMake
- 它用 DSL 为不同平台生成不同配置，例如在 Linux 生成 Makefile，在 Windows 生成 MSBuild，等等。
- 优点：使用最广，文档充足。
- 缺点：DSL 很丑，直到 2023 才更好地支持 C++20 模块，等等。
- 还有 Scons、premake、autotools、bazel 等……

# XMake

- 通常我们还需要包管理器来引入外部库。
- 在 Python 里你可以 `pip` / `conda install`，比如 numpy。
- 但在 C++ 里，你可能还没用过外部库……
- 我们需要一个能轻松获取包、并可能做版本控制的包管理器。
- C++ 没有官方包管理器，你可以用：
- vcpkg：Windows 上不错，其他平台一般……
- conan（bazel + conan 很流行）
- 系统包管理，例如 Linux 的 apt-get
- XMake 把它们整合在一起；它既是构建工具也是包管理器。

# XMake

- 优点：
- 用 Lua 做配置语言，比 DSL 更好写。
- 由中国人 ruki 开发。
- 因此有中文文档。
- 缺点：
- 文档仍不够全；你可能需要去 GitHub 提问等。
- 若要写高级脚本配置，可能需要一点 Lua 知识（但本课内容不要求你学 Lua）。
- 所以来看看怎么用它！
- 先去 https://xmake.io/#/zh-cn/guide/installation 安装。
- 如果你用 VSCode，可以安装扩展。
- 更多例如 Clion 的扩展见这里。

# XMake

- 特别地，XMake 会把包装在用户主目录；这在 Linux 上通常没问题，但在 Windows 上不一定合适。
- 把软件装在 D: 盘，因为系统盘不会用它……C 盘满了会拖慢系统并缩短 SSD 寿命。
- 此外，有些包来自 GitHub，而访问 GitHub 不稳定，你可以让 xmake 使用镜像站。
- XMake 内部在 `scripts/pac/github_mirror.lua` 提供了一个。

我把它装在 `D:\Work\CS\BuildTools\Xmake\Software`，所以在 `D:\Work\CS\BuildTools\Xmake\Software\scripts\pac\github_mirror.lua`；另外两个路径是随意的，只要放在 D 盘即可。

# XMake

- 你需要在 `xmake.lua` 里配置工程：
- （可选）工程名、工程版本、所需 xmake 版本。
- 非正式工程里可以省略。
- 模式与语言版本。
- 通常我们只需要 debug 与 release 模式；也还有其他模式，例如最小代码体积。
- 语言版本，例如 `cxxlatest`、`cxx20`、`c17`。
- 一些其他选项。
- 我个人会加 `–Wall`（警告与错误一样重要！）。
- `set_policy("build.warning", true)` 表示即使编译成功也报告警告。
- 以及所需包。

# XMake

- 然后需要指定构建目标（target）。
- 例如：
- `target(name)`
- `set_kind(…)`
- `"binary"` 会编译成可执行文件；
- `"static"`——静态库；
- `"shared"`——共享库/动态链接库；
- `"phony"`——空目标，用来组合库等目标；
- `"header_only"`——只有头文件的工程。
- `add_files(…)`
- 添加源文件，支持 `**` 与 `*` 通配符。
- `add_headerfiles(…)`
- 添加头文件；这是可选的，但若希望生成带头文件的 `.sln`，就需要它。

# XMake

- 所以完整工程可能像这样：
- 然后直接 `xmake` 编译所有目标……
- 或 `xmake –b xx` 编译指定目标。
- 或 `xmake run xx` 运行指定可执行文件。
- 你可以在 VSCode 底部栏切换 debug/release 模式：

- 或在 Visual Studio 顶部栏，或其他 IDE 的对应位置。
- 你也可以用 `xmake f -m debug` 手动切到 debug 模式等。

# XMake

- 注 1：对 Visual Studio 与 XCode，你可以用：
- `xmake project –k xcode`。
- `xmake project –k [vs2010|vs2013|…|vs2019|vs2022] –m "debug;release"`。
- 你也可以加 `add_rules("plugin.vsxmake.autoupdate")` 来检测 `xmake.lua` 的更新，从而自动更新 `.sln`。
- 注 2：对 VSCode，为了正确自动补全，需要编辑 `c_cpp_properties.json`。
- `Ctrl + Shift + P` 找到：

- 点它之后，json 会生成在 `.vscode`。

只有指定 `add_headerfiles` 时，这类头文件才会被加进 `.sln`。

每次保存 `xmake.lua`，`compile_commands.json` 会更新；若仍不行，在 `xmake.lua` 里添加 `add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})`，以便构建时更新它。

- 然后再编辑……
- 增加 `"compileCommands"`，vscode 会自动找到语言版本等。
- 这对源文件有用；对头文件仍不够；总之给出了完整代码示例。
- 注 3：可以把 `xmake.lua` 分发到子目录，用 `includes(…)` 一起处理。
- 当工程很大时，你可以把它们拆成多个 `xmake.lua` 让每个文件保持干净。

在根目录写版本等信息，最后这会让它找到 `src/xmake.lua`。

# XMake

- 包管理方面，XMake 集成了 xrepo。
- `xrepo search xx` 搜索包……
- `xrepo install xx` 安装它。
- 有些包例如 cuda 只从系统获取；也就是你需要手动安装，而 xmake 能自动找到。
- `xrepo remove xx` 移除包。
- `xrepo update-repo` 从远端更新包列表。
- 官方包更新很频繁。
- 在 `xmake.lua` 里，`add_requires("xx", "yy")` 用来指定要求用户安装的包。
- 若用户没有，xmake 会提示安装。
- 当某个 target 需要该包时，用 `add_packages("xx")`。
- 若所有 target 都需要，也可以全局 `add_packages`。

# XMake

- 例如：

- XMake 里还有更多内容，本课只会用到其中一小部分。
- 想深入学习，请读完整文档，或知乎上例如 xq114、Pointer 的文章。
- 顺便，xq114 是 XMake 的主要贡献者之一；他也是北京大学物理学院的学生，曾获 2018 年 IPhO 金牌。

# 如何制作库（How to make a library）

- 有了 Xmake，制作静态库很容易。
- 只要 `set_kind("static")`，并且源文件里不要包含 `int main()`。
- 但共享库仍然有点麻烦（无论用哪种构建工具）。
- 库作者可以决定导出哪些符号。
- 在 msvc 里默认不导出符号，而 gcc/clang 默认导出。
- msvc 用方言 `__declspec(xx)` 指定可见性，GCC/Clang 用 `__attribute__((visibility(xx)))`。
- 所以为跨平台，你需要一个 `DLLMacro.h`。

# 如何制作库（How to make a library）

- 例如：

把宏限制在局部生效，否则 `#include` 会影响其他头文件……

# 如何制作库（How to make a library）

- 对作为库接口的头文件，这样写：
- 对共享库，我们需要导出符号，因此不妨 `#define FUNC_H_EXPORT_`。

- 对用户，我们需要导入符号，因此不妨 `#define FUNC_H_IMPORT_`。

不要泄漏给包含者（includers）。

# 如何制作库（How to make a library）

- 注意：我们不直接 `add_defines("DLL_MACRO_NEED_EXPORT")`，因为你的共享库可能还会加载其他共享库，于是那里需要 `DLL_MACRO_NEED_IMPORT`。但若能保证不会这样，也可以。
- 现在我们 `xmake`，库就做好了，可执行文件会链接到库上。
- 恭喜！
- 由于共享库是惰性加载的，你可以 `xmake –b xx` 只重建库，而 `xmake run yy` 仍能拿到最新代码。
- 我给你看个例子……

# 如何制作库（How to make a library）

- 不过你需要保证两件事：
- 之前的符号仍能找到最新的那些；也就是命名空间、类名、函数名以及函数参数类型不应改变。
- 还记得吗？我们的第一次复习课……

# 如何制作库（How to make a library）

- 暴露给用户的类的布局不能变。
- 例如你最初定义 `class A{ int a; int b; };`，但库更新后改成 `class A{ int b; int a; };`。
- 那么程序仍能运行，因为它仍能找到 `class A` 的符号；但结果是错的……
- 对用户来说，他知道的是 `class A{ int a; int b; };`，在二进制层面意味着两个偏移，例如 `sp + offset_a` 与 `sp + offset_b`。
- 假设 `offset_a` 是 0，`offset_b` 是 4。
- 对库来说，当它使用 `class A{ int b; int a; };` 时，`var.b` 会走到偏移 0，`var.a` 走到偏移 4……
- 所以现在用户侧的 `var.b` 实际会读到 `var.a`，反之亦然；
- 假设你有一个成员函数 `int GetSub(){ return b – a; }`
- 更新前你得到 1；更新后你会得到 -1……

因此尽管返回类型不参与名字改编（name mangling），改变返回类型仍可能破坏函数调用（用户以为是 `A`，库却给出 `B`）。

# 如何制作库（How to make a library）

- 另一件与布局相关的是虚函数。
- 你不应改变虚函数顺序，否则旧 vtable 会指到错误函数。
- 也不应轻易在基类里新增虚函数。
- 但你可以扩大虚表：也就是在末尾添加虚函数；用户不能调用它们，但原有方法仍正确。
- ABI 兼容性用于维护共享库的正确性。
- 著名例子是 GCC 5.1：它改变了 `std::string` 的定义，导致许多旧库“无法链接”。
- 更糟的是，错误往往在运行时才悄悄发生。
- 这也是为什么有些很保守的公司不用新 g++，从而把 C++ 版本钉在 C++11……

需要保持 ABI 兼容时，可查 KDE policy 与这篇文章了解更多。我们以后也会讲一种叫 PImpl 的技术来帮忙。

# 语言链接（Language linkage）

- 有时你需要链接其他语言的库，这就要求你从符号加载。
- 但名字改编因编译器而异，很难跨平台。
- 解决办法：用 C！
- C 没有函数重载、模板、命名空间等，因此可以直接用函数名作为符号。
- 例如 `void Func()` 在 msvc/C++ 里可能被改编成 `?Func@@YAXXZ`，但在 C 里就是 `Func`。
- 但 C++ 有很多 C 没有的特性，这会把接口限制在一小集合里。

# 语言链接（Language linkage）

限制：
- 例如：
1. 所有命名空间都会被忽略。

`namespace A{ int a; }` 与全局 `int a` 引用同一符号。

2. 函数重载会造成符号冲突。

3. `extern "C"` 对类、模板、非静态类成员函数没有作用；但对类成员函数的参数类型仍可能有影响。

# 语言链接（Language linkage）

- 一句话：能用 C 表示的符号会受 `extern "C"` 影响；否则仍保持 C++ 的原始符号。

- 注 1：`extern "C"` 实体具有外部链接。
- 注 2：`extern "C"` 不代表你不能用任何 C++ 特性；你可以在源文件里包装 C++ 特性，用户只需要在头文件里识别符号。
- 注 3：严格说，`extern "C"` 表示 C 链接器能链接它；若有 `extern "Java"`，就表示 Java 能用它。
- 但大多数编译器只支持 `extern "C"`。

# 语言链接（Language linkage）

- 例如，若我们希望在 C 里使用 C++ 库。
- 我们不希望手动加载符号，而是直接用 C 链接器。
- 靠头文件！
- 在编译 C 的流程里，头文件声明会编译成 C 符号。
- 链接 C 程序时，就能用该符号找到我们的库。
- 两种方式：
- 1. 为 C 新建一个头文件，不带源文件。
- 于是你的 C++ 接口头文件可以包含 C++ 特性、`#include` C++ 库等。
- 在你的 C 头文件里，只写那些在 C++ 头文件里 `extern "C"` 的声明。
- 例如：与 `extern "C" { int a; void Func(); }` 相同

# 语言链接（Language linkage）

- 对给 C 用的头文件，只写你需要链接的内容，例如：

注意有些人更喜欢用 `.hpp` 作为 C++ 头文件后缀；你喜欢就用。

事实上源文件可以叫 `.cxx`、`.cc`、`.cpp`，头文件可以是 `.h`、`.hh`、`.hpp`。

# 语言链接（Language linkage）

- 2. C 与 C++ 共享头文件。
- 用 `__cplusplus` 判断是否按 C++ 编译。
- 这不方便使用 C++ 特性。

- 啊，你也可以把 C++ 库链接到其他语言，例如 C#、Go 等；但那取决于各语言规范。
- C 约定基本被所有语言接受。
- 必要时查它们的手册。

# 多文件编程（Programming in multiple files）

模块（Modules）

# 模块（Modules）

- 头文件有几个问题：
- 非 `inline` 函数不能定义，否则会破坏 ODR。
- `#include` 总要预处理器复制全部内容，导致“真实文件”巨大并拖慢编译。
- 有些编译器支持 PCH（预编译头），但限制很多很难完全匹配。
- 若宏没有 `#undef`，会泄漏。（例如 minmax）
- 模块消除了这些问题！
- 所有实体都可以在“接口文件”里定义，而不用担心 ODR。
- 模块总是“预编译”的，因此未来 `import` 会很快。
- 宏不会影响其他模块。

模块接口文件没有固定后缀；msvc 用 `.ixx`；clang 用 `.cppm`；gcc 不太在乎。也可以是 `.mpp`、`.mxx`。

# 模块（Modules）

这里我们用 `.mpp`，xmake 会识别 `.mpp`、`.mxx`、`.cppm`、`.ixx`。
- 每个模块只有一个主接口单元（primary interface unit），类似头文件。
- 以 `export module Name;` 开头。
- 它通过 `export` 规定哪些实体对其他模块可见。

`Person.mpp`

整个函数定义可以放在这里。

未 `export` 的，从其他模块不可见。

# 模块（Modules）

- 具有外部链接但未 `export` 的实体具有模块链接（module linkage）。
- 例如模块 A 里的 `class Person` 与模块 B 里的 `class Person` 是不同实体（也就是不同类型）。
- 显式内部链接的实体（即 `static` 或匿名命名空间）不能导出。
- 天生内部链接的实体（即 `const` 变量）若被导出，则会获得外部链接。
- 若模块接口 `import` 了另一个模块并希望让别人也看到，可以用 `export import`。
- 导出它所导入的模块！`main.cpp`

# 模块（Modules）

`Customer.mpp` `main.cpp`

# 模块（Modules）

- 但就像头文件一样，也允许拆分声明与定义。
- 小接口更便于维护者与用户理解；很多非导出方法应当隐藏。
- 也可能让你在只改实现不改接口时编译更快。
- 所以模块也可以有模块实现单元（module implementation unit）。
- 它很像 C++20 之前的源文件。
- 它以 `module Name;` 开头，且不应出现任何 `export`。
- 由接口负责规定导出与否；实现只提供定义。
- 一个模块可以有多个实现文件，只要它们都以 `module Name;` 开头。

与头文件类似，模板仍应定义在接口文件里，否则链接会失败。

# 模块（Modules）

完全不能 `export`。

`Person.mpp` `Person.cpp`

# 模块（Modules）

- 但很多老库按“头文件 + 源文件”组织，模块如何协作？
- 你可以直接 `import "Header.h";`，它们叫头文件单元（header unit）。

`Person.cpp` `Old.h`

`Old.cpp`
- 我个人为了让工程结构干净，喜欢把旧代码归组并编译成静态库，于是 `.cpp` 总是模块实现。

# 模块（Modules）

- 有时头文件一部分由宏控制。
- 例如在 glm 里：
- 你可以用宏禁用默认构造函数。
- 但正如我们所说，宏只影响当前文件，因此导入的头文件单元看不到那个宏。
- C++ 用全局模块片段（global module fragment）来解决这个历史问题。
- 它写在 `export module Name;` 或 `module Name;` 之前。

# 模块（Modules）

- 我们也能在 glm 的模块里看到它：

- 以上内容基本被所有编译器支持（2024.2）。
- 尽管 gcc 仍不支持直接 import 标准库的头文件单元，且 gcc 与（搭配 MS-STL 的 clang）也尚未支持 `import std;`。
- 搭配 libc++ 的 Clang 已经支持。
- 但后面这些内容支持仍不好，所以只应在玩具项目里用。

# 模块（Modules）

- 若仍觉得模块太大，可以对接口或实现做分区（partition）。
- 接口分区单元：以 `export module Name:Subname;` 开头。
- 实现分区单元：以 `module Name:Subname2;` 开头。
- 注意模块名里可以有圆点（例如 `A.B`），但它没有语义含义，只是逻辑含义。
- 我们把它当作 A 的“子部分”，但原则上它们仍是不同模块。

- 1. 分区是模块的内部概念；其他模块无法知道它是否有分区。
- 在模块内部，可以用 `import :SubName` 导入分区。
- 但在其他模块里，不能用 `import Name:SubName`。

# 模块（Modules）

- 2. 与模块实现不同，实现分区并不是接口分区的实现。
- 也就是说，若存在 `module A:B;`，就不应再存在 `export module A:B`。
- 若你想拆分接口分区里的定义，请在 `module A` 或另一个分区 `module A:C` 里做。
- 3. 分区不能再有分区（深度 == 1）。

`Person-Order.mpp` 一个实现分区

注意：实现分区不能被

导出，只在模块内部可见。

主接口可以选择导出接口分区。

`Person-Utils.mpp` 一个接口分区

`Person.mpp` 主接口

`main.cpp`

# 模块（Modules）

- 私有模块片段（private module fragment）：
- 以 `module:private;` 开头。
- 当某部分（尤其是类定义）放进私有模块片段后，其他模块无法触及。
- 当模块拥有私有模块片段时，它只能有单个单元；也就是主接口单元。

- 我个人觉得它不太有用……

# 模块（Modules）

`main.cpp`

`Person.mpp`

# 小结（Summary）

- 预处理与宏•内联的坑！
- 翻译单元与 ODR•模块
- 声明与定义•模块接口与实现
- 头文件与源文件
- 头文件单元、全局模块
- 特殊情况——模板片段
- 头文件保护
- 分区、私有模块片段
- 内联函数与变量
- 命名空间
- 链接
- Xmake
- 静态库与共享库

# 下一讲……

- 我们会讲 C++ 的错误处理。
- 呃……你会知道异常是什么（终于，在大量“以后讲”之后）。
- 你还会学到一些其他辅助手段，例如断言、stacktrace、expected 等……
- 最后是单元测试（Unit Test）！测试是专业程序员最重要的技能之一。
