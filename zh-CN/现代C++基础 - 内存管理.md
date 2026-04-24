内存管理

## 现代C++基础

## 现代 C++ 基础篇

梁佳明，北京大学本科生

### • 底层内存管理

### • 智能指针

### • 分配器

- PMR

- 内存的真实结构相当复杂……

致谢：北京大学体系结构课程 张杰

- 不过，操作系统已通过页表将其抽象为虚拟内存，因此在大多数情况下，用户可以把内存看作一大段连续数组。
- 当这种抽象成为性能瓶颈时，程序员需要进一步深入；
- C++ 也提供了一些工具来解决若干常见问题。

# 内存管理

底层内存管理

# 内存管理

- 底层内存管理
- 对象布局
- 详解 `operator new` / `delete`

# 对象布局

- 对象会占据一段连续的内存，这段内存：
- 起始于满足某种对齐要求的某个地址；
- 结束于满足某种大小要求的某个地址。
- 一个完整对象可能包含许多作为成员或元素（例如数组或类）的子对象。
- `sizeof` 反映的是该类型在构成完整对象时的大小，且恒大于 0。
- 例如：
- 在大多数情况下，子对象只是以相同方式占用内存：

# 对象布局

- 然而，某些作为类成员的子对象可以有 0 大小……
- 正式名称为“可能重叠的对象”（potentially-overlapping objects）。
1. 对于一个类，若满足：
- 无非静态数据成员；
- 无虚函数或虚基类；
- 它是基类。则允许其大小为 0。

此外，若

- 派生类是标准布局类（standard-layout），则强制其大小为 0。
- 也称为“空基类优化”（Empty Base (Class) Optimization，EBO/EBCO）。

# 对象布局

- 于是我们现在可以理解 `static_cast` / `reinterpret_cast`……

见第 5 讲《生命周期与类型安全》。

供参考：自 C++20 起，可用 `std::is_pointer_interconvertible_with_class` 与 `std::is_pointer_interconvertible_base_of` 进行检查。

# 对象布局

- 空基类会被折叠，因此转换是安全的。

: 严格来说，应当是“相似类型”，例如允许添加 cv 限定符。详见 [conv.qual]。

2: 除了像字符串字面量这类可能非唯一（potentially non-unique）的对象。

2. 自 C++20 起，对于带有属性 `[[no_unique_address]]` 的成员子对象，允许其大小为 0。
- 特别地，为保持向后兼容，msvc 会忽略该属性；取而代之的是 `[[msvc::no_unique_address]]`。
- 例如：在 gcc/msvc/clang 中，`sizeof(Y) == 4`。

- 注意：C++ 规定同类型的两个对象必须有不同地址。三种编译器都会使
- 例如：`sizeof(Y) == 2`。

# 对象布局

- 理论上可优化为 `sizeof(W) == 2`；然而三种编译器均使 `sizeof(W) == 3`。
- 于是我们又能理解标准布局……

# 对象布局

- 现在 EBCO 并不保证一定发生：

- 在 ABI 中，基类可能被放在最前；
- 作为基类的对象必须与第一个成员区分开来，于是基类未必真的“空”。*
- 而非空基类会导致非标准布局。

*：当前定义中可能存在一些缺陷。见 StackOverflow 上的问题。

# 布局兼容（Layout Compatible）*

- 本部分为可选内容。
- 最后我们修正此前的说法：

- 严格地说，当类型具有共同初始序列（common initial sequence）时，在对象生命周期之外进行访问是合法的：

# 布局兼容（Layout Compatible）*

- 正式地，若满足以下条件，则称两个类型布局兼容（layout compatible）：
- 简单情形：
- 二者为同一类型（忽略 cv 限定符）；或
- 二者为具有相同底层整数类型的枚举。
- 否则，
1. 二者均为标准布局；且
2. 它们的共同初始序列覆盖所有成员。
- 其中共同初始序列指：按声明顺序，非静态数据成员与位域的最长序列，满足
1. 对应实体布局兼容；且
2. 对应实体具有相同的对齐要求；且
3. 要么二者均为宽度相同的位域，要么二者都不是位域。

# 布局兼容（Layout Compatible）*

- 例如：A 与 B 布局兼容。

*
- 自 C++20 起，可用 `std::is_layout_compatible` 与 `std::is_corresponding_member` 进行检查。

*：严格来说，`std::is_layout_compatible` 会容忍非结构体类型，而标准仅规范结构体类型。

# 对齐（Alignment）

- 为最大化效率，数据应被正确对齐。
- 例如，在某些平台上：

- 在 C++ 中，可用 `alignof(T)` 检查；
- 与平台相关，返回 `std::size_t`，与 `sizeof` 颇为相似。

*或使用类型特征 `std::alignment_of`。

# 对齐（Alignment）

- 在类中封装数据时，每个对象都会按自身对齐要求对齐，从而产生填充（padding）。
- 例如：

C 数组中的每个元素都应被适当对齐，因此 `sizeof(X)` 必须是 `alignof(X)` 的倍数。

# 对齐（Alignment）

- 自然，所有标量类型的对齐不会超过 `alignof(std::max_align_t)`（在 `<cstddef>` 中）。
- 且分配在默认情况下会对齐到该对齐。
- 不过，有时你可能需要过度对齐（over-aligned）的数据。
- 此时可使用 `alignas(N)` 使对齐为 N。
- 当 N == 0 时忽略；若 N 不是 2 的幂则编译错误。
- 例如，以匹配 OpenGL uniform 布局：

这三个成员都对齐到 16。

# 对齐（Alignment）

- 注 1：也可使用 `alignas(T)` 获得与 T 相同的对齐。
- 注 2：使用多个 `alignas` 时，将选取其中最大的一个。
- 于是此前的代码段可改写为：

- 注 3：可在 `alignas` 中进行包展开，等价于 `alignas(arg1) alignas(arg2) … alignas(argN)`。
- 即在 N 个实参中选取最大对齐。

# 对齐（Alignment）

- 注 4：仅可过度对齐：若 `alignas` 弱于其天然对齐（即未写 `alignas` 时的对齐），则编译错误。
- 某些编译器会忽略或仅警告。
- 注 5：对齐不是类型的一部分，因此不能在 `using` 或 `typedef` 中对其起别名。属性写在 `struct` 之后。

- 注 6：函数形参与异常形参不允许使用 `alignas`。

# 伪共享（False Sharing）

- 实践示例：伪共享
- 从抽象上看，不同线程操作不同数据时，由于无需加锁，并行度会最大化。

- 然而，受计算机体系结构限制，这种抽象并不成立……
- 不同处理器上的缓存必须遵守诸如 MESI 的一致性协议。
- 简而言之，当某缓存行发生写操作时，会通知其他也持有该行的处理器使其失效。
- 失效行需要重新加载，从而导致低效。

# 伪共享（False Sharing）

处理器 1 的缓存行。处理器 2 的缓存行。

… 内存

arr

伪共享的示意动画。（不同体系结构细节可能不同）

# 伪共享（False Sharing）

`arr[0]++`

…

arr

# 伪共享（False Sharing）

失效（Invalid）

…

arr

# 伪共享（False Sharing）

`arr[1]++`

…

arr

# 伪共享（False Sharing）

从其他处理器加载以读取

…

arr

# 伪共享（False Sharing）

执行写入

…

arr

# 伪共享（False Sharing）

失效（Invalid）

…

arr

# 伪共享（False Sharing）

- 因此，当不同线程的写入落在同一缓存行上时，每次写入都会独占发生，仿佛持有一把锁。
- 这造成“伪并行”，性能下降。
- 解决办法：让线程访问位于不同缓存行上的数据！
- C++17 在 `<new>` 中提供常量 `std::hardware_destructive_interference_size`。
- 例如：

# 伪共享（False Sharing）

- 另一方面，对单线程而言，我们希望被访问的数据落在同一缓存行上以尽量减少污染。
- 例如：

对齐不当，占用两条缓存行。

对齐得当，占用单条缓存行。

- 为强制数据落在同一缓存行，可将头部对齐到缓存行起点。
- C++17 因此引入 `std::hardware_constructive_interference_size`。

# 伪共享（False Sharing）

- 例如：

- 问题：`std::hardware_destructive_interference_size` 与 `std::hardware_constructive_interference_size` 不就等同于缓存行大小吗？
- 为何需要两个常量来表示？

# 伪共享（False Sharing）

