并发进阶 Advanced Concurrency

## 现代C++基础

## Modern C++ Basics

Jiaming Liang，北京大学本科生

### • 内存序基础（Memory Order Basics）

### • 原子变量细节（Atomic Variable Details）

### • 进阶内存序（Advanced Memory Order）

### • 协程（Coroutine）

# 并发进阶（Advanced Concurrency）

内存序基础（Memory Order Basics）

“即便有 C++11 支持，我也认为无锁编程属于专家级工作。”—— Bjarne Stroustrup，HoPL4，第 33 页

# 并发进阶（Advanced Concurrency）

- 内存序基础（Memory Order Basics）
- 概览（Overview）
- 顺序一致模型（Sequentially consistent model）
- 获取-释放模型（Acquire-release model）
- 宽松模型（Relaxed model）

- 还存在 consume-release 模型，但由于用户很难标注、编译器也很难分析以获得更好的优化，所有编译器都会把 consume-release 加强为 acquire-release 模型。
- C++20：

- C++26：consume 操作被弃用（deprecated）。

# 内存序（Memory order）

- 当今编程世界建立在顺序执行的基础之上……
- 编译器 / JIT 可能做激进优化……
- 这里我们会把全局变量“缓存”到寄存器，并消除冗余表达式（例如 `b = addend + 1`）。

- 处理器可能乱序执行与推测计算……
- 每个处理器可能有自己的 L1/L2 缓存……

# 内存序（Memory order）

- 这些优化在顺序世界里既聪明又正确，但进入并行世界后，有些假设就不再那么直观……

- 如果还有另一个线程在这里修改 `addend` 呢？
- `b` 可能不是 `tempb + 4`，但编译器优化会让这种情况不可能发生。

# 内存序（Memory order）

- 在众多编译器优化、处理器 ISA 规定、缓存一致性协议……之中
- 我们需要找到一种方式，用抽象来统一“如同”（as-if）行为！
- 这就是 C++ 中内存序（memory order）的作用。
- 三种内存序：
- 顺序一致模型（`seq_cst`）
- 获取-释放模型（`acq_rel`）
- 宽松模型（`relaxed`）
- 顺便一提：Rust 在这方面的规定与 C++ 完全相同。

Atomics - The Rustonomicon

[1]：A Concurrency Semantics for Relaxed Atomics that Permits Optimisation and Avoids Thin-air Executions | POPL’16，Jean & Peter，剑桥大学。POPL 是程序设计语言设计方向的顶级学术会议。

# 内存序（Memory order）

- 但如何描述内存序在学术界仍是一个未解问题（即便 `seq_cst` 模型在 C++20 中也有缺陷修复）。
- 而 C++ 在该领域是先驱，因此标准几乎每个版本都会修订。
- 但通常这是理论模型上的缺陷；现实行为并不会受到严重影响。[1]
- 关键问题是：内存序是公理式的（axiomatic），因而相当弱，无法精确描述我们真正想要的。
- 内存序给出约束，任何能满足约束的结果都是合法解。而某些解其实并不真的合法……我们稍后会看到它们。内存序规范的内容：

形式上，标准用 RR/RW/WR/WW 来规范；这里我们换一种表述。

# 内存序（Memory order）——标准中的连贯性（coherence），此处重述。

- 内存模型里有一些直观的基本规则。
1. 修改顺序（modification order）：对单个原子变量，所有线程看到的操作序列相同。
- 那么 `r1 == 1 && r2 == 2 && r3 == 2 && r4 == 1` 可能吗？
- 不可能！
- 原因：`r4` 不能读到比 `r3` 更新的值，`r2` 也不能读到比 `r1` 更新的值。
- `r1 == 1 && r2 == 2`：2 比 1 新；
- `r3 == 2 && r4 == 1`：1 比 2 新；矛盾！
- 编译器不允许重排。
- 但对不同原子变量的操作，在不同线程里可以有不同的顺序。

# 内存序（Memory order）

2. 先序于（sequenced before）：我们之前讲过求值顺序……

# 内存序（Memory order）

- 若求值 A 一定在求值 B 之前完成计算，则称 A sequenced before B。
- 例如，对不同语句。
- 在同一条语句中：

- 自 C++17 起函数实参的求值顺序为 indeterminately sequenced，因此有某种顺序但未指定；
- 有些求值完全不受约束，即它们是 unsequenced 的（例如 `a = b++ + b` 是未定义行为（UB），因为 `b++` 与 `b` 无顺序关系而 `b++` 有副作用）。
- 再次强调：这种顺序存在于顺序视角中……

# 内存序（Memory order）

- 当对同一内存位置的非原子操作不存在某种特定的 happens-before 关系时，就会发生数据竞争（data races）。

3. Happens-before：在并行世界里，哪个求值先执行由 happens-before 来约束。
- 若 A sequenced before B，则 A happens before B（单线程情形）；
- 若 A synchronizes-with B，则 A happens before B（线程间情形）；
- 或 A happens before B 且 B happens before C，则 A happens before C。

- 对非原子变量，只有当 A happens before B 时，A 的效果才对 B 可见。
- 因此只要不可见，编译器就可以做激进优化。
- 对原子变量，HB 顺序是内存序的一部分；若两操作没有 HB 关系，则它们在内存序中的顺序也可能是任意的。
- 亦即，若 B 不 happens before A，则 A 的效果仍可能对 B 可见。
- 内存序主要规范这种 synchronize-with 关系。注意：我们这里讲授的术语自 C++26 起是 happens-before；在此之前（自 C++20）它被称为 simply-happens-before，但当不涉及 consume 操作时，它与自 C++11 起的 happens-before 等价（而我们已说明 consume 操作从未被真正实现）。

# 并发进阶（Advanced Concurrency）

- 内存序基础（Memory Order Basics）
- 概览（Overview）
- 顺序一致模型（Sequentially consistent model）
- 获取-释放模型（Acquire-release model）
- 宽松模型（Relaxed model）

# 顺序一致性（Sequential Consistency）

- 在现实世界里，所有事件都以某种方式排成序列，所有观察者看到同一序列。
- 类似地，我们可以认为操作存在某种全序（total order），所有线程观察到同一顺序。
- 这就是顺序一致模型的核心！
- 回到例子：`x.load()` `r1=0` `x.store(1)` `x.store(1)` `x.load()` `r3=1` `x.store(2)` `x.store(2)` `x.load()` `x.load()` `x.load()` `r4=2`

`x.load()` `x.load()` `x.load()` `r2=2` 随机交错它们，就得到全序。

# 顺序一致性（Sequential Consistency）

- 形式化地，当原子加载操作 B 读到由原子存储操作 A 写入的值时，A synchronizes-with B。
- 于是自 B 起，此前所有结果都可见。
- 例如：

- 这个 `assert` 会触发吗？

- 不会，`z.load()` 永远非零。
- 原因：存在全序，因此要么 `x.store(true)` 要么 `y.store(true)` 先发生。
- 由于完全对称，不妨假设 `x.store(true)` 先发生。

- 对线程 3：仅当 `x.load()` 读到 `true` 才能继续；对线程 4：仅当 `y.load()` 读到 `true` 才能继续（二者分别与对应的 `x.store(true)` / `y.store(true)` 形成 synchronize-with）。
- `y.load()` 可能仍为 `false`；也可能在另一交错下 `x.load()`、`y.load()` 读到 `true`。

- 同步通过“读到某个值”隐式建立。
- 注意：`x.store(true)` 与 `y.store(true)` 没有 happens-before 关系；顺序由全序强加。

- `seq_cst` 模型实际使用 strongly-happens-before 关系，但这里二者等价；下一节会讲到。

# 顺序一致性（Sequential Consistency）