- 原因：在某些体系结构上，破坏性干扰（destructive interference）范围会大于一条缓存行……
- 例如，在 Intel Sandy Bridge 处理器上会进行相邻行预取（adjacent-line prefetching）。
- 因此加载一条缓存行时，下一条缓存行可能被替换也可能不会，从而导致 `hardware_destructive_interference_size == 128` 而 `hardware_constructive_interference_size == 64`。

# 补充说明（Supplementary）

- 注 1：`<memory>` 中还存在若干对齐相关工具。
1. `std::align`：

- 假设有一段从 `ptr` 开始、大小为 `space` 的空间；
- 现在希望在该空间上分配一个具有给定 `size` 与对齐要求的对象；
- 假设可以分配到 `new_ptr`，剩余空间为 `new_space`（即已适当对齐）。
- 则 `std::align` 会把 `ptr` 修改为 `new_ptr`，`space` 修改为 `new_space`，并返回 `new_ptr`。
- 若空间过小，则什么也不做并返回 `nullptr`。

# 补充说明（Supplementary）

- 例如：

# 补充说明（Supplementary）

2. 为最大化优化，自 C++20 起可用 `std::assume_aligned<N>(ptr)` 告知编译器指针按 N 对齐。
- 若实际未对齐到 N，则为未定义行为（UB），与 `[[assume]]` 颇为相似。
- 自 C++26 起还可使用 `std::is_sufficiently_aligned<N>(ptr)` 在调试模式下检查前置条件。
- 例如：

# 补充说明（Supplementary）

- 注 2：自 C++17 起，可用特征 `std::has_unique_object_representations` 检查：两个对象的值表示相同是否必然导致对象表示相同。
- 例如，对 `float` 而言，两个 NaN 不可区分但比特可能不同，因此该特征返回 false。
- 特别地，对结构体而言，若存在填充字节，则必然返回 false，因为它们不属于结构体的值的一部分。

- 该特征可用于检查某类型按字节数组做哈希是否合适。

# 内存管理

- 底层内存管理
- 对象布局
- 详解 `operator new` / `delete`

# `new` / `delete`

- 为将分配与构造结合，C++ 使用 `new` 表达式以替代 C 中的 `malloc`。
- 粗略地说，它调用两类不同函数：
- 分配用的 `new`，仅分配内存（很像 `malloc`）。
- 定位 `new`（placement new），即在内存上构造对象。
- 类似地，`delete` 表达式有两部分：
- 析构函数，即在内存上析构对象。
- 释放用的 `delete`，仅释放内存（很像 `free`）。
- C++ 允许通过 `operator new` 覆盖（替换）分配 `new`，通过 `operator delete` 覆盖释放 `delete`。

# `new` / `delete`

- 于是最基础的类似 `malloc` / `free` 的版本如下：
- 可在全局作用域（即命名空间 `::`）中覆盖它们。

- 此外，可提供类专属的分配与释放：

- 这比全局覆盖更受偏好，且不必为 `noexcept`。
- 它们始终是静态函数，即便你不写 `static` 关键字。

注意：在 P3107（DR23）之前，`std::println` 会使用 `std::string` 从而可能需要调用 `operator new`/`delete`，导致无限递归。MS-STL 已实现该 DR，因此这样做没有问题。

此处我们不写 `static`，但它仍是静态函数。此处不能使用 `this`。

# `new` / `delete`

- 然而，C++ 也允许你 `delete basePtr`，这会调用虚析构。
- 理想情况下，应调用 `Derived::operator delete` 而非 `Base::operator delete`……
- 来试试看！

# `new` / `delete`

- 输出类似：
- 于是调用了 `Derived::operator delete`，很像虚析构！
- 但 `operator delete` 是静态的！如何做到？*
- 原因：编译器会生成一个“删除析构函数”（deleting destructor）。
- 也就是说，它会生成一个新的虚函数：
- 对普通对象，仍用普通析构；
- 对 `delete ptr`，会调用这一新函数。

- 借助虚分派，可从类型中提取更多信息以改进类似 `malloc` 的版本！