- 注意 2：线程开始与线程 `join` 也会与函数开始与返回建立 synchronize-with 关系。
- 因此这里线程 `join` happens before `z.load()`，函数返回 happens before 线程 `join`，而 `++z` happens before 函数返回。于是 `z.load()` 能正确得到 1 或 2。
- 注意 3：对原子变量的操作不可分割（从而防止数据竞争），这一点不受内存序影响。
- 称为原子性（atomicity）。
- 我们上节课的例子：
- 若 `a` 是原子变量，则不需要锁保护。

# 并发进阶（Advanced Concurrency）

- 内存序基础（Memory Order Basics）
- 概览（Overview）
- 顺序一致模型（Sequentially consistent model）
- 获取-释放模型（Acquire-release model）
- 宽松模型（Relaxed model）

# 获取-释放模型（Acquire-release Model）

- 在许多架构（如 RISC-V、ARM、Power）上，这种全序假设代价很高，而它们对更弱模型支持更好。
- acquire-release 是常见支持的序！

- 那么 acquire-release 模型保证什么？
- 只有读操作可以是 acquire，只有写操作可以是 release。
- 对一次 acquire 操作 B，若它读到来自 release 操作 A 的值，则 A synchronizes-with B（从而 A happens before B）。
- 没有全序。

# 获取-释放模型（Acquire-release Model）

- 例如：

- `store` 之前的代码 sequenced before 该 `store`，因此 happens before 该 `store`。
- 仅当 `ptr` 加载到某个非空值后程序才继续，于是 `store` synchronizes-with `load`；由此经 happens-before 传递可知：`load` 之后的代码 sequenced after 该 `load`，故 `data` 恒为 42，且该 `load` happens before `assert`。

# 获取-释放模型（Acquire-release Model）

- 另一方面，由于没有全序：

对 `x` 与 `y` 的 store 没有 happens-before 关系。

这里只知道 `x` 为 true（synchronize-with），而 `y.load` 与 `y.store` 未必有 happens-before 关系。

同理，`x.load` 与 `x.store` 也未必有 happens-before 关系。

因此这里 `z.load()` 可能为 0。

# 获取-释放模型（Acquire-release Model）