*：这是实现定义的；此处采用 Itanium ABI 的做法。详见 [这篇博客](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#deleting-destructor)。

# `new` / `delete`

- 在继续之前，先稍作回顾……
- 在 ICS 中，我们写过一种非常基础的分配策略：
- 分配比请求略大的内存块，并在其旁存储块大小与指向下一块的指针。
- 然而，许多元数据在分配后永不改变，会污染缓存行。
- 因此在现代内存分配器中，情况复杂得多……
- 粗略地说，常见做法是把内存按近似大小分成若干桶（bin）索引。
- 而元数据可能记录更多信息：

摘自 mimalloc。

# 带大小的 `delete`（Sized-delete）

- 而在 C++ 中，我们几乎总能确切知道对象类型……
- 于是可以知道对象大小！
- 为便于优化，C++ 引入带大小感知的 `delete`（也称 sized-delete）。
- 全局 sized delete 在 C++14 提供，类专属版本自 C++11 起。

- 举一个简单的例子：

# 带大小的 `delete`（Sized-delete）

- 注 1：一些实际例子：
- 例如在 jemalloc 中：

- 注 2：编译器可自由选择 sized-delete 或普通 delete。
- 因此，程序员应始终同时提供二者。
- 对 gcc/msvc/clang（clang 需要 `--fsized-deallocation` 标志）：
- 对全局覆盖，若存在 sized 版本则优先使用。
- 对类专属覆盖，若存在普通 delete 则优先使用（因为可用 `sizeof` 轻易得知大小）。

# 对齐的 `new` / `delete`（Aligned new/delete）

- 但这些重载并未指定对齐……
- 在 C++17 之前，过度对齐的类型可能无法被正确处理（例如 `-Wall` 下通常会有编译器警告）。

- 自 C++17 起，引入了对齐感知的 `new`/`delete`。
- 此处 `std::align_val_t` 是作为标签的作用域枚举。
- 对于对齐要求超过宏 `__STDCPP_DEFAULT_NEW_ALIGNMENT__` 的类型，优先使用对齐感知重载。
- 当然，你也可以覆盖它们。

- 对类专属版本：

C11/C++17 类似地提供 `aligned_alloc`；但 MS-STL 不提供 `aligned_alloc`，因为 Windows 不具备直接分配对齐内存的能力，因此必须过度分配再手动对齐。故不能用 `free` 正确释放；应改用 `_aligned_alloc` 与 `_aligned_free`。

# `new` / `delete`

- 注 1：所有 `new` 重载都有 nothrow 变体：

- 注 2：本质上，`new` 表达式 `new(args…) Type{…}` 会调用 `operator new(size(, align), args…)`。
- `args…` 之前的实参通常由编译器决定，后者由用户指定。

*：在 `new`/`delete` 语境中“placement”一词被滥用；此处仅指在 `new(…)` 中提供额外参数，

# `new` / `delete`

包括 nothrow 变体与 placement-new 变体。

- 因此你可以使用：
- `new(std::nothrow) Type`，因为存在 nothrow 变体。
- `new(ptr) Type`，因为存在 placement-new 变体。
- 但它们不能被用户覆盖。

- 更一般地，可为用户自定义的 placement 分配 `new` 提供自定义实参：
- 类专属版本也存在，此处从略。

- 另有 placement 释放 `delete`：
- 每个用户自定义的 `new` 必须有匹配的用户自定义 `delete`；当构造函数抛出时，新分配的内存会由对应 `delete` 释放。
- 否则内存泄漏！例如（省略 sized-delete）：

只有失败的 `new` 表达式才会调用对应的 placement `delete`！防止内存泄漏。

# `new` / `delete`

- 类似地，对 nothrow `new`，你需要自定义 placement `delete`……

- 最后，若某个 placement 分配对应的是非 placement 释放，则编译错误。

这是 sized delete。

# `new` / `delete`

- 注 3：对默认会抛出的 `operator new`，分配失败时会调用 new handler。
- 如同：

默认 new handler 就是 `nullptr`，因此会直接抛出 `std::bad_alloc`。

# `new` / `delete`

- 可在 `<new>` 中用 `std::set_new_handler(…)` 自定义（线程安全），handler 预期应：
1. 使更多内存可用（调用 handler 后分配重试可能成功）；
2. 终止程序（例如通过 `std::terminate`）；
3. 抛出派生自 `std::bad_alloc` 的异常，或 `std::set_new_handler(nullptr)`。
- 返回值：先前的 handler。
- 例如：

# `new` / `delete`

- 注 4：C++20 引入类专属的 destroying-delete。

- 它会优先于所有其他重载。
- `delete` 表达式会直接调用 destroying-delete，而不调用析构函数。
- 也就是说，调用析构是 destroying-delete 的职责。
- 数组没有该重载。
- 注 5：调用 `operator new`/`delete` 应当是线程安全的。

# 协程中的 `new` / `delete`

- 特殊示例：控制协程的分配。
- 协程会通过 `new` 分配其状态/帧；
- C++ 允许你自定义 `promise_type` 的 `operator new`/`delete` 来控制此类分配！
- 它被特殊处理，因此与普通的类专属分配/释放并不完全相同。
- 类专属版本需要大量重载以覆盖所有可能情形；
- 但 `promise_type` 只需定义少数几个供编译器选择！

- 对 `new`，只需：
- 对 `delete`，只需：
- 当该重载不存在时，需要：

# 协程中的 `new` / `delete`

内存资源（memory resource）将在后续章节讨论。
- 例如：

# 协程中的 `new` / `delete`

- 注 1：定义 `get_return_object_on_allocation_failure` 时，应使 `operator new` 表现得如同 nothrow，而不是定义 nothrow 变体。
- 例如：

- 注 2：编译器在执行 HALO 时允许省略你的 `operator new`/`delete`。
- 因此理论上，确保 HALO 的一种方法是只声明 `operator new`/`delete` 而不定义，从而在堆上分配会导致链接错误。
- 注 3：`operator new` 允许接受协程相关参数。
- 简单示例：
- 若存在则优先使用。

# 协程中的 `new` / `delete`

- 以 `std::generator` 为例：

对成员协程。
- 实现可能分配比 `size` 更多的字节，再把分配器放在额外空间中。
- `delete` 可从帧中提取分配器以完成释放。

# 协程中的 `new` / `delete`

- 通过传入额外参数使用。

# `new` / `delete`

- 最后说明：在共享库中，对全局 `operator new`/`delete` 的覆盖需格外谨慎。
- 原因：若每个共享库都有自己的覆盖，可能不清楚实际调用的是哪一个。
- 例如，加载 A 时，其内存由 A 的 `operator new` 分配；
- 再加载 B，则 `operator delete` 被替换；
- 当 A 释放其内存时，使用的是 B 的 `operator delete`，导致结果未知。
- 行为完全由实现定义。
- 在静态库中，会导致符号冲突的链接错误。

# 内存管理

智能指针

# 概述

- 与每个 RAII 类型类似，智能指针可在析构函数中释放资源，从而防止内存泄漏。

见第 7 讲《错误处理》中“异常安全”一节。

- 一般而言，智能指针表达某种“所有权”。
- `std::unique_ptr` 表示独占所有权；只有一个能销毁它。
- `std::shared_ptr` 表示共享所有权；最后一个持有者销毁它。
- ……
- 因此当某人不需要所有权时，使用裸指针即可。不要滥用智能指针。
- 我们稍后会进一步讨论。

# 内存管理

- 智能指针
- `unique_ptr`
- 间接与多态（C++26）
- `shared_ptr`
- `weak_ptr`（均定义在 `<memory>` 中）
- 适配器

# `unique_ptr`

- 因较简单且已简要讲过，先列出 API 并补充若干重要说明。仅可移动，即有移动构造与赋值，无拷贝构造与赋值。

放弃所有权并置为 `nullptr`；返回原指针。销毁原资源；用参数 `ptr` 替换（默认为 `nullptr`）。

# `unique_ptr`

- 注 1：我们知道 `unique_ptr` 也可通过指定 `T[]` 处理数组。
- 该偏特化略有不同：
1. 没有 `operator->`/`*`，而有 `operator[]`，如同访问数组。
2. 当然，默认会调用 `delete[]`。
- 这也导致无法做 CTAD：给定指针，无法判定是 `unique_ptr<T>` 还是 `unique_ptr<T[]>`。
- 注 2：若要表示 `const T*`（即指向不可变对象），应使用 `unique_ptr<const T>` 而非 `const unique_ptr<T>`。

# `unique_ptr`

- 注 3：更一般地，`unique_ptr` 可通过自定义删除器管理任意资源。
- 删除器需定义：
1. 名为 `pointer` 的类型（若不存在则使用 `T*`）；
- 它会被存储在 `unique_ptr` 内部管理。
2. `operator()` 执行销毁操作（例如 `std::default_delete<T>` 中的 `delete`，以及特化 `std::default_delete<T[]>` 中的 `delete[]`）。
- 例如：

此 `unique_ptr` 管理内存。

此 `unique_ptr` 管理 OpenGL 描述符。移除与描述符缓冲区相关的 GPU 资源。

# `unique_ptr`

- 再来一例？

1. `unique_ptr` 现在存储 `unsigned int` 而非指针；
2. 析构时会调用 `operator()`。

- 但它无法使用某些方法（如 `.release()`），因为会尝试将 `nullptr` 赋值为“空资源”……
- 若要完全兼容，应使 `pointer` 满足 NullablePointer。
- 若需使用 `unique_ptr` 的 `operator*`/`->`，还需额外支持 `operator*`/`->`。

# `unique_ptr`*

- 例如：相当复杂……

若只需一般 RAII 包装，可自行编写，而不必使用带自定义删除器的 `std::unique_ptr`。此时 `std::unique_ptr` 的所有方法都有定义（尤其当 `pointer` 是某自定义类时）。我们不再深入，直接查阅 cppreference 即可。

# `unique_ptr`

- 注 4：析构函数会先检查是否为空。
- 因此若删除器无法正确处理 `nullptr`，也无妨。

- 注 5：也可用 `std::make_unique<T>(Args…)` 进行构造。

用 `()` 初始化而非 `{}`
- 对数组，只能指定大小，所有元素均做值初始化。
- 例如此处所有元素为 0。

# `unique_ptr`

- 在 C++17 之前，`std::make_unique` 可防止由不确定求值顺序引起的微妙内存泄漏。
- 例如，顺序可能是 `new int{1}` -> `new int{2}` -> 构造 `unique_ptr`；
- 于是当 `new int{2}` 抛出时，仍可能发生内存泄漏。
- 自 C++17 起，我们知道函数实参以非重叠方式求值，因此该问题不再出现。
- 有时也可能不需要值初始化……
- 例如我们从网络读取二进制数据，无需把所有元素置 0。
- 此时自 C++20 起可使用 `std::make_unique_for_overwrite`。
- 本质区别只是 `new int()` 对比 `new int`。

# `unique_ptr`

- 回到此前的论断……
- “当某人不需要所有权时，使用裸指针即可。不要滥用智能指针。”

- 以函数参数为例……
- 裸指针（`T*`）
- `std::unique_ptr<T>`
- `std::unique_ptr<T>&`
- `std::unique_ptr<T>&&`
- `const std::unique_ptr<T>&` 该选哪一个？

# `unique_ptr`

1. 多数情况下，裸指针足够……
- 前置条件：除 `nullptr` 外，所指对象有效。
- 函数通过指针读/写对象。
- 相比之下，应很少通过 `delete` 等方式操纵生命周期。
- 观察而非所有权。
- 例如：

- 该函数不关心 `ptr` 来自何处（栈、堆或静态区等）；它仅观察。

# `unique_ptr`

2. 相比之下，`std::unique_ptr<T>` 表示持有所有权；
- 于是调用方会放弃其所有权。
- 函数可能将所有权转移给他人，或仅在退出函数时自动销毁。

`std::unique_ptr<T>&&` 颇为
- 例如：相似，区别在于若函数内不移动，则调用方不会释放所有权。

若以值传递，则所有权必然被释放。

# `unique_ptr`

3. 对 `std::unique_ptr<T>&`……
- 一般地，对引用参数 `U&`，我们意图是修改参数本身。
- 因此类似地，`std::unique_ptr<T>&` 表示修改调用方的 `unique_ptr`。
- 例如，设置新的对象所有权：

- 当然，它可以读/写内容，并将所有权转移给他人；
- 但若仅需承担这些职责，则不必使用 `&`。
- 这很像 `T*` 对比 `T**`。
4. 最后，对 `const std::unique_ptr<T>&`，由于其只读特性与 `T*` 相同，这种形式没有用处。

# 内存管理

- 智能指针
- `unique_ptr`
- 间接与多态（C++26）
- `shared_ptr`
- `weak_ptr`
- 适配器

# PImpl

- 在继续之前，先介绍一种称为 pointer to implementation idiom（pimpl）的技术。
- 在多文件编程中，对一个类：
- 我们需在头文件中暴露：
- 数据成员；
- 方法声明与（非内联）静态变量；
- 并在源文件中隐藏：
- 方法与静态变量的定义。
- 因此当我们：
1. 想增/删方法；
2. 想修改数据成员，无论改类型还是新增。
- 都必须修改头文件，且所有相关文件都要重新编译……

# PImpl

- 但理想情况下，当公开成员不变时，对用户暴露的内容不变；其他文件不应重新编译。
- PImpl 试图解决该问题。
- 头文件中的类仅持有一个指向其实现成员的指针，并暴露公开接口。
- 例如，普通实现的朴素示例：

当我们添加 `float cacheSum_, cacheProd_;` 并移除 `InnerProd_` 时，其他部分需要重新编译……

当我们添加 `float cacheSum_, cacheProd_;` 并移除 `InnerProd_` 时，只需修改此源文件。于是我们只需

# PImpl

重新编译单个文件，并重新链接。

- 若使用 PImpl：

# PImpl

- 我们注意到 pimpl 有许多变体。
- 例如，前述代码对继承处理不佳。
1. 派生类需为新成员分配新空间，导致内存碎片；
2. 不能随意更改受保护 API，因为会改变头文件。
- 可改进为：
1. 将 `BaseImpl` 类写入另一头文件，仅在被继承处包含（从而将重编译限制在少量文件）；
2. `DerivedImpl` 类再继承 `BaseImpl`；
3. `Base` 将指向 `BaseImpl` 的指针作为 `protected` 暴露；
4. 最后，`Derived` 继承 `Base`，在构造函数中用 `new` 的 `DerivedImpl` 赋给 `Base`；需要 `DerivedImpl` 时只需 `static_cast`。
- 此外，若想在 `ClassImpl` 中使用 `Class` 的接口，也可在 `ClassImpl` 中加入 `Class*` 指回。上述两种变体被 QT

# PImpl

等采用……并重命名为“d-pointer 与 q-pointer”。

# PImpl

- 优点：
- 项目较大时可显著减少构建时间。
- 维持二进制兼容性。
- 通常，数据成员变化时，对象布局也会变化；
- 则新版本头文件 + 旧版本共享库会崩溃；用户必须重新链接。
- 然而通过 pimpl，用户只传递指针，如何处理完全由库决定。
- 只要用户不使用新的公开 API，就不必重新链接。
- 完全隐藏原本在公开头文件中的成员，因此无隐私顾虑。

- 当然，凡事皆有代价……

# PImpl

- 缺点：
- 初始化开销：需要一次额外动态分配；
- 运行时开销：所有成员访问都多一次间接寻址；
- 无法内联简单方法，因为头文件不知道成员；
- 无法利用默认特殊成员函数（例如默认拷贝构造、默认析构等）；
- `const` 正确性问题：`const Class` 对象拥有的是 `ClassImpl* const` 而非 `const ClassImpl*`，因此 `Class` 的 `const` 方法可访问 `ClassImpl` 的非 `const` 方法。
- 从而需要额外注意以保持正确性。

# PImpl

- 我们注意到仍在手动管理指针……
- 使用 `std::unique_ptr` 似乎非常合适！
- 但编译时会失败……

- 原因：删除具有非平凡析构函数的不完整类型是 UB。
- 于是 `std::default_delete` 增强安全性，会在 `operator()` 内直接报错。
- 而默认析构函数在类内联，因此类型不完整时等价于调用 `operator()`。
- 解决办法：在源文件中写默认定义！

在头文件中：

# PImpl

在源文件中：
- 例如：

*
- 移动构造与移动赋值颇为相似。
- 有时默认移动并非我们所期望，因为它破坏 pimpl 的抽象。
- 它只指向实现，因此应表现值语义（即所有操作都应在底层对象上发生）。
- 例如，对拷贝，我们会写成：

*：严格来说，`unique_ptr` 的移动构造不要求完整类型。然而 C++ 规定构造函数可能调用子对象的析构函数（见 [class.base.init]），因此所有编译器都拒绝内联 `=default`。

# PImpl

- 类似地，对移动：
- 若愿意，可调用底层的移动构造与赋值。
- 于是移动后的接口指向移动后的实现，而非得到 `nullptr`。

- 相比裸指针更易实现，但 `std::unique_ptr` 仍有些不便。
- 你需要重新实现许多方法，如拷贝、比较等。
- 因为默认版本会拷贝/比较/……指针，那是指针语义而非值语义。
- 且 `const` 正确性仍是问题。

# `std::indirect`

- 自 C++26 起，可用 `std::indirect` 解决！
- 它是值语义的 `std::unique_ptr`，即主要操作直接调用底层对象的方法。
- 拷贝构造与赋值；
- 比较；
- 哈希。
- 以及一些特殊方法：
- `swap`：交换指针；
- 移动构造：转移指针。
- 于是被移动对象的存储指针将为 `nullptr`，可用 `.valueless_after_move()` 检查。
- 移动赋值：交换指针，并销毁另一方的资源。

# `std::indirect`

- 对构造函数：

通过转发 `v` 或 `args` 或初始化列表 + `args` 构造 `T`。

- 默认构造函数对底层对象做值初始化，而非拥有 `nullptr`。
- 每个构造函数都有分配器感知变体；分配器稍后讨论。

# `std::indirect`

- 最后也可用 `operator->`/`*` 访问。
- 所有方法会维持 `const` 正确性，例如此处 `const std::indirect<T>` 会通过 `const T*` 访问。
- 对 pimpl，于是很容易实现基本操作：
- 只需在源文件中全部 `=default`。

# `std::indirect`

- 我们注意到，若两个 `std::indirect` 的分配器不相等，实际效果略有不同。
- 例如，对移动构造 `std::indirect<T> a = std::move(b)`：
- 当分配器“相等”时，`a` 直接取得 `b` 的指针；
- 但当分配器“不相等”时，行为类似：
- `a` 用其分配器分配内存；
- 用 `std::move(*b)` 构造 `T`。
- 稍后再详述……

# `std::polymorphic`

- 最后，`std::indirect<T>` 只能处理 `T`，尽管它存储 `T*`。
- `std::polymorphic<Base>` 用于正确处理继承！
- 可在其中存储任意 `Derived` 对象。

此处 `U` 必须与 `T` 相同或为 `T` 的公开派生类。实参也用于构造 `U`。

# `std::polymorphic`

- 拷贝构造：构造 `Derived` 对象，其中 `Derived` 与底层被拷贝对象相同。
- 不会发生切片，即不会为 `std::polymorphic<Base>` 只构造 `Base`。
- 拷贝赋值：copy-and-swap，仍为防止切片问题。
- 对两个 `std::polymorphic<Base>`，假设底层对象为 `Derived1` 与 `Derived2`，它将存储指向 `Derived2` 副本的指针。
- 移动构造/赋值：与 `std::indirect` 相同，通过取指针与 swap-and-destroy。
- 析构：直接调用 `Derived` 的析构，即便 `Base` 的析构非虚。
- 由于类型可变，其他方法受限：

# 内存管理

- 智能指针
- `unique_ptr`
- 间接与多态（C++26）
- `shared_ptr`
- `weak_ptr`
- 适配器

# `shared_ptr`

- `unique_ptr` 表示对资源的独占所有权。
- 因此仅可移动。
- 有时存在被多方共享的资源……
- 每人都有权销毁它，且若为最后一个持有者则销毁。
- 此时可使用 `std::shared_ptr`。
- 例如：

你不应用同一裸指针构造两个 `shared_ptr` 来共享所有权；

应使用拷贝构造！

# `shared_ptr`

- 本质上，它维护一个原子引用计数；
- 正如我们在 stop token 处理中所述。
- 因此与仅存储原始指针的 `unique_ptr` 不同，`shared_ptr` 存储指向控制块的指针。
- 控制块示例：*
- 构造：计数置 1；
- 拷贝构造：将 `blockPtr` 设为 `another.blockPtr`，计数增加；
- 移动构造：将 `blockPtr` 设为 `another.blockPtr`，将 `another.blockPtr` 置 `nullptr`。
- 赋值：copy-and-swap 惯用法；
- 析构：计数减 1；当计数为 0 时销毁资源。

*：初始计数值略复杂，稍后讨论。

# `shared_ptr`

- 对构造函数：

`Y*` 必须可转换为 `T*`。

问题：不同于 `unique_ptr<T, Deleter>`，`shared_ptr` 只有一个模板参数 `<T>`；如何用删除器与分配器初始化？

# `shared_ptr`

- 通过类型擦除！（见第 12 讲《模板进阶》）
- 例如：

当然，此处可利用 EBO 压缩 `Deleter`。

- 因此对某些构造函数，即便传入 `nullptr`，仍可能需要分配。

# `shared_ptr`

- 除构造函数外，也可用 `make_shared` 构造。
- 或若需要分配器则用 `allocate_shared`。

- 不像赋值指针如 `PlainBlock`，它把对象直接存在控制块内。
- 只需分配一次，因此：
1. 减少运行时分配开销；
2. 防止内存碎片。
- 因此：通常优先使用 `make_shared` 与 `allocate_shared`，而非通过构造函数获得新的 `shared_ptr`。

# `shared_ptr`

- 注 1：自 C++17 起，`shared_ptr` 也正确支持数组语义。*

- 且自 C++20 起，`make_shared` 与 `allocate_shared` 也增加了相关重载：

每个元素值初始化；当 `T` 为无界数组时，例如 `std::shared_ptr<int[]>`。每个元素用 `u` 拷贝初始化。

当 `T` 为有界数组时，例如 `std::shared_ptr<int[5]>`。

*：此前可能编译通过，但不正确。详见 StackOverflow。

# `shared_ptr`

- 注 2：自 C++20 起，也增加了 `for_overwrite` 变体。

- 注 3：自 C++17 起新增的一些类型别名：

# 计数共享（Count sharing）

- 有时，两个 `shared_ptr` 共享同一所有权，尽管它们不可互相转换。
- 例如：

- `PartialWork` 只需使用 `a`，但它仍持有所有权（即需保持资源有效）。
- 为此，`shared_ptr` 引入所谓别名构造（aliasing ctor）。

# 计数共享（Count sharing）

作业中有 `LockFreeStack` 的实用示例。
- 例如：

- 于是它与 `ptr` 共享同一控制块；
- 注意当引用计数为 0 时，原对象被销毁。
- 特别地，`shared_ptr` 的 `operator<=>` 针对存储的指针比较；
- 因此别名指针可能与原指针不相等（即此处 `ptr.get() != &(ptr->a)`，假设 `int*` 与 `Resource*` 可比较）。
- 若要做基于所有者（owner-based）的比较，应使用 `.owner_xx`。
- 在基于所有者的含义下，别名指针总与原指针相等（因共享同一所有权）。
- 实践上只是比较控制块的地址。

# 计数共享（Count sharing）

- 例如：

前两个为 true + false；后两个均为 false。

# 计数共享（Count sharing）

- 注 1：也被包装为函数对象：
- 然而它只能比较同类型的两个指针；
- 但所有权可能在不同类型间共享（如前述 `shared_ptr<int>` 与 `shared_ptr<Resource>`）。
- 因此自 C++17 起，通过 `std::owner_less<void>` 增加模板 `operator()`。
- 很像透明比较器。

# 计数共享（Count sharing）

- 注 2：自 C++26 起，也增加基于所有者的相等性与哈希。

成员函子与方法：

- 但 `owner_hash` 与 `owner_equal` 根本不是模板；它们直接使用模板方法 `operator()`。

- 注 3：别名构造在 C++20 增加右值重载。
- 会像移动构造那样直接转移指针。

# 计数共享（Count sharing）

- 注 4：若想在转型指针上共享所有权，可使用 `std::xx_pointer_cast`：
- 等价于转型裸指针，再调用别名构造。

# 自共享（Self-sharing）

- 某些情况下，类可能需要在成员方法内保持“对自身所有权”。
- 例如：`boost::asio::tcp_connection::async_xx()` 的简化示例。

- 我们需要类似代码：但若 `Work` 返回时 `conn` 被析构，则新线程中的代码为 UB。

# 自共享（Self-sharing）

- 使其正确的两个因素：
1. `TcpConnection` 不能被正常构造；用户应只能获得其 `shared_ptr`。
- 这很容易；把所有构造函数隐藏为 `private`，并以静态 `shared_ptr<TcpConnection> Create(Args…)` 暴露。
2. 在 `AsyncRead` 中，必须获得该 `shared_ptr` 的拷贝。
- 然而，不能写 `shared_ptr<TcpConnection>{this}`；
- 我们已说过，用同一裸指针构造两个 `shared_ptr` 来共享所有权是错误的。
- 为解决该问题，可令 `T` 继承 `std::enable_shared_from_this<T>`。

- 例如：

1. 现在用户只能获得指向 `TcpConnection` 的 `shared_ptr`；
2. 在成员方法内共享所有权，使用继承来的 `.shared_from_this()`。

注意 `enable_shared_from_this` 由 `std::weak_ptr` 实现（稍后讨论），且自 C++17 起还有 `.weak_from_this()`。

*：TOCTOU（Time-of-check To Time-of-use），指当你使用值时，它已与检查时的值不同。（于是后续代码的前置条件可能不再成立。）

# `shared_ptr`

- 最后仅列出 API：
1. 构造与赋值也接受 `std::unique_ptr&&`，将所有权转移给自身。
2. `.reset()` 有接受删除器与分配器的变体，等价于构造再交换。
3. `.use_count()` 通常使用 relaxed 加载；在多线程环境中对 TOCTOU 问题通常不可靠。*

4. `std::get_deleter()`：若 `shared_ptr` 没有删除器则返回 `nullptr`；否则返回指向删除器的指针。（非成员）

所指对象的地址 对比

# `shared_ptr`*

其所指的控制块（控制块可能存储 `nullptr`）
- 一个不那么重要的说明……
- (A) `shared_ptr.get() == nullptr`（或 `!operator bool`）并不等价于 (B) `shared_ptr.use_count() == 0`（或称“空 `shared_ptr`”）。
- A && B：仅这两种构造。
- A & !B：其他赋 `nullptr` 的构造；

- B & !A：接受空源 `shared_ptr` 的别名构造，而别名指针不是 `nullptr`。

# 内存管理

- 智能指针
- `unique_ptr`
- 间接与多态（C++26）
- `shared_ptr`
- `weak_ptr`
- 适配器

# `weak_ptr`

- 有时我们不需要所有权，只需观察……
- 即，我们不强制资源存在；
- 而是使用时先尝试共享所有权；
- 若资源不存在也无妨；可另行处理。
- 否则我们获得其所有权并使用。
- `weak_ptr` 正是为此！
- 它对 `shared_ptr` 持有弱引用；
- 即，即便存在弱引用，只要没有共享引用，`shared_ptr` 的资源仍可释放。
- 当想使用资源时，用 `.lock()` 获得 `shared_ptr`；
- 可能返回空指针，需要特殊处理；
- 也可能返回有效 `shared_ptr`，于是持有所有权，可安全使用资源。

# `weak_ptr`

- 例如：

在并发环境中，某些资源可能被许多线程使用；与其反复加载，不如准备缓存。

当无人使用时，缓存会立即释放资源。

在使用时转换为或构造带 `shared_ptr` 的 `shared_ptr`。

*注意：除非迭代器访问是线程安全的，否则该函数仍可能线程不安全。

# `weak_ptr`

- 本质上，`shared_ptr` 的控制块也为 `weak_ptr` 维护弱计数。
- 当共享计数为 0（或称“已过期”）时，对象被销毁；
- 之后所有 `weak_ptr` 的 `.lock()` 将返回空 `shared_ptr`。
- 当弱计数也为 0 时，控制块被销毁。
- `weak_ptr` 也存储指向控制块的指针。

- 注 1：弱引用会给 `make_shared` 带来问题……
- `make_shared` 在单一控制块中分配所有内存；
- 且仅当弱计数为 0 时，控制块才会释放。
- 因此即便共享计数为 0，对象所占内存仍会保留。

# `weak_ptr`

- 因此在以下情形：
1. 对象内存非常大；
2. 可能被 `weak_ptr` 引用；
3. 希望及时释放内存；
- 使用 `make_shared` 并不合适。
- 注 2：`enable_shared_from_this` 仅有一个 `weak_ptr` 数据成员。问题：为何构造

# `weak_ptr`

`shared_ptr` 时会检测并赋值。不直接持有 `shared_ptr`？

# `weak_ptr`

- 注 3：`weak_ptr` 也可用于打破 `shared_ptr` 的循环引用。
- 例如，实现链表时：
- 通常我们会分配空节点并将两端相接。
- 链表只持有指向空节点的 `shared_ptr`。
- 然而当链表析构时，节点并未释放……
- 每个节点被前后节点引用，故所有 `.use_count()` 均为 2。
- 这产生不可达内存，导致内存泄漏……
- 真实场景中循环引用更微妙且难检测。
- 若确实需要此类循环引用，应插入 `weak_ptr`。

# `weak_ptr`

- 注 4：最后列出 API。
1. 由 `shared_ptr<Y>` 或 `weak_ptr<Y>` 构造，其中 `Y*` 可隐式转换为 `T*`。
2. 无 `.get()`，因为仅通过转换为 `shared_ptr` 再 `.get()` 访问才安全。

3. `.use_count()` 仍是共享计数。

4. 尽管 `shared_ptr` 的构造可接受 `weak_ptr`，但若 `weak_ptr` 已过期会抛出 `bad_weak_ptr`。因此应始终使用 `.lock()`。

# 原子智能指针（atomic smart pointer）

- 最后，对 `shared_ptr` 本身的并发访问会导致数据竞争。
- 例如：

- 原因：`shared_ptr` 持有指向原子计数器的指针（即带两个原子计数的控制块）；但指针本身非原子。
- 对裸指针，有特化 `std::atomic<T*>`；
- 自 C++20 起，增加 `std::atomic<shared_ptr/weak_ptr>`。

# 原子智能指针（atomic smart pointer）

- 用法也与 `std::atomic<T*>` 颇为相似。
- 但没有 `.fetch_add()`/`operator+=` 这类特化方法。

- 注 1：这不意味着底层对象被原子访问。
- 正如 `std::atomic<T*>` 不意味着可并行通过指针访问 `T`。
- 相反，应使用 `shared_ptr<atomic<T>>`，或通过 `atomic_ref`。
- 正如 `std::atomic<T>*`。

# 原子智能指针（atomic smart pointer）

- 例如：

# 原子智能指针（atomic smart pointer）

- 注 2：尽管 C++20 之前不存在 `atomic<shared_ptr>`，可对 `shared_ptr` 使用全局方法（无 `weak_ptr`）。
- 例如：

- 然而，由于函数难以保存状态，这些方法性能可能劣于特化类。
- 例如，`std::atomic<shared_ptr>` 可将 `std::atomic_flag` 作为数据成员实现，但函数很难做到。
- 因此在 C++26 中移除。

# 内存管理

- 智能指针
- `unique_ptr`
- 间接与多态（C++26）
- `shared_ptr`
- `weak_ptr`
- 适配器

# 智能指针适配器（Smart pointer adaptors）

- 在 C API 中，有时需要接受 `T**`，意为设置指针。
- 用智能指针管理略嫌笨拙：

- C++23 增加小工具以简化：

- 本质上，该函数创建临时的 `std::out_ptr_t<SmartPtr, RawPointer, Args…>`。

# 智能指针适配器（Smart pointer adaptors）

- `std::out_ptr_t` 基本上如同：

1. 它也有 `operator void**`，仅从 `RawPtr*` 做 `static_cast`。

2. 实际上是如下所示的 `.reset(static_cast<SP>(rptr), …)`：

# 智能指针适配器（Smart pointer adaptors）

- 额外实参 `args` 对 `std::shared_ptr` 很有用。
- 例如：

- 因通常与外部框架一起使用……
- 调用 `.reset()` 时几乎总要传入删除器。
- 因此当 `sizeof…(Args) == 0` 时，`std::out_ptr_t` 会使 `shared_ptr` 编译失败。

- 另有适配器 `std::inout_ptr_t`。
- 用于先释放资源再重新初始化的函数（很像 `freopen`）。

# 智能指针适配器（Smart pointer adaptors）

- 本质上如同：

1. 因外部函数会自动释放，我们不应调用 `.reset()`。（因此无法使用 `std::shared_ptr`）

2. 它们也支持裸指针；`std::out_ptr(rawPtr)` 与 `std::inout_ptr(rawPtr)` 基本上等价于 `&rawPtr` 或 `(void**)&rawPtr`。（务必注意可能的泄漏）

注意：这些适配器应始终通过将 `std::inout_ptr(…)` 或 `std::out_ptr(…)` 直接传入函数来使用。否则容易导致例如悬垂引用。

# 内存管理

分配器（Allocator）

# 内存管理

- 分配器
- 基础
- PMR

# 分配器（Allocator）

- 在标准库中，大多数需要动态内存分配的类型都会使用分配器。
- 用户可指定不同分配器以控制行为。
- 正式地，分配器是封装分配/释放策略的通用概念。
- 标准库通过 `std::allocator_traits<Alloc>` 提取分配器性质。
- 理论上，分配器需规定许多别名以泛化；
- 例如，`T` 的指针类型是什么？
- 但 `std::allocator_traits` 提供许多默认值，多数情况足够。
- 因此我们将省略琐碎部分。

# 分配器（Allocator）

- 例如：

唯一需要定义的。

默认值通常足够（例如 `T*`、`const T*`、……）

# 分配器（Allocator）

- 要支持最小分配器 `Alloc<T>`，你需要：
1. 嵌套类型 `value_type`，例如别名 `using value_type = T;`
2. 成员方法 `allocate(elemNum)` 与 `deallocate(ptr, elemNum)`；
3. “可相等比较”且“可拷贝”。2.
- 例如：

1.

3. 2.

3.

# 分配器（Allocator）

- 于是可与 `std::vector` 一起使用！
- 也可轻易编写“分配器感知”容器：

注意正确版本应释放先前内存。

# 分配器（Allocator）

- 可类似地对 `std::list` 应用分配器：
- 嗯，并不那么类似……
- `std::list` 真的只分配单个 `int` 吗？
- 不，它分配 `Node<int>`，无法由 `Alloc<int>` 处理。
- 而应由 `Alloc<Node<int>>` 处理。
- 因此分配器支持“rebind”，即将 `Alloc<T>` 变换为 `Alloc<U>`。

注意：在 C++20 之前使用 `Container<T, Alloc<U>>` 属于库 UB，自 C++20 起为编译错误。此处我们增强说明。

# 分配器（Allocator）

- 若在我们自己的 `vector` 实现中应用：

- 于是“可拷贝”意味着能将 `Alloc<U>` 转换为 `Alloc<T>`。

# 分配器（Allocator）

- 类似地，对我们此前的作业 `List` 实现：

用分配器重写……

# 分配器（Allocator）

# 分配器定义的构造/析构（Allocator-defined con/destruction）

- 此外，分配器还可控制构造与析构。
- 例如，分配器可能传递额外参数，因此仅直接 placement new 并不足够……

- 类似地，`allocator_traits` 会在其静态方法中提供默认版本：

- 对 `construct`，默认 placement new；
- 对 `destroy`，默认析构。

# 分配器（Allocator）

- 因此本质上应在代码中通过分配器管理：

- 然而，似乎缺少异常安全保证……
- 当构造抛出异常时，需要撤销分配。
- 于是可自行编写简单 RAII 包装：

# 分配器（Allocator）

- 更一般地，甚至可能需要撤销所有先前构造：

- C++ 提供未初始化内存算法（定义在 `<memory>`），但遗憾的是它们不感知分配器。
- 即构造只用 placement new，析构用析构函数。

# 分配器（Allocator）

自 C++20 起也提供 ranges 版本。

# 分配器（Allocator）

- 如同：

否则可能在 catch 块中抛出，后续元素不会被析构。

- 例如：分配器感知版本留作作业。

# 分配器传播（Allocator propagation）

- 最后，容器赋值时发生什么？
- 例如，此前我们说明过 `std::vector` 在移动赋值时通常做什么：
- 释放原内存并交换三个指针。
- 分配器在哪里？
- 因此本质上，分配器通过定义 `propagate_on_container_move_assignment`（POCMA）来决定应做什么。通常有两种情况：
1. 当 `POCMA::value` 为 true（即 POCMA 为 `true_type`）时：分配器被拷贝到被赋值容器；被赋值容器用原分配器释放内存，取得新分配器的别名，然后交换指针。
2. 当 `POCMA::value` 为 false（即 POCMA 为 `false_type`）时：被赋值容器保留自身分配器。
- 然后比较分配器是否相等；
- 相等分配器意味着其内存可互操作，即用分配器 A 释放分配器 B 分配的内存是正确的。

# 分配器传播（Allocator propagation）

- 于是对情形 2 再分两种子情况：① 当两容器分配器相等时，则与通常情形相同。
- 即释放原内存并交换三个指针。
- 无分配器拷贝。② 当两容器分配器不相等时，不能直接交换内存指针。
- 于是每个元素单独移动赋值。
- 若内存不足，原分配器应分配更多。
- 最后，分配器可定义 `is_always_equal` 以优化。
- 于是分配器比较可在编译期完成。
- 通常仅无状态分配器定义它，即所有分配器管理同一块内存（例如 `std::allocator` 仅管理全局堆）。

# 分配器传播（Allocator propagation）

- 类似地，可定义 `propagate_on_container_copy_assignment`（POCCA）与 `propagate_on_container_swap`（POCS）以决定拷贝赋值与 `swap` 的行为。
- 多数情况下，POCMA、POCCA 与 POCS 取值相同。
- 特别地，当 POCS 为 false 且分配器不相等时，直接 `swap` 良定义性不足（标准容器标为 UB）。
- 也可通过 `allocator_traits` 提取这些性质：

当分配器类为空类时，它不保存任何状态，被视为始终相等。

# 分配器传播（Allocator propagation）

- 例如：

# 分配器传播（Allocator propagation）

- 最后，容器拷贝构造时也会通知分配器。
- 只需定义 `select_on_container_copy_construction`（SOCCC），它应从当前分配器返回新分配器。
- 类似地，可通过分配器特征调用：
- 默认仅返回参数的拷贝。

- 例如：

# 分配器传播（Allocator propagation）

- 特别地，这不适用于容器的移动构造。
- 移动构造仅移动分配器并交换指针。

- 所有构造（包括拷贝与移动）都有分配器感知版本，会拷贝分配器并按其分配。

猜猜带分配器的移动构造如何实现。

# 分配器传播（Allocator propagation）

- 除容器外，再以 `std::indirect` 为例看如何处理传播行为。
- 拷贝构造：自身分配器要么用对方分配器的 SOCCC 初始化，要么用显式提供的参数 `a` 初始化。
- 拥有的对象从对方的对象分配并拷贝构造。

伪代码，无例如异常安全。

# 分配器传播（Allocator propagation）

- 移动构造：自身分配器要么从对方分配器移动而来，要么用显式参数 `a` 初始化。`other.valueless`
1. 当从对方分配器移动时，直接取其指针；移动后 `_after_move()` 为
2. 否则若其分配器与对方相等，直接取其指针；true。
3. 否则，拥有的对象被分配并从对方对象移动构造。

# 分配器传播（Allocator propagation）

- 拷贝赋值：最终在 POCCA 为真时拷贝分配器。
1. 若自身分配器与对方相等且自身非 valueless，则拷贝赋值底层对象。

2. 否则两分配器不相等，`std::indirect` 选择“拷贝并交换”以统一处理所有情况。

# 分配器传播（Allocator propagation）

- 严格地说，为避免不必要操作（例如临时需拷贝分配器），手动实现 copy-and-swap。

# 分配器传播（Allocator propagation）

- 移动赋值：基本上等价于拷贝赋值，区别是情形 1 不做移动赋值，而是窃取并销毁。

# 分配器传播（Allocator propagation）

- `Swap`：类似地，当 POCS 为 false 而分配器不相等时为 UB。

- 小结：尽管分配器有若干规范（例如通过 POCMA），具体传播行为仍由类自身决定。
- 例如移动赋值或窃取并销毁，取决于你的设计！

# 分配器（Allocator）

- 注 1：C++23 为分配器增加可选接口 `allocate_at_least`。

- 应分配不少于 n 个元素的空间。
- 这对容忍更多空间的分配很有用；典型例子是 `vector` 重分配。
- 例如，我们的分配器只能分配 16 字节块；但重分配策略想要三个 `int`（假设 4 字节）。
- 于是可返回 16 字节块，使该重分配容纳四个 `int`。

历史略复杂……`void*` 在编译器的 constexpr 求值器中似乎难处理，因此引入模板方法 `construct_at`。但实际上类型信息始终被跟踪，因此自 C++26 起允许在某些语境下转换为 `void*`，从而 placement new 也可行。

# 分配器（Allocator）

- `allocator_traits` 的默认行为就是返回 `{allocate(alloc, n), n}`，即精确分配请求大小。
- 注 2：placement new 自 C++26 起可为 `constexpr`（带若干限制，见第 11 讲作业）。
- 在此之前，自 C++20 起可使用模板函数 `std::construct_at`。
- 这也是 `allocator_traits::construct` 的默认行为在 C++20 改为 `std::construct_at` 的原因。

# 分配器（Allocator）

特别说明：在 [issue 3436](https://cplusplus.github.io/LWG/issue3436) 之前，`std::construct_at` 无法正确处理有界数组；该 issue 旨在使其正确返回指向数组的指针。

- 对称地，`std::destroy_at`：

- 实际上析构函数可为 `constexpr`，因此 C++17 引入该函数，行为如同 `ptr->~auto()`，因类型由模板推导。

# 内存管理

- 分配器
- 基础
- PMR

# PMR

- 我们说分配器相等，当其内存可互操作时。
- 因此实现分配器的简单方法是用静态变量表示其内存竞技场（arena）：

- 然而，每个竞技场导致唯一分配器类型……
- 糟糕，你必须写模板代码以支持每个分配器。
- 且容器不支持例如对不同分配器做拷贝。
- 你必须写例如 `.assign(another.begin(), another.end())`。
- 相反，可使用相同分配器类型，并通过多态内存资源（PMR）控制内存竞技场。

定义于 `<memory_resource>`。所有组件定义在命名空间

# PMR

`std::pmr` 中，下文不再重复说明。

- 分配器就是 `std::pmr::polymorphic_allocator`，它控制由 `std::pmr::memory_resource` 表示的内存。
- 如同：

你可改变分配策略而不改变分配器类型。

- `memory_resource` 需暴露如下接口：

# PMR

- 要定义自定义资源，只需继承 `memory_resource` 并覆盖私有虚函数：

- 因此本质上 `memory_resource` 只是使用模板方法模式。
- 一个非常朴素的例子：

# PMR

- 标准库中有一些预定义内存资源。
- 两个由函数返回的朴素单例资源：
1. `null_memory_resource()`：不分配任何内容。

单例只需比较单例地址，无需 `dynamic_cast`。

# PMR

2. `new_delete_resource()`：使用 `operator new` 与 `delete`。

- 存在“全局默认资源”，可通过 `get_default_resource()` 与 `set_default_resource(ptr_to_mr) -> old_ptr`（线程安全）获取/设置。
- 很像 `std::get`/`set_new_handler()`。
- 初始默认资源就是 `new_delete_resource()`。

# PMR

- 以及三个复杂类。
- 它们都有“上游”内存资源；当自身内存耗尽时，会向上游请求更多。
- 默认上游就是全局默认资源。

1. 内存池：`synchronized_pool_resource` / `unsynchronized_pool_resource`；
- 它由一组块池（bin）组成，每个池拥有许多相同大小的块。
- 正如我们在 sized-delete 的分配策略中所述。
- 分配被派发到能容纳所需大小的最小 bin。
- 释放将内存归还池，池可能进一步也可能不向上游释放块。
- 且 `unsynchronized_pool_resource` 假设所有分配在同一线程发生（即无竞争可能），通常比同步版本更快。

# PMR

- 如同：

致谢：Thanks for the memory (allocator) - Sticky Bits - Powered by Feabhas

# PMR

- 可用结构体 `pool_options` 配置池，其有两个数据成员：
- `std::size_t max_blocks_per_chunk`：当池从上游分配新块（chunk）时，一次最多允许分配多少块。
- 实现可使用比指定更小的值。
- `std::size_t largest_required_pool_block`：池中最大块大小；若所需大小更大，则直接从上游分配。
- 实现可使用直通阈值（pass-through threshold）。
- 每个成员存在实现定义的上限，当提供大于上限的值或 0 时将使用该上限。
- 非继承 API 很简单：

等价于调用 `upstream->deallocate()`。

`Pool(const pool_options& opts, memory_resource* upstream)`，

# PMR

且每个参数均可省略（用默认值填充）。

例如，此处默认构造等价于 `{ pool_options{}, get_default_resource() }`。
- 例如：

`std::pmr::list<T>` 是 `std::list<T, std::pmr::polymorphic_allocator>` 的别名。

实际由 `polymorphic_allocator` 构造，后者可进一步由 `memory_resource*`（隐式）构造。

块的 chunk 已耗尽，于是从上游分配，导致间隔跳跃。尽管 `sizeof(Node)` 是 24 或 176，块大小为 32 与 256，通过内存资源，我们可安排节点整体分配得彼此靠近，从而对缓存更友好。

# PMR

- 2. 单调内存：`monotonic_buffer_resource`。
- 已分配内存永不释放；新分配单调填充缓冲区。
- 因此分配与释放非常快。
- 例如：分配（忽略对齐）“释放” 1 字节 实际什么也不做 分配从第 1 字节开始而非第 0 字节。分配发生 2 字节

- 可提供初始缓冲区（不由类管理）；耗尽时，`monotonic_buffer_resource` 会向上游请求，且
1. 像 `vector` 重分配一样，请求大小指数增长。
2. 当 `monotonic_buffer_resource` 析构或调用其 `.release()` 时，所有请求的内存都会被释放（即调用 `upstream->deallocate()`）。
- `.release()` 会将缓冲区与大小重置为初始缓冲区与大小。

*注意：不同实现的重分配策略不同。此处使用 MS-STL 与 Visual Studio Release 模式。在 Debug 模式下，MS-STL 可能分配更多内存用于安全检查。

# PMR

- 例如：

用栈作为初始缓冲区使分配非常快。

在栈上分配。

由于缓冲区最多容纳 16 个 `int`，必须从上游分配新缓冲区。且尽管初始缓冲区可容纳 13 个 `int`，`monotonic_buffer_resource` 永不回退，因此仍在堆上分配。

# PMR

- 并注意 `monotonic_buffer_resource` 的构造函数：

(1~4)：将当前缓冲区设为 `nullptr`；当发生分配时，用 `initial_size` 作为初始缓冲区从上游分配。省略 `initial_size` 则为某实现定义值，省略 `upstream` 则使用默认资源。

(5, 6)：用显式缓冲区设置；省略 `upstream` 则使用默认资源。

# PMR

- 当然，可通过设置上游组合这些资源。
- 如我们之前在协程中的例子：

分配失败时，不请求更多内存；而是抛出异常。

# PMR

- 注 1：PMR 中大量使用裸指针，因此必须仔细管理生命周期。
1. 需确保在使用时上游资源与初始缓冲区有效。
2. 调用 `set_default_resource` 时，原资源不应立即销毁，因为先前分配的容器可能仍在使用它。
3. 当你析构 `monotonic_buffer_resource` / `(un)synchronized_pool_resource` 的对象（或等价地调用 `.release()`）时，需确保没有东西仍占用其分配的内存。

*存在 DR 以修复此前不必要的 `dynamic_cast`。

# PMR

- 注 2：这些类有一些特殊性质：
- 它们既不可拷贝也不可移动。*
- 不同资源对象始终不相等。
- 尽管理论上这些内存资源可能互操作，标准库选择保守地标记为不相等。
- 注 3：PMR 是“粘性”的；它不遵循 POCMA、POCCA 与 POCS。
- 且 SOCCC 仅返回默认构造的分配器（使用默认内存资源，而非与被拷贝分配器相同资源）。
- 并记住……

- 这导致通常对具有不相等 `polymorphic_allocator` 的两个 `std::pmr` 容器执行 `swap` 为 UB。
- 特别地，不同标准 MR 始终不相等，因此此类代码为 UB：

# PMR

- 注 4：自 C++20 起，`polymorphic_allocator` 增加一些工具以避免 rebind。

等价于直接用 `resource` 分配字节（而非 `* sizeof(T)`）。默认对齐参数是 `alignof(std::max_align_t)` 而非 `alignof(T)`。

带模板参数 `<U>`，等价于用相同内存资源的 `polymorphic_allocator<U>` 进行分配。

带模板参数 `<U>`，等价于用相同内存资源的 `polymorphic_allocator<U>` 进行分配与构造。异常安全，即构造失败时会释放空间。

且 C++20 还将 `std::byte` 作为 `polymorphic_allocator` 的默认模板类型参数。我个人认为这是一个设计失误，与 Arthur O’Dwyer 的观点一致。

# 使用分配器的构造（Uses-allocator Construction）

- 最后，PMR 会执行 uses-allocator 构造以“向下传播”分配器。
- 也就是说，`polymorphic_allocator::construct` 会尝试将自身传给被构造对象。
- 简单示例：

- 此处因 `std::pmr::string` 可接受 `polymorphic_allocator`，`vector` 的分配器会传给新构造的 `std::pmr::string`。
- 于是 `string` 也从池中分配内存。

# 使用分配器的构造（Uses-allocator Construction）

- 本质上，是否将分配器传播给类型 `T` 的新对象由以下步骤判断：
1. 用特征 `std::uses_allocator_v<T>` 判断 `T` 是否使用分配器。
- 若 `T` 有嵌套类型 `allocator_type` 则为 true。
- 或你可像许多其他类型一样手动特化：
2. 若 1. 为 true，寻找传递分配器的方式；① 先尝试前导分配器约定，即可用 `T(std::allocator_arg, alloc, args...)` 调用构造。② 若失败，再尝试尾随分配器约定，即可用 `T(args..., alloc)` 调用构造。③ 否则非良构。
- 若最终找到方式，则通过构造传播分配器。

# 使用分配器的构造（Uses-allocator Construction）

- 自 C++20 起，该过程封装为 `std::make_obj_using_allocator<T>(alloc, args…)`；
- 本质上：

- 其中 `<tuple>` 中的 `std::make_from_tuple<T>` 与解包 tuple 并构造 `T` 相同（很像 `std::apply`）；
- 且 `std::uses_allocator_construction_args<T>` 判断调用约定，并返回满足约定的对应 tuple；
- 特别地，若 `std::uses_allocator_v<T>` 为 false，则忽略 `alloc` 并返回指向 `args` 的引用的 tuple。

# 使用分配器的构造（Uses-allocator Construction）

- 若要做 placement new，可使用 `std::uninitialized_construct_using_allocator<T>(p, alloc, args…)`。

- 于是 `polymorphic_allocator` 的 `construct` 可轻易实现：

# 使用分配器的构造（Uses-allocator Construction）

- 注 1：注意在容器外构造对象，可能导致低效。
- 例如，解释下面代码中发生什么：

- `.push_back` 接受 `std::pmr::string`，因此在外部构造。
- 从 `const char*` 到 `string`，若未提供分配器则使用默认构造的分配器，意味着使用默认内存资源！
- 因 PMR 粘性且不相等，会发生逐元素移动（在 `string` 中等价于拷贝）。
- 解决办法：要么用 `.emplace_back`，要么显式用分配器构造。

# 使用分配器的构造（Uses-allocator Construction）

- 注 2：`std::uses_allocator_v<std::pair<…>>` 为 false，但 `std::pair` 实际对其两个元素使用分配器。
- 其构造不满足调用约定，因此 uses-allocator 构造为其提供大量重载（每个对应一个构造重载）。

# 使用分配器的构造（Uses-allocator Construction）

- 相比之下，`std::tuple` 使用前导分配器约定，因此只特化 `std::uses_allocator` 而无其他特殊处理：

定义于 `<scoped_allocator>`

- 注 3：标准库还提供一种将分配器“传播”以构造子对象的分配器。
- 更准确地说，它一次性收集分配器并逐级分发。
- 例如，对 `vector` of `vector` of `int`……
- 我们需要两个分配器，分别用于 `int` 与 `vector of int`。

- 因此通常需要写如下代码：

- 若忘记传递 `b` 呢？

# 使用分配器的构造（Uses-allocator Construction）

- 则新 `vector` 使用默认构造的分配器，而非 `b`。

- 若使用 scoped allocator：

# 使用分配器的构造（Uses-allocator Construction）

- 对嵌套容器，需使用嵌套 scoped allocator：

`Scoped<Outer, Inners…>{ o, is… }` 会用 `o` 分配并构造子对象 `S`；构造时传入 `Scoped<Inners…>{ is… }` 以执行 uses-allocator 构造。

# 使用分配器的构造（Uses-allocator Construction）

- 若停止使用 scoped allocator，则 uses-allocator 构造找不到调用约定，从而不带分配器转发。

过程：
1. `LVVec` 用 `LVAlloc`（`a`）构造，并传入 `ScopedAlloc1`（`b+c`）给 `VVec`；

2. `VVec` 接受 `VAlloc`，而 `ScopedAlloc1` 继承自 `VAlloc`，因此 `VVec` 用 `b` 构造。
- 此处丢弃 `c`！

3. `VVec` 用 `VAlloc`（`b`）构造；因非 scoped allocator，`Vec` 以默认 `Alloc` 正常构造。

# 小结（Summary）

- 底层内存管理；对象布局、对齐；`operator new` 与 `delete`。
- 分配器：接口与特征（POCMA、POCCA、POCS、SOCCC）；PMR；uses-allocator 构造。
- 智能指针：`unique_ptr`；pimpl；`indirect` 与 `polymorphic`；`shared_ptr`、`weak_ptr`；原子特化；`out_ptr`、`inout_ptr`。

# 下一讲……

- 最终，我们已成功完成所有主要主题！
- 在最后一讲中，我们将快速覆盖尚未涉及的主题。
- 文件系统；
- Chrono（与时间相关的设施）；
- 数学（包括随机数生成）；
- 最后，我们将对 C++26 做粗略介绍以展望未来。