- 传递性的另一个例子：
- SB(#0, #1)
- SW(#1, #2)
- 因为只有当 #2 读到 true 时 `thread_2` 才会继续。
- SB(#2, #3)
- SW(#3, #4)
- SB(#4, #5)
- 于是可知 HB(#0, #5)。

显然，acquire-release 模型可用于实现自旋锁。

# 获取-释放模型（Acquire-release Model）

- 借助 happens-before 关系，acquire-release 模型隐式禁止编译器重排优化。
- 一次 acquire 操作 B 可能发生在另一次 release 操作 A 之后……
- 若编译器把语句 S1 从 B 之后重排到 B 之前；
- 或把语句 S2 从 A 之前重排到 A 之后；
- 则 S1 可能看不到 S2 的结果。
- 因此 acquire 与 release 隐式提供单向指令屏障。
- 所有会产生副作用（可能被其他线程使用）的操作不能越过 release 向下“逃逸”；
- 所有可能依赖副作用的操作不能越过 acquire 向上“逃逸”。
- 直观上，acquire-release 形成某种临界区；不能把中间代码移出去。

# 并发进阶（Advanced Concurrency）

- 内存序基础（Memory Order Basics）
- 概览（Overview）
- 顺序一致模型（Sequentially consistent model）
- 获取-释放模型（Acquire-release model）
- 宽松模型（Relaxed model）

# 宽松模型（Relaxed Model）

- 有时我们还想更弱……
- 即只需要保持原子性；不需要 synchronize-with 关系。
- 这就是宽松模型。
- 例如：

练习：这个 `assert` 会触发吗？

# 宽松模型（Relaxed Model）

- 不会！
- 假设 `r1 == 42`，
- 则 #1 从 #4 读到值，acquire-release 模型给出 SW(#4, #1)。
- 且 SB(#3, #4)、SB(#1, #2)，于是 HB(#3, #2)。
- 因此 #2 的效果对 #3 不可见，`r2` 必为 0。
- 那宽松模型呢？
- 这个断言可能触发……
- 即 `r1 == 42 && r2 == 42` 可能为真。

# 宽松模型（Relaxed Model）

- 既然宽松模型不建立任何 synchronize-with 关系……
- 回忆我们的效果规则？

- 这里 #1 不 happens before #4，于是 #4 的效果可被 #1 读到，故 `r1 == 42` 可能为真。
- 且 #1 happens before #2，于是 `x` 可存 42。
- 且 #3 不 happens before #2，于是 #2 的效果可被 #3 读到，故 `r2 == 42` 可能为真。
- 因此 `r1 == 42 && r2 == 42` 可能为真。

# 宽松模型（Relaxed Model）

- 注意 1：再次强调没有全序。
- 若有全序，则在线程 2 中 SB(#3, #4) 会阻止任何顺序使 `r2 == 42`。
- 实践中，编译器允许重排 #3 与 #4，因为破坏该 HB 不会影响任何可见效果。
- 注意 2：该结果不违反单个原子变量的修改顺序约束。所有线程看到同一修改顺序。

- 注意：此处 #3 读到的组合可以出现多种情况；值为 42 是“可以”而非“必须”。#1 与 #3 也可能读到更旧的值。在满足单变量修改顺序的前提下，`r1` 仍可以为 42。

# 宽松模型（Relaxed Model）

- 另一个复杂例子：

像自旋锁，后面会讲。

所有线程现在开始。

# 宽松模型（Relaxed Model）

- 它做的事情是：
- 三个线程，每个只修改一个原子变量，并读取全部；
- 两个线程只读取全部原子变量。
- 只能保证：
- 修改该变量的线程会按 happens-before 关系看到它逐个递增。
- 例如 `values[0]` 会是 `(0, …, …)`、`(1, …, …)`、…、`(9, …, …)`。
- 且受单原子修改顺序约束，未被自己修改的其他变量取值非递减。
- 即一旦读到某个值（未必最新），就不可能再读到比它更旧的值。

# 宽松模型（Relaxed Model）

- （书中示意图，第 2 版。）
- 摘自 Anthony Williams《C++ Concurrency in Action》第 2 版。

# 宽松模型（Relaxed Model）

- 宽松模型可能产生非常惊人的结果，因此必须极其谨慎地使用……
- 通常要么配合其他同步操作（如 acquire-release 模型）……
- 要么只做只需要原子性的很简单工作。
- 例如 `std::shared_ptr` 有计数器统计拷贝份数；当所有拷贝析构后内存才最终释放。
- 可用 `.use_count()` 查看共享计数，通常这是 relaxed load，因为它不需要参与同步。

# 并发进阶（Advanced Concurrency）

原子变量（Atomic Variables）

# 并发进阶（Advanced Concurrency）

- 原子变量（Atomic variables）
- 基本操作（Basic operations）
- `atomic_flag`
- 特化（Specializations）
- `atomic_ref`

# 基本操作（Basic Operations）

- 可把原子操作分为三类：
- 读操作；
- 写操作；
- 读-改-写（RMW）操作。
- 对最一般的原子类型 `std::atomic<T>`：
- 读操作是 `.load(memory_order=std::memory_order_seq_cst)`；
- 以及 `operator T`，其只能使用 `seq_cst` 作为序。
- 写操作是 `.store(T newObj, memory_order=std::memory_order_seq_cst);` 注意不是 `T&` 或 `std::atomic<T>&`！
- 以及 `operator=(T)`，同样只能用 `seq_cst`，并返回 `T newObj`。
- 注意原子类型既不可复制也不可移动。

对接受内存序的原子操作方法，默认参数为 `std::memory_order_seq_cst`；若不接受内存序，则同样使用 `std::memory_order_seq_cst`。后续幻灯片不再重复说明。

# 读-改-写（Read-Modify-Write）

- 还需要原子 RMW 操作……
- 这与“原子读 + 原子写”不同，因为两次原子操作是可分割的。
- RMW 操作整体不可分割。
- 例如：`a++`；
- 读：`temp = a`；改：`temp++`；写：`a = temp`。
- 若可分割，两个线程跑 `Inc` 仍可能得不到 200000。

- RMW 操作包括：
- `.exchange(T desired, memory_order) -> T`：读出旧值并写入 `desired`；返回旧值。
- 实际上没有“改”，但读与写作为整体是原子的。

# 读-改-写（Read-Modify-Write）

- 以及组合操作：
- `.compare_exchange_strong(T& expected, T desired, memory_order success, memory_order failure) -> bool`：
- 读操作：读出值 `v`，与 `expected` 比较；
- 改操作：无修改；
- 写操作：若相等（成功），写回 `desired`；否则（失败）不写回（但执行 `expected = v`）。
- 返回值表示是否成功。
- 因此有点奇怪的是：其操作类型取决于读操作：
- 若失败，则只是 load；
- 若成功，则是 RMW（整体原子）。
- 于是可以指定两种内存序。

这类操作通常称为 CAS（compare-and-swap / compare-and-set）。CAS 是无锁数据结构的基础操作。

# 读-改-写（Read-Modify-Write）

- 由于 RMW 同时涉及读与写，内存序会同时约束二者。
- `seq_cst`：读与写都使用顺序一致模型。
- `relaxed`：读与写都使用宽松模型。
- 但对 acquire-release 模型，acquire 与 release 是分开的……
- 于是可使用 `std::memory_order::acq_rel`！
- `acq_rel`：使用 acquire-release 模型，读是 acquire，写是 release。
- `acquire`：读是 acquire，写是 relaxed。
- `release`：写是 release，读是 relaxed。
- 且 RMW 保证在内存序中读到最新值，并把结果作为最新值写回。

# 读-改-写（Read-Modify-Write）

- 还存在只接受一个内存序的重载：
- `.compare_exchange_strong(T& expected, T desired, memory_order success) -> bool`；
- 失败时的序来自 success，因为 RMW 的序包含读序。

- 注意：失败默认不是 `seq_cst`。

但我们注意到用 RMW 做自旋锁通常不是好主意，因为写在

# 读-改-写（Read-Modify-Write）

缓存一致性协议（如 MESI）里较特殊，会损害效率。这里仅作示例。

- 用之前的例子：

可改写为：

# 原子操作（Atomic Operations）

- 注意 1：原子变量按位比较并按位写入。
1. 自定义 `operator==` 不影响 CAS；
2. 特别地，对浮点数按位比较很容易误导。
- 例如 `-0.0` 与 `0.0` 按位不相等。

- 当 `expected` 为 `-0.0` 时该 CAS 返回 false；为 `0.0` 时为 true。
3. 为使按位写入合理，`std::atomic<T>` 对 `T` 有约束——`T` 应为 trivially copyable。

# 原子操作（Atomic Operations）

- 注意 2：自 C++20 起填充位（padding bits）不参与比较。
- C++20 之前会参与比较。
- 形式化地：对象表示（object representation）对比值表示（value representation）。Object Representation

0 1 2 3 4 5 6 7

char c padding float f

Value Representation

内存布局会在《内存管理》中详讲。

# 原子操作（Atomic Operations）

- 原因：比较 padding 可能导致惊人的假阳性。若真要比较 padding，可手动把 padding 作为成员安排。

- 但若为原子 `union`，当不同类型值表示不同（即 padding 位置不同）时，只省略共享的 padding 部分。

- 注意：
1. libc++ 尚未实现；libstdc++ 目前按 DR 实现（即自 gcc13 起，无论你指定何种标准，都只比较值表示）。
2. 对 `union`，无论类型是否共享 padding 位置，截至 2025/7，MS-STL 仍只比较对象表示。可见简单测试。

# 原子操作（Atomic Operations）

- 注意 3：还存在 `.compare_exchange_weak(…)`，参数与 `.compare_exchange_strong` 完全相同。
- 效果也相同，除了 weak 可能虚假失败（spuriously fail）。
- 即可能在实际相等时仍报告失败；但一旦报告成功，则一定相等。
- 因此在某些平台上 weak 可能比 strong 更便宜。
- 通常我们不希望虚假失败，因此 weak 常用于循环。

- 当循环体相对便宜时，weak 可能有利于性能。
- 但若不在循环中使用或循环很贵，应使用 strong。

# 自旋锁（spinlock）

- 虽然可用 `acq_rel` 实现自旋锁，但效率很低。

- 通常应依赖平台相关特性。
- 例如在 x86 上有大量空闲指令（`PAUSE`、`UMWAIT` 等）降低忙等开销。
- 在操作系统层可用许多原生工具，如 Linux 的 `futex`、Windows 的 `WaitOnAddress` 等。
- 为最大化效率，C++20 为原子类型引入 `.wait()`。

关于空闲指令的简明好文：漫话Linux之“躺平”: IDLE 子系统

- `.wait(T old, memory_order)`：当 `.load(memory_order)` 等于 `old` 时阻塞。
- 类似条件变量：
1. 修改后需要调用 `.notify_one()` 与 `.notify_all()` 唤醒等待方；
- 并注意可能的 ABA 问题。
2. 可能虚假唤醒并做比较，即便未被通知。
- 例如，再次：

# 自旋锁（spinlock*）

- 关于 `.wait()` 如何实现，供参考。
- Windows 与 MS-STL：若 Windows SDK 支持则用 `WaitOnAddress`；否则例如用条件变量。
- Linux 与 libstdc++：

内存序包含在 `__vfn` 中。

介绍文章见 Implementing C++20 atomic waiting in libstdc++ | Red Hat Developer。

- 自旋 `__atomic_spin` `_count` 次

- 先“放松”（relax）尝试

- 若为 false 则直接跳过该段

- 例如通过 `PAUSE` 指令

- 通过 `futex`

# 无锁？（Lock-free?）

- 最后，我们通常用原子变量替代锁是为了更高效。
- 但原子变量真的无锁吗？
- 不一定……
- C++ 并不规定原子变量必须 lock-free。
- 常见平台会用 ISA 的原子指令让小类型（如整数）lock-free；
- 但若使用很大的结构体，则没有原子指令能做到！
- 或平台 ISA 很弱，则甚至不是所有原子整数都 lock-free……
- 相反，C++ 提供接口检查是否 lock-free。

对非 lock-free 的原子类型，可能需要链接额外库；如在 gcc 与 clang 中需要 `-latomic`。

# 无锁？（Lock-free?）

- `constexpr static` 布尔：`std::atomic<T>::is_always_lock_free`；仅当当前平台上 `std::atomic<T>` 肯定 lock-free 时为 true。
- 普通成员函数：`.is_lock_free()`；某些 lock-free 类型可能只能在运行时确定（例如地址过度对齐时）。
- C++20 增加若干保证 lock-free 的别名：

# `atomic_flag`

- 此外，C++ 标准规定一种特殊类型必定 lock-free：`std::atomic_flag`。
- 它类似 `std::atomic<bool>`，但后者并未被规定必须 lock-free。
- 由于其值只有 true/false，方法直接改名。
- 读操作：
- `.test(memory_order)`，自 C++20。
- 写操作：
- `.clear(memory_order)`：置为 false。
- RMW：
- `.test_and_set(memory_order)`：置为 true 并返回先前测试结果。
- 自旋锁：
- `.wait(bool old, memory_order)`、`.notify_one()`、`.notify_all()`，自 C++20。

# `atomic_flag`

- 关于构造函数：

C++20 之前

# 并发进阶（Advanced Concurrency）

- 原子变量（Atomic variables）
- 基本操作（Basic operations）
- `atomic_flag`
- 特化（Specializations）
- `atomic_ref`

# 特化（Specializations）

- 某些原子类型被特化以提供更便捷的方法；此处列出。
- 整数：

- 浮点（自 C++20）：

- 以及裸指针。

# 特化（Specializations）

- 它们只是增加常见 RMW 运算符及对应函数重载（以提供内存序）。
- 运算符：`+=`、`-=`、`++`、`--`、`&=`、`|=`、`^=`；
- 但它们不返回 `*this`；除后缀 `++` 外，返回新值（即存储的值）。
- 函数：`fetch_xxx`，即 `fetch_add`/`sub`/`and`/`or`/`xor(T, memory_order)`。
- 并返回旧值。
- 另有两个自 C++26 的函数：`fetch_max`/`min(T, mo)`，写回最大/最小值。
- 浮点只提供 `+=`、`-=`、`add`、`sub`；
- 指针只提供 `+=`、`-=`、`++`、`--`、`add`、`sub`、`max`、`min`；

# 特化（Specializations）

- 注意 1：自 C++20 起还存在 `std::shared_ptr` 与 `std::weak_ptr` 的特化，将在《内存管理》中讨论。

- 注意 2：原子指针并不意味着你对所指对象的原子访问；它只表示指针本身是原子的。
- 因此原子指针没有 `operator*` 与 `operator->`。
- 自 C++20 起引入 `std::atomic_ref` 用于那种原子访问。

# `atomic_ref`

- 示例改编自 Nicolai M. Josuttis《C++20 the Complete Guide》。

- `std::atomic_ref<T>` 的大多数方法与 `std::atomic<T>` 相同，如同直接操作它。
- 故不再列出。

# `atomic_ref`

1. 要表示 const 引用，可用 `std::atomic_ref<const T>`；写操作将被禁用。
- `const std::atomic_ref<T>` 是浅 const；引用本身已 const，该浅 const 无效果。
2. 当对象由 `std::atomic_ref` 访问时，不应再通过普通引用与指针访问，以免数据竞争。
- 当然还需保证被引用对象生命周期不结束（即无悬垂引用）。
- 不同 `std::atomic_ref` 不应重叠。形式化地：

# `atomic_ref`

3. 一些独有成员：
- 数据成员：
- `static constexpr std::size_t required_alignment`，被引用对象应按 `required_alignment` 对齐；否则 UB。
- 方法：
- `.address()`：自 C++26，返回指向被引用对象的指针。
- 拷贝构造：引用与另一 `std::atomic_ref` 同一对象。
- 但不可拷贝赋值。
4. 即便 `std::atomic<T>` 是 lock-free，`std::atomic_ref<T>` 也未必 lock-free；实现不同。

# 并发进阶（Advanced Concurrency）

进阶内存序（Advanced Memory Order）

# 并发进阶（Advanced Concurrency）

- 进阶内存序（Advanced Memory Order）
- 释放序列（Release Sequence）
- 凭空产生问题（Out-of-thin-air Problem）
- 内存模型冲突（Memory Model Conflict）
- 栅栏（Fence）

# 释放序列（Release Sequence）

- 观察下面代码：

# 释放序列（Release Sequence）

- 假设只有一个生产者与一个消费者，则肯定正确。
- 若生产者未就绪，消费者会等待；
- 第一次 `idx > 0` 意味着 #2 从 #1 读到值，于是 SW(#1, #2)。
- 且 SB(#0, #1)、SB(#2, #3)，于是 HB(#0, #3)。
- 即消费者取出值时，保证生产者已存储，从而正确。
- 对后续 `fetch_sub`，因同线程执行，SB 仍保证正确。

- 但若一个生产者 + 两个消费者呢？

# 释放序列（Release Sequence）

- 对首先看到 `idx > 0` 的消费者仍正确（如前分析）。
- 但对第二个消费者：其线程内某次读所见的值可能仅来自同一线程先前的写；若该写使用 `memory_order_relaxed`，则不存在 SW（同一线程上“前一次相关写/后一次相关读”之间的同步），因而无法推出 HB(#0, #3)……
- 即第二个消费者取出值时，并不保证生产者已存储。
- 解决：用 `acq_rel` 代替 `acquire`；
- 但我们说过 `acquire` 只引入单向屏障，而 `acq_rel` 引入双向屏障，损害优化。

# 释放序列（Release Sequence）

- 为克服这种反直觉结果，C++ 引入 release sequence。

[intro.multithread]

[atomics]

- 换言之，release 操作可通过一串连续的 RMW 操作（无论其内存序）延续，只要没有新的修改插入打断。
- 在我们的例子里，这意味着 SW(#1, #2) 因而得到保证。

# 释放序列（Release Sequence）

- 这比 `acq_rel` 略弱。
- 若用 `acq_rel`，可推出 SW(#2, #2) 并进一步 HB(#1, #2)。
- 但通过 release sequence，只能推出 SW(#1, #2)；无法推出消费者线程内两次关键操作之间的 HB（原稿以两个 #2 表示同一线程上的不同步骤）。
- 再用之前的例子：能否把这里的序弱化到 `relaxed`？

# 释放序列（Release Sequence）

- 可以，#1 + #2 构成 release sequence，于是 SW(#1, #4)。
- 结合 SB(#0, #1) 与 SB(#4, #5)，可知 HB(#0, #5)，断言永不触发。
- 但右侧代码不正确：
- 因为没有 SW(#1, #2)，不知 HB(#1, #6)，也就没有 HB(#0, #6)。
- 两个无 HB 关系的非原子操作会造成数据竞争。
- 因此 #6 是 UB。
- 若此处用 `acq_rel`，由 SW(#1, #2) 知 HB(#0, #6)，则正确。

# C++20 之前的释放序列（Release Sequence before C++20*）

- 本段可选。
- 其实 C++20 之前 release sequence 可包含更多成分：

- C++20 削弱了 release sequence；但为何 C++11 引入而 C++20 删除部分规则？
- 考虑右侧代码：

# C++20 之前的释放序列（Release Sequence before C++20*）

- 要执行 #5，#4 需为 true，即 #4 需从 #3 加载到值。
- 但 #3 是 relaxed store，无 SW 关系，无法推出 HB(#1, #5)。
- 于是这里 #5 与 #1 数据竞争。
- 但直观上由于 SB(#2, #3) 且 #2 是 release，会“自然”认为 SW(#2, #4)。
- 因此 C++11 规定同线程后续写也属于 release sequence。

[1, 2]
- 然而这并不自然……

[1]：Common Compiler Optimisations are Invalid in the C11 Memory Model and what we can do about it | POPL’15，Viktor 等。[2]：P0982R1: Weaken release sequences

# C++20 之前的释放序列（Release Sequence before C++20*）

- 可引入新线程：
- 若 #6 在 `x` 的修改顺序上插入 #2 与 #3 之间……
- 则该 release sequence 被破坏，#5 突然又与 #1 数据竞争。
- 再次反直觉，会让程序出错。
- 即本不该参与任何 SW 关系的 relaxed 序，奇怪地破坏了其他 HB 关系。

# C++20 之前的释放序列（Release Sequence before C++20*）

- 两种解决思路：
- [1] 作为学术论文提出很复杂的方法强化内存模型以使 release sequence 仍成立；
- [2] 作为 C++ 提案提出最小改动，通过取消第一条规则来削弱 release sequence。

- 这意味着即便没有线程 3，代码也可能数据竞争（如前所述 HB(#1, #5) 不成立）。

[1]：Common Compiler Optimisations are Invalid in the C11 Memory Model and what we can do about it | POPL’15，Viktor 等。[2]：P0982R1: Weaken release sequences

# 并发进阶（Advanced Concurrency）

- 进阶内存序（Advanced Memory Order）
- 释放序列（Release Sequence）
- 凭空产生问题（Out-of-thin-air Problem）
- 内存模型冲突（Memory Model Conflict）
- 栅栏（Fence）

# 凭空产生问题（Out-of-thin-air Problem）

- 我们说过宽松模型可产生惊人结果：
- 这里 `r1 == 42 && r2 == 42` 可能。
- 下面代码呢？

- 我们只加了两个 `if`，没有新的原子操作，理论上不应影响任何 HB 顺序，`r1 == 42 && r2 == 42` 仍是理论合法解。

# 凭空产生问题（Out-of-thin-air Problem）

- 推理过程与之前相同：

- 然而该结果与逻辑因果律矛盾。

# 凭空产生问题（Out-of-thin-air Problem）

- 逻辑前提如下（→ 表示“需要”）：
- #2 的 store 发生 → `r1 == 42` → #1 加载 42 → #4 的 store 发生 → `r2 == 42` → #3 加载 42 → #2 的 store 发生。
- 于是使 #2 发生的前提，是 #2 已经发生。
- 这是逻辑谬误，即循环论证（通过假设结论正确来推出结论正确）。
- 对比普通例子：
- #2 的 store 发生没有前提。

- 宽松序中这种逻辑谬误称为“凭空产生问题”（out-of-thin-air problem）；`r1 == 42 && r2 == 42` 凭空而来却被理论模型允许。

# 凭空产生问题（Out-of-thin-air Problem）

- 若允许它发生，可怕例子也成立：
- 初值 `x == 0 && y == 0`，仍可得 `r1 == 42 && r2 == 42`。
- 因为可以“假设” `r1` 加载到 42，然后发现 `x == 42 && y == 42 && r1 == 42 && r2 == 42` 是合法一致解。
- 再次循环论证……
- 然而这些问题学术界仍在研究：
1. 如何在当前模型中描述凭空产生问题？
2. 编译器如何检测凭空产生问题？
- 目前只能靠数据依赖描述，在复杂程序中几乎不可追踪。
3. 如何以最高效方式避免凭空产生问题？

- 论文 *An Initial Study of Two Approaches to Eliminating Out-of-Thin-Air Results* 是这方面的好综述。

- 当然大量学术工作尝试不同途径解决……
- 在广泛接受的模型与描述出现前，C++ 标准选择最保守的表述：

由于凭空产生问题尚未被良好定义，我们仅断言不会有处理器执行违反因果性的操作。

# 并发进阶（Advanced Concurrency）

- 进阶内存序（Advanced Memory Order）
- 释放序列（Release Sequence）
- 凭空产生问题（Out-of-thin-air Problem）
- 内存模型冲突（Memory Model Conflict）
- 栅栏（Fence）

# 内存模型冲突（Memory Model Conflict）

- 我们说过顺序一致模型保证全序，而 acquire-release 与宽松模型不保证。
- 若混合这些操作呢？[1, 2]
- 例如：

- `x` 与 `y` 初值均为 0，`r1 == 1 && r2 == 3 && r3 == 0` 可能吗？

[1]：Repairing sequential consistency in C/C++11 | PLDI’17，Lahav 等。[2]：P0668R5: Revising the C++ memory model

# 内存模型冲突（Memory Model Conflict）

- 在 `seq_cst` 全序中：
- 要 `r1 == 1 && r2 == 3`，C 需读到 `y == 1` 而 D 需读到 `y == 3`，于是全序中 C → E。
- 若 E → C，则 D 永远得不到 3。
- B 不是 `seq_cst` 操作，故 B → C 不在全序之中。
- 要 `r3 == 0`，F 需读到 `x == 0`，于是全序 F → A。
- 且 SB 约束全序 E → F。
- 故在 `seq_cst` 模型中，该结果只需全序 C → E → F → A。

- 而在 HB 关系中……
- 首先注意在 SW 关系中，`seq_cst` 等价于 `acq_rel`。

# 内存模型冲突（Memory Model Conflict）

- 我们只知道：
- SW(Init_x, F)、SB(A, B)、SB(C, D)、SB(E, F)。
- 要 `r1 == 1`，C 从 B 读到值，于是 SW(B, C)。
- 于是 HB(A, B, C, D)。
- 要 `r2 == 3`，只要 HB(D, E) 不成立。
- 且无法推出其必成立，于是 Okay。
- 于是从不同视角似乎得到矛盾结果：
- 全序中 C → E → F → A；
- HB 顺序中 HB(A, B, C, D)。
- C++20 之前规定 HB 应与全序一致，即 `r1 == 1 && r2 == 3 && r3 == 0` 不可能。

# 内存模型冲突（Memory Model Conflict）

- 然而 Power 与 ARM 允许它（尤其 Power）……
- 为最大化优化，C++20 标准修订为允许这种矛盾，而非只修编译器。
- 形式化地，只有 strongly-happens-before 关系才需服从全序。

- 在我们的例子里，SW(B, C) 并非全是 `seq_cst` 原子操作，于是 SHB(A, B, C, D) 不成立。
- 只能断言 SHB(A, D)，因为 SB(A, B) && HB(B, C) && SB(C, D)；但 D 不是 `seq_cst`，SHB(A, D) 仍可能不参与全序。

# Happens-before 的修订（Happens-before Revision*）

- 原幻灯片为示意表格，此处用列表对应标准演进（术语与括号内英文与标准一致）：
- **C++11**：定义 **happens-before**；**consume** 操作存在。
- **C++20**：引入 **simply-happens-before**；在不涉及 consume 时可与 happens-before 一并讨论；**seq_cst** 全序与 HB 的一致性要求被修订，引入 **strongly-happens-before（SHB）** 以描述必须与全序一致的部分；**release sequence** 规则被削弱。
- **C++26**：**consume** 操作被弃用（deprecated）；教学上默认采用 **happens-before** 表述；在混合不同内存序时，需关注 **SHB** 是否必须服从 **total order**（例如 Power 等架构上的可见行为）。

| 概念 | 与全序（total order）关系（要点） |
|------|-----------------------------------|
| happens-before / simply-happens-before | C++20 前常要求与 `seq_cst` 全序一致；C++20 起由 SHB 分担“必须服从全序”的部分 |
| strongly-happens-before | 在混合 `seq_cst` 与非 `seq_cst` 时，用于判定哪些边必须进入全序约束 |

本课程基于 C++26 表述；由于 consume 从未被主流实现真正提供，也可在实践上等价地按 C++20 之后模型理解。

# 并发进阶（Advanced Concurrency）

- 进阶内存序（Advanced Memory Order）
- 释放序列（Release Sequence）
- 凭空产生问题（Out-of-thin-air Problem）
- 内存模型冲突（Memory Model Conflict）
- 栅栏（Fence）

# 栅栏（Fence）

- 有时我们希望在没有显式原子变量的情况下同步……
- 栅栏作为全局屏障正为此存在！
- `std::atomic_thread_fence(memory_order)`。
- 它在某种程度上全局施加内存序：
- 对 release fence，如同为后续原子写附加 release 序。
- 但关系只是来自 fence 的 SW。
- 对 acquire fence，如同为之前的原子读附加 acquire 序。
- 但关系只是来自 fence 的 SW。
- 具体地：

# 栅栏（Fence）

- 例如：
- 当 #3 为 true，它从 #2 读到值；
- 我们说 #3 “如同” acquire 操作，因为它是 acquire fence 之前的原子读……
- 于是建立 SW。
- 且 SW 从 fence 开始……
- 因而是 SW(#2, #4)，不是 SW(#2, #3)。
- 于是 HB(#1, #5)，该读安全。

# 栅栏（Fence）

- 若交换 #4 与 #5 还对吗？
- 不对，因为 SW(#2, #4) 不再蕴含 HB(#1, #5)。
- 可见栅栏在全局强化原子操作，也会带来更高开销。
- 形式化地：

# 栅栏（Fence）

- 另一例（幻灯片以 #1…#7 标注步骤；此处保留编号以便对照）：
- 若 #6 为 true，则 #4 从 #3 读到值；
- 且 #4 是 acquire fence 之前的原子读，#3 是 release fence 之后的原子写；
- 于是“如同” #3 是 release、#4 是 acquire；
- 因而从 fence 建立 SW；
- 然后 SW(#2, #5)，于是 HB(#1, #7)，故 #7 读取安全。

# 并发进阶（Advanced Concurrency）

协程（Coroutine）

# 并发进阶（Advanced Concurrency）

- 协程（Coroutine）
- 概览（Overview）
- 基础（Basics）
- 等待体（Awaiter）
- `std::generator` 详解（std::generator in Detail）

# 协程（Coroutine）

- 我们知道线程是竞争性执行上下文。
- 除非调度器通过中断强制暂停，否则它们不会挂起。

- 但有时指令流彼此协作。
- 即它们自愿挂起并切换到其他流。
- 这就是协程（cooperative routine）。
- 协程定义一组挂起点；
- 当执行流到达挂起点，会把权利交给其他协程；
- 当其他协程把权利交回时恢复执行。
- 就像状态机！

# 协程类别（Coroutine category）

- 实现协程有两种方式…… *
- 有栈协程（stackful coroutine），或称 fiber（有栈协程 / 纤程）。
- 回顾：线程模型。
- 每个线程有自己的寄存器与栈；上下文切换在内核空间保存旧寄存器并加载新寄存器。
- 类似地，有栈协程有自己的寄存器与栈。
- 挂起点只是在用户空间自愿保存与加载寄存器。
- 无栈协程（stackless coroutine）（无栈协程）。
- 状态显式分配在某处；
- 因此寄存器不代表其状态，无需保存。
- 它们只占用当前线程的栈，而不是分配新栈。

*：有时 fiber 指用户空间线程，那仍是竞争性而非协作式。

# 协程（Coroutine）

- 很朴素的例子（伪代码）。

- 对有栈协程：
- `coro` 就是 `Context`；
- Resume：恢复指令指针、栈指针、寄存器。
- Suspend：设置结果，保存指令指针、栈指针、寄存器。

# 协程（Coroutine）

- 对无栈协程，可无汇编重写。
- 任何生命周期跨越挂起点的变量会保存在上下文中。
- 就像普通函数调用，因此使用调用者的栈。

- C++ 采用无栈协程以实现零开销抽象。
- 但也存在 P0876（fiber）提案。

# 并发进阶（Advanced Concurrency）

- 协程（Coroutine）
- 概览（Overview）
- 基础（Basics）
- 等待体（Awaiter）
- `std::generator` 详解（std::generator in Detail）

协程接口（Coroutine interface）

# 协程（Coroutine）

挂起点（Suspension point）

- 在 C++ 中使用协程需要定义含 `co_await`、`co_yield` 或 `co_return` 的函数。此处可省略 `co_return;`。

- 对调用者，它从协程获得协程接口。
- 协程接口需封装底层细节，并提供操纵协程的 API。

先声明以便在模板参数中使用。

# 协程（Coroutine）

- 协程接口如何定义？
- 应有数据成员 `std::coroutine_handle`（定义于 `<coroutine>`），已表示协程的高层抽象。
- 然后需定义 `promise_type`，定制协程行为。
- 粗略地说，协程会被翻译成：

先把协程参数拷贝到协程帧（coroutine frame）。

# 协程（Coroutine）

- `std::coroutine_handle` 暴露若干操作：

协程帧上的 promise 可用来取得 handle，反之亦然。

等价于

# 协程（Coroutine）

- 然后，协程函数体被视为如下：

逐步看发生了什么……

# 协程（Coroutine）

1. 这里协程会立即停止，在用户自定义代码执行之前。
2. 若用户代码抛异常，将调用 `.unhandled_exception()`。
3. 用户代码 `co_return` 时执行，这里协程在最终点挂起。
- 必须是 `noexcept`。
4. `co_return;` 不返回任何值，因此 promise 需定义 `return_void`。

# 协程（Coroutine）

- 最后，需在协程接口中暴露 handle 的 API：

# 协程（Coroutine）

1. 除 `std::suspend_always` 外，有时也用 `std::suspend_never`。
- `co_await std::suspend_never{}` 表示“不挂起，继续执行”；
- 例如：

创建协程时立即恢复。

suspend_always suspend_never

# 协程（Coroutine）

2. `.final_suspend()` 几乎总应挂起，因此这里很少用 `std::suspend_never`。
- 若 `final_suspend` 不挂起，协程帧会自动销毁并返回调用者；因此并非内存泄漏。
- 原因 1：`coroutine_handle` 与 promise 在协程帧释放后会非法……
- 因此之后用 `.done()` 检查是否可恢复是 UB，访问 promise 数据成员也是 UB。
- 同样，`.destroy()` 会导致双重释放 UB，难以封装进协程接口。
- 就像悬垂指针。
- 原因 2：若协程生命周期嵌套在调用者内，编译器可能做堆分配省略优化（HALO）。
- 在最终点挂起让编译器更容易判断 HALO。

# 协程（Coroutine）

3. 即便在 `.final_suspend()` 挂起，再次调用 `.resume()` 也是 UB。
- 在非挂起协程上 `resume` 或 `destroy`（例如在另一线程调用 `.resume()`）也是 UB。
4. 若要 `co_return` 某值，可定义 `void return_value`。

协程接口（Coroutine interface）

`return_void` 与 `return_value` 的返回类型必须是 `void`。

# 协程（Coroutine）

- 注意 1：可为 `return_value` 定义重载，甚至模板化，从而 `co_return` 不同类型。
- 例如：

- 注意 2：不能同时定义 `return_void` 与 `return_value`。
- 且 `co_return;` 只解释为 `p.return_void()`，因此定义不接受参数的 `return_value` 也无用。
- 注意 3：协程中不能使用 `return`。

[dcl.fct.def.coroutine], [stmt.return.coroutine].

# 协程（Coroutine）

5. 实现 `.unhandled_exception()` 的常见做法：
- 忽略异常；
- 在本地处理异常；例如：

在 `catch` 块中 `rethrow`。

- 结束或中止程序（例如 `std::terminate()`）；
- 用 `std::current_exception()` 存储异常供后用。

# 协程（Coroutine）

6. `promise_type` 实际由 `std::coroutine_traits<R, Args…>` 查找。
- 如本例，由 `std::coroutine_traits<Task, int>` 找到。
- 默认只用 `R::promise_type`，因此需在 `Task` 内定义 `promise_type`。
7. 除默认初始化外，promise 也可用协程参数构造。
- 如本例，

- 这对分配器参数有用（以后再讲）。

极少数情况下可特化 `std::coroutine_traits` 以通过参数传入协程接口并在别处定义 `promise_type`。仍需保持 `.get_return_object()` 的返回类型可转换为协程返回类型。此处不展开，因不太常用。

# 协程（Coroutine）

8. 若不想在协程帧分配失败时抛异常，可额外定义 `static CoroutineInterface get_return_object_on_allocation_failure()`；
- 于是 `new` 会使用 `std::nothrow_t`……

- 例如：

# 协程（Coroutine）

9. 关于 `std::coroutine_handle` 的细节……
- 很像指向协程帧的裸指针。
- 默认构造 / `nullptr` 构造：handle 不代表协程；
- 拷贝与移动构造 / 赋值：浅拷贝；副本指向同一协程；
- 与地址相关：

- 可比较、可哈希。
- 最后，`std::coroutine_handle<void>` 特化为表示任意协程帧。
- 因擦除了 promise 类型，未定义 `.promise()` 与 `.from_promise()`。
- 对通用执行有用，因为存在 `.resume()`。

# 协程（Coroutine）

- 但我们现在还只实现了挂起……
- 如何实现开头的伪代码？
- 要产出值，需要：
- 协程中 `co_yield xxx;`；
- `promise_type` 中 `Awaiter yield_value(Type xxx)`。
- 常见 `Awaiter` 就是 `std::suspend_always`，表示 `co_yield` 总会挂起协程。
- 类似 `.return_value()`，可重载 / 模板化。
- 以及接口：

# 协程（Coroutine）

- 总之，`promise_type` 需要五个必备操作：
- `.get_return_object() -> CoroutineInterface`；
- `.initial_suspend() -> Awaiter`，指定执行用户代码前的初始行为；
- `.final_suspend() -> Awaiter`，指定执行用户代码后的最终行为；
- `.return_void`/`return_value() -> void`，支持 `co_return`；
- `.unhandled_exception() -> void`，处理协程内抛出的异常；
- 以及一些可选操作：
- `.yield_value()`，支持 `co_yield`；
- `static get_return_object_on_allocation_failure()-> CoroutineInterface`，在协程帧分配失败时提供默认协程接口对象而非抛异常。

协程接口的最小示例。

也可添加例如 `.yield_value()`，并在协程接口中添加 API（在 `promise_type` 定义之后，因使用其成员需要完整类型）。

# 协程（Coroutine）

- 练习：实现一个非常简单的 `std::generator`。
- 我们在第 4 讲 Ranges & Algorithms 中简要讲过 `std::generator`……
- 因此只需加 `.yield_value()` 与迭代器！
- 由于无法确定尾迭代器，使用 `std::default_sentinel_t`。

平凡部分……

这里我们：
1. 在 promise 中存储值并在 `.yield_value()` 中转发赋值；
2. 在 `operator*` 中返回其引用；

`std::generator` 的实际规范与实现更复杂，稍后详述。

# 协程生命周期注意（Coroutine Lifetime Concern）

- 最后非常重要：协程参数应很少使用引用。
- 例如：
- 对普通函数，绑定到 `s` 的临时对象何时析构？
- 临时对象在语句结束、即 `;` 之后析构。
- 通常参数使用在函数返回时结束，因此传临时完全没问题。
- 但对协程……

# 协程生命周期注意（Coroutine Lifetime Concern）

- C++ 中的协程仍是函数！
- 编译器只是把协程代码“变换”成另一个普通函数，如我们的 `switch + goto` 代码。
- 于是协程里的 `for` 循环，在第一次挂起之后……
- `s` 成为悬垂引用，后续 `.resume()` 错误。

- 对 lambda 协程该问题更隐蔽。
- 下面代码对吗？

# 协程生命周期注意（Coroutine Lifetime Concern）

- 不对！
- 我们知道 lambda 捕获与 lambda 本身同生命周期。
- 且 `getCoro()` 创建临时 lambda，传入 3 生成协程接口 `task`。
- 临时 lambda 析构，捕获失效，于是协程代码 UB。
- 正确写法颇为奇怪：
- 总之，除非仔细考虑过：
1. 协程参数不要用引用类型；
2. 不要用带状态的 lambda 协程，或一般要特别留意 public member coroutine。

# 并发进阶（Advanced Concurrency）

- 协程（Coroutine）
- 概览（Overview）
- 基础（Basics）
- 等待体（Awaiter）
- 对称转移（Symmetric Transfer）
- `std::generator` 详解（std::generator in Detail）

# 等待体（Awaiter）

- 此前我们只用两种预定义类型来 `await`：`std::suspend_always` 或 `std::suspend_never`。
- 但它们如何挂起 / 继续？
- 一般而言可定义 awaiter 以操纵后续协程行为。
- 对 awaiter，需定义三个 API：
- `.await_ready() -> bool`：`false` 表示后续未就绪，即挂起；`true` 表示不挂起；
- `.await_suspend(CoroutineHandle continuation) -> void`：挂起时调用，决定后续执行应做什么；
- `.await_resume() -> T`：当前挂起恢复时，在后续执行之前应先执行什么。`T` 即 `co_await` 的返回值类型。

# 等待体（Awaiter）

- 其实预定义版本很容易实现：`suspend_never` 在这里只返回 true。

无需额外操作。只需挂起。

无需在此处理；`handle.resume()` 将在恢复后继续执行，包括调用 `.await_resume()` 与销毁临时 awaiter。

# 等待体（Awaiter）

- 例如实现 `NaiveTaskCont`：
- 当我们 `await` 另一个任务时，希望下一次 `.Resume()` 恢复新任务。

可用链表走到最深的任务。基础部分完全相同。

在 `co_await x;` 中我们调用 `x.await_suspend()`，需要把新任务 `x` 设为当前协程的下一层 handle。

每次调用 `.Resume()`，通过链表找到最深任务并恢复它。

1. 调用 `Test()`，创建协程接口 A 并在初始点挂起；
2. `Resume()`，执行到 `co_await Test2()`；
- 调用 `Test2()`，创建新接口 B 并在初始点挂起；
- `B.await_ready()` 返回 false，表示 A 需要挂起；
- `B.await_suspend(cont)` 把 B 的 handle（`this->coroHandle_`）挂到 A 的 handle（`cont`）下一层。
3. `Resume()`，找到最深任务（B）并恢复，执行到 `suspend_always`。
4. `Resume()`，执行直到 B 到达最终点。
5. `Resume()`，执行直到 A 到达最终点。
- 先调用 `B.await_resume()`；
- `B` 析构，因为它是 `Test2()` 返回的临时对象。
- 随后执行诸如打印输出的后续代码。

# 等待体（Awaiter）

- 除直接定义 awaiter API 外，还有另外两种 `co_await` 方式：
1. 通过 `operator co_await`，把 `co_await` 委托给返回值。
- 很像 `operator->`！
- 可把 awaiter API 隔离，不在协程接口中暴露给用户。

# 等待体（Awaiter）

2. 通过 promise 中的 `.await_transform()`，会拦截几乎所有 `co_await x;` 并替换为 `co_await promise.await_transform(x)`。

- 也可加约束或重载；
- 若要在协程中禁用 `co_await`，可用 `=delete`。
- 如 `std::generator`：
- 顺序：对 `co_await x`，若 `p.await_transform()` 失败，则 `x.operator co_await`；若再失败，则查找 `x` 的 awaiter API。

# 等待体（Awaiter）

- 回顾 `co_yield x`……
- 它本质上等价于 `co_await promise.yield_value(x)`。
- 特别地，初始 await、最终 await 与 `co_yield` 会绕过 `.await_transform()`。
- 例如若要做来回转换：

# 等待体（Awaiter）

- `.await_resume()` 的返回值视为 `co_await` 的返回值，因而也是 `co_yield` 的返回值。

- 不太有用，只是朴素示例。

# 并发进阶（Advanced Concurrency）

- 协程（Coroutine）
- 概览（Overview）
- 基础（Basics）
- 等待体（Awaiter）
- 对称转移（Symmetric Transfer）
- `std::generator` 详解（std::generator in Detail）

# 对称转移（Symmetric Transfer）

- 此前程序虽可挂起恢复，但仍是顺序执行。
- 许多情况下协程会与多线程配合，从而并行执行不同流。
- 考虑常见情形：
- 有 `ServiceA` 与 `ServiceB`，`ServiceA` 的结果会送给 `ServiceB`。
- 例如 `ServiceA` 是网络接收，`ServiceB` 是数据处理。
- 对同步代码，可能直接写：

# 对称转移（Symmetric Transfer）

- 更一般地，可用回调：
- 于是我们把 `DoServiceB` 视为 `DoServiceA` 的后续执行。
- 当然也可用协程自动完成：

当前任务结束时自动恢复原始任务。

任务被 await 时附着执行流并恢复新任务。

任务只能附着一次，然后会执行到结束。用右值引用限定符提醒用户。

整个过程如：
1. `DoServiceB()` 创建协程接口 B；
2. `.Start()` 恢复它直到 `co_await`；
3. `DoServiceA()` 创建协程接口 A；
4. `awaiter.await_suspend()` 恢复 A；
5. A 到达 `co_return`；
6. `final_awaiter.await_suspend()` 恢复 B；
7. B 到达 `co_return`；
8. `B.resume()` 返回 -> `final_awaiter.await_suspend()` 返回 -> `A.resume()` 返回 -> `awaiter.await_suspend()` 返回。
9. `Task.Start()` 返回，所有指令执行完毕。

# 对称转移（Symmetric Transfer）

- 继续之前，小讨论：
- 似乎完全不必用协程……
- 然而可用 awaiter 轻易把执行转移到另一线程：

- 于是自 `DoServiceAAsync()` 起的所有指令流在线程池的另一线程执行，主线程可继续不等。
- 借助协程，在不同调度器间切换执行很容易。

然而在转移前后要特别注意同步问题。特别地，互斥锁不能在 A 线程上锁却在 B 线程解锁（协程里很容易写出这种代码！）。

# 对称转移（Symmetric Transfer）

- 然而这种流拼接有严重问题……
- 考虑另一简单代码段：

- 当 `count` 足够大时会导致栈溢出。

- 再分析整个过程……
- 每次循环栈占用类似：
- `completes_synchronously()` 创建新协程；协程帧生成会暂时占用栈。
- 该函数返回时栈会弹出。

# 对称转移（Symmetric Transfer）

- 其他部分确实占用栈：
1. `awaiter.await_suspend()`；
2. `newTaskHandle.resume()`；
3. 新任务执行自身代码直到 `co_return`。无栈协程占用调用者栈，就像函数调用。
4. `final_awaiter.await_suspend()`；
5. `lastTaskHandle.resume()`；
6. 循环继续执行，再次占用栈。
- 只有循环结束栈才全部弹出。
- 但我们希望循环执行而不压栈……
- 核心原因：`.resume` 的递归调用。

# 对称转移（Symmetric Transfer）

- 为解决该问题，协程还支持对称转移（symmetric transfer）。
- 即 `.await_suspend()` 可返回协程 handle，表示继续执行该 handle。
- 相比在 `.await_suspend()` 内直接调用 `.resume()`，对称转移会先弹出栈。
- 在前面的分析中，这意味着：
- 2、3、4 会在 5 之前弹出；
- 5、6 与下一次 1 会在下一次 2 之前弹出。
- 这优雅地利用尾调用优化，此处不深入。

若感兴趣，强烈推荐阅读 Lewis Baker 的 C++ Coroutines: Understanding Symmetric Transfer。

并注意 `gcc` 在此处尚未实现相关尾调用优化（见其 Bugzilla 跟踪项），对称转移仍可能导致栈溢出。

# 对称转移（Symmetric Transfer）

- 若像上面的代码那样缺少对空指针的判断……
- 仍存在一个小问题……
- 当 `lastLevelHandle` 为 `nullptr` 时我们在恢复 `nullptr`，于是 UB。
- 要像 `void` 一样返回调用者，可用 `std::noop_coroutine()`：

- 它返回 `std::coroutine_handle<std::noop_coroutine_promise>`，另一种特化 handle，在 `resume` 时直接返回调用者。

# 对称转移（Symmetric Transfer）

- 最后，当 `.await_suspend()` 只有两种选择……
- 要么挂起，要么继续执行；
- 则也可用返回 `bool` 的变体；例如：

- 注意 1：可用返回 handle 的变体改写，但返回 `bool` 的变体更简单，编译器更可能优化得更好。

# 对称转移（Symmetric Transfer）

- 注意 2：相比 `.await_ready()`，协程在 `.await_suspend()` 时已经挂起。
- 若在 `.await_ready()` 返回 true，`.await_resume()` 不会被调用；
- 但若在 `.await_suspend()` 返回 false，`.await_resume()` 会被调用。
- 注意 3：所有 awaiter API 与 `promise_type` API 都可为 `constexpr`，如同普通方法。
- 然而协程目前不能是 `constexpr`。
- 未来可能由 P3367R4: constexpr coroutines 增强。

# 并发进阶（Advanced Concurrency）

- 协程（Coroutine）
- 概览（Overview）
- 基础（Basics）
- 等待体（Awaiter）
- `std::generator` 详解（std::generator in Detail）

# 生成器（Generator）

- 学完协程后看看 `std::generator` 真正做什么！
1. promise 类型存指向产出值的指针，而不是存值再赋值。
- 首先看 `T` 是值类型还是右值引用。

- 我们知道它被变换为：

# 生成器（Generator）

- 于是似乎存了指向临时的指针……
- 我们知道普通函数里这样做很危险。
- 类似例子来自第 5 讲《生命周期与类型安全》的作业：

- 但这里是安全的！
- 所有临时对象在整个语句（即到 `;`）结束时析构。
- 那 `co_await` 何时结束？
- 协程再次恢复、调用 `.await_resume()` 之后！

# 生成器（Generator）

- 因此临时对象持续到下一次恢复，使得存指针并解引用安全。
- 相比移动赋值，省一次移动。
- 于是迭代器只返回右值引用：

- 等等，产出左值怎么办？
- 这里只接受右值引用……
- 若用户产出左值，不应在 `operator*` 里像右值那样移动它。

# 生成器（Generator）

- `std::generator` 为 `const T&` 增加重载，把它拷贝到 awaiter 再存指针。
- 迭代器 `operator*` 仍返回右值引用，但引用的是那份拷贝。

# 生成器（Generator）

- 对 `std::generator<T>`，其中 `T` 为左值引用……
- 则只存指针并按对应引用返回。
- 总之 `T` 只表示如何引用对象：

# 生成器（Generator）

- 此外生成器还支持用 `std::generator<Ref, Value>` 指定值类型 + 引用类型。
- 因此 `std::generator<Ref>` 实际把 `Value` 设为 `void`，并从 `Ref` 推导真实值类型。

- 例如若希望 `operator*` 返回值类型：
- 但 promise 仍存指针，并在 `.yield_value()` 中接受 `const&`。

# 生成器（Generator）

- 也可用于代理引用类型。
- 如 `std::vector<bool>::reference` 作为位代理。

但说实话 `std::generator<Ref, Value>` 很少用。

`std::generator` 也接受分配器作为第三个类型参数；下一讲再讲。

# 生成器（Generator）

- 最后，`std::generator` 还支持递归产出（recursive yield）。
- 即把恢复委托给另一生成器；委托方结束后继续执行自身。
- 为区分 yield 与委托，需要 `stdr::elements_of` 作为标签。
- 例如二叉树中序遍历：

其实可委托给任意 range，等价于循环并产出其元素。但目前可能有些缺陷，会作为 DR 提交。

# 生成器（Generator）

- 注意：`.unhandled_exception()` 会逐级传播异常，因此可在任意层捕获。
- 例如：

若此处不捕获，会传播给调用者。

通过存储 `exception_ptr` 并在委托返回的 `.await_resume()` 中检查来实现。

# 小结（Summary）

- 内存序基础 • 凭空产生问题
- happens-before、synchronizes-with • strongly-happens-before • 栅栏（Fence）
- 顺序一致（Sequential Consistent）
- 协程（Coroutine）
- 获取-释放（Acquire-Release）
- 有栈与无栈协程；
- 宽松（Relaxed）
- 基础：`promise_type`；
- 原子变量 • 等待体（Awaiter）与对称转移；
- 读、写、RMW、自旋锁 • `std::generator`
- 特化、`atomic_flag`、`atomic_ref`
- 进阶内存序
- 释放序列（Release Sequence）

# 下一讲……（Next lecture…）

- 我们将讨论内存管理……
- 智能指针；
- 分配器（Allocator）；
- PMR；
- ……
