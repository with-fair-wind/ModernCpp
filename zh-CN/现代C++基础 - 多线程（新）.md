多线程 Multithreading

## 现代C++基础

## Modern C++ Basics

Jiaming Liang，北京大学本科生

### • 线程（Thread）

### • 同步工具（Synchronization utilities）

### • 异步操作的高层抽象（High-level Abstraction of Asynchronous Operation）

# 多线程（Multithreading）

线程（Thread）

# 多线程（Multithreading）

- 线程（Thread）
- 抽象线程模型（Abstract thread model）
- `thread`
- `jthread`
- 杂项主题（Miscellaneous topics）

# 线程模型（Thread model）

- 我们首先简要回顾什么是线程……
- 我们在 ICS 中学过，每个程序是一个进程；
- 它有独立的地址空间，以及可能还有文件描述符等其他状态（取决于操作系统）。
- 隔离性好、保护性好，访问另一进程内存的能力非常有限。
- 线程：保护更少，数据共享更方便！
- 它们仍部分保留自己的一套资源（例如寄存器）……
- 但位于同一虚拟地址空间中，因此很容易访问其他线程的内存！
- 通常，操作系统会调度线程而不是进程；
- 因此在某种程度上可以说，线程是利用多核并行性的最小单位。顺便一提：C++ 标准事实上没有“进程”的概念，因为它规范的一切都发生在单个程序内；你需要依赖操作系统的 API 来操作进程，例如 Linux 上的 `fork`/`exec`，或诸如 Boost.Process 之类的外部库。

# 线程模型（Thread model）

- 简而言之：

粗略地说，若有两个物理核心，实际上只有两个线程在同时执行；但操作系统通过调度制造出多于两个线程在并发运行的假象。

致谢：北京大学操作系统课程金鑫教授。这里对线程只给出非常粗略的理解；你需要在操作系统课程中系统学习。

# 线程模型（Thread model）

- 调度事实上就是暂停正在运行的线程，然后运行处于就绪状态的线程。
- 调度算法决定暂停哪一个、运行哪一个。
- 上下文切换期间会保存并恢复寄存器。
- 线程彼此竞争以获得执行机会！
- 因此，你可能会认为语句可能以任意顺序执行，从而导致 [1] 数据竞争与同步问题。

- 线程创建后需要 `join` 或 `detach`；前者会等到线程函数退出，后者则让它独立自由地执行。

[1]：我们将在《并发进阶》中给出数据竞争的严格定义。

# 多线程（Multithreading）

- 线程（Thread）
- 计算机中的线程模型（Thread model in computers）
- `thread`
- `jthread`
- 杂项主题（Miscellaneous topics）

# 线程（Thread）

- 我们在 ICS 中学过如何在 POSIX 系统上使用 `pthread`。
- `pthread_create`/`pthread_join`/……，像这样：
- 晦涩的 C 接口……
1. 线程函数应为 `void* func(void*);`
2. 参数被打包在 `void*` 中传入。
- 你需要在线程函数内部解包，例如这里的 `(int)`。
3. 返回值由 `void*` 接收。
- 这里我们 `pthread_join(…, NULL)`，即不需要返回值。
- 若需要，再次解包……
- 非常奇怪……能在 C++ 中改进吗？

# 线程（Thread）

- 当然可以！我们可以这样写：
- 非常直观、非常简单，通过定义在 `<thread>` 中的 `std::thread`。
1. 不限于函数，可以使用任意函数对象（functor）。
- 等价于调用 `std::invoke`，因此也可以传入成员函数指针并带上 `this`，例如 `{ &SomeClass::MemberFunc, this, params… }`；
- 函数对象会被复制；你可以显式 `std::move(functor)` 以移动它，或使用 `std::ref` 使其成为 `std::reference_wrapper`。
2. 可以调用 `.detach()` 来分离线程。
- 在 `join`/`detach` 之后，底层线程不再与 `std::thread` 对象关联。除非你向其中移入一个新对象，否则该 `std::thread` 对象处于空状态（如同默认构造/被移动后）。
3. `func` 的返回值会被忽略；应通过引用参数传出。

# 线程（Thread）

- 以及其他一些 API：
1. 移动构造函数、移动赋值，以及可交换（通过 `.swap()` 或 `std::swap`）。
2. `.joinable()` -> `bool`：线程是否处于非空状态。
- 例如在调用 `.join()`/`.detach()` 之前返回 `true`；之后返回 `false`。
3. 析构函数：若 `.joinable()` 则 `std::terminate`，否则什么也不做；
- 也就是说，每个正在运行的 `std::thread` 在销毁前必须调用 `.join()`/`.detach()`。
4. `.get_id()`：获取线程 id；
- 类型为 `std::thread::id` 而不是简单整数。
- 受限整数：仅支持比较、可哈希、可打印（C++11 起可用 `<<`，C++23 起可用 `std::formatter`）。
- 特别地，它只支持 fill-align-width 格式。
- 处于空状态的 `std::thread` 会得到默认构造的 id。

# 线程（Thread）

- 注意：参数会经 decay 后复制到线程函数。
- 自 C++23 起可用 `auto(…)` + 实质化（materialization）来解释，即在当前线程中生成一个 prvalue 并实质化。
- 因此 `auto` 会做类型 decay，例如 `Object&` -> `Object`。
- 当转发类型为 `Object&` 时，则复制；
- 当为 `Object&&` 时，则移动；
- 我们逐步拆解……

# 线程（Thread）

1. 参数传入 `std::thread` 的构造函数。
- 遇到引用时，会实质化 prvalue `Object{}` 并构造为 `arg0`。
2. 随后对 `arg0` 做 decay-copy。
- 这里是 prvalue `Object{ std::move(arg0) }`，并实质化（从而移动构造）。
3. 然后线程执行 `std::invoke`；
- 实质化后的参数被传给新线程。
4. 最后 `arg1` 被转发到 `func` 的参数。
- 从而作为对象被移动构造。

# 线程（Thread）

你们会在作业中自己实现 `std::thread`，以了解这些在实现中究竟如何发生。
- 参数处理的前几步（构造 `std::thread`、为调用准备实参等）发生在创建线程的当前线程侧；将实参转发给用户函数并在新线程中完成调用的阶段发生在新线程侧。
- 因此控制台输出等副作用的先后顺序可能与纸面上的步骤编号并不一致：可能出现新线程已执行用户代码的同时，线程构造函数形参仍在创建线程中析构等情况。
- 构造 `std::thread` 及准备实参期间（线程尚未真正开始执行用户逻辑之前）抛出的异常仍会在旧线程中被捕获。用户函数形参以及为此次调用实质化出的临时对象的析构，则通常发生在新线程的执行上下文中。
- 练习：下面这段代码对吗？
- 不对，因为我们把实质化的临时对象转发给 `func`，即 `func(std::move(…))`；
- 你不能把右值绑定到左值引用……
- 即便使用 `const int&`，它事实上引用的是临时对象，而不是你传给线程构造函数的那个参数。

# 线程（Thread）

- 原因：用 decay-copy 而不是引用更安全。
- 我们在 ICS 中学过，你可以向另一线程传递指针，使另一线程能访问当前线程的内存。
- 若被引用对象已离开其生命周期，你就是在访问无效内存！
- 不同线程同时访问也可能导致数据竞争。

- 需要显式使用 `std::ref()` / `std::cref()` 来传递（`const`）引用。
- 你在 `std::bind_xx` 中见过类似写法，它也会提醒你注意生命周期问题。

# 线程（Thread）

- 注意 1：`std::thread` 提供的 API 是高层抽象；有时你可能需要更细粒度的控制。
- 例如，你可能想改变某些线程的优先级。
- 这取决于平台，因此 C++ 提供了 `.native_handle()`。
- 返回类型取决于平台（例如 POSIX 系统上的 `pthread_t`）。

- 注意 2：静态成员函数 `constexpr std::thread::hardware_concurrency()` 可用于查询真实并行线程数的大致提示。
- 粗略地说，即有多少个物理核心。
- 这只是对可能的最大并行度的一个提示；你需要性能分析才能为程序性能选出最佳线程数。
- 当系统无法给出提示时，返回 0。

# 线程（Thread）

- 注意 3：一些罕见但可能发生的异常，列于此处。
- 所有异常均为带某错误码的 `std::system_error`。
- 构造函数：

即错误码

- `.join()`/`.detach()`：

线程等待自身；`.detach()` 没有这种情况。

# 线程（Thread）

- 注意 4：命名空间 `std::this_thread` 中有许多针对当前线程的方法。
- `get_id()`：获取当前线程的 id。
- `sleep_for()`/`sleep_until()`：暂停当前线程。
- `yield()`：请求让出调度。
- 我们说过线程彼此竞争；操作系统会在线程运行一段时间后对其进行调度。
- 也就是说，线程会“急切”地执行，操作系统会强制它暂停。
- `yield` 表示线程自愿放弃当前执行权，操作系统会重新调度它。
- 然而，若该线程优先级足够高，调度算法仍可能选择原来的线程继续运行。
- 即暂停线程、保存其状态、再加载相同状态并继续运行。

# 多线程（Multithreading）

- 线程（Thread）
- 计算机中的线程模型（Thread model in computers）
- `thread`
- `jthread`
- 杂项主题（Miscellaneous topics）

# `jthread`

- C++ 鼓励 RAII，即析构函数释放构造函数获取的资源。
- 但 `std::thread` 似乎违反了这一点，因为若忘记 `join`/`detach` 线程，整个程序会终止。
- C++20 起 `<thread>` 中的 `std::jthread` 用于解决该问题；若析构时 `joinable()` 仍为真，它会自动 `join` 线程。
- 它拥有 `std::thread` 的全部 API，即你可以使用 `join`/`detach`/`swap`/`move`/`native_handle`/`joinable`/`get_id`/`hardware_concurrency`。
- 但移动时会尝试 `join` 其所持线程，然后再把另一个移入 `*this`。
- 自移动也会 `join` 自身！
- 在 C++11 中，有人认为终止优于静默等待，因此未将自动 `join` 作为 `std::thread` 的默认行为。

# `jthread`

- 此外，`std::jthread` 还增加了停止令牌（stop token）处理。
- 有时也称为协作式取消（cooperative cancellation）。
- 我们知道线程竞争并急切执行。
- 你很少能强迫线程做某事，只能请求它去做。
- 要停止线程，你可以设置一些共享数据，线程周期性检查；检查成功时自愿返回。

- 注意：你不能“杀死”线程（尽管可以杀死进程），因为线程间的数据依赖太常见。
- 例如，若某线程仍持有一把锁却被强制退出会怎样？等待的线程就会死锁！
- 总之，你几乎无法保证杀死线程时它处于一致状态；这就是我们需要把 stop token 当作提示的原因。

# 停止令牌（Stop token）

- 因此请求方持有 stop source，线程持有与 stop source 关联的 stop token。
- 为防止释放后使用（use-after-free），它们共享底层 stop state 并带引用计数；
- 该状态记录相关信息，当计数归零时释放。
- stop source 只能请求一次，在状态中设置某个标志；
- 后续请求没有实际效果。
- 而 stop token 可以定期检查该标志是否已设置。

- 因此该状态应暴露如下接口：
- 设置方：`request_stop()`，设置标志；
- 查询方：`stop_requested()`，检查标志是否已设置；
- 共享/分离：增加/减少引用计数。

# 停止令牌（Stop token）

- 相应地，`<stop_token>` 中的 `std::stop_source` 与 `std::stop_token` 以线程安全的方式封装并暴露它们。

- `std::stop_source`：
- 设置方：`.request_stop()`；
- 查询方：`.stop_requested()`；
- 共享与分离：
- 默认构造函数：创建一个带有全新状态的 `stop_source`。
- 拷贝构造与拷贝赋值：与他人共享当前状态；
- 移动构造与移动赋值：转移所有权；
- 析构函数：分离（detach）。

- `.get_token()` -> `std::stop_token`：获取共享同一状态的 stop token。

# 停止令牌（Stop token）

- `std::stop_token`：
- 无设置方；
- 查询方：`.stop_requested()`；
- 共享与分离：
- 拷贝构造与拷贝赋值：与他人共享当前状态；
- 移动构造与移动赋值：转移所有权；
- 析构函数：分离。
- 例如：

# 停止令牌（Stop token）

- 它们也可以附着到空状态，此时所有操作都不起作用。
- `std::stop_token`：默认构造它；
- `std::stop_source`：添加占位标签：

- 因为默认构造函数会创建新状态。
- 当它们例如被移动走后，状态也会变为空。
- 要检查当前状态是否为空，可使用 `.stop_possible()`；为空时返回 `false`。
- 为空时 `.stop_requested()` 也返回 `false`。
- 特别地，当仅有 stop token 关联到尚未被请求的状态（即没有 stop source，因此不可能有请求）时，`token.stop_possible()` 也返回 `false`。

- 小结：

`std::stop_token` `std::stop_source`

# 停止令牌（Stop token）

- 那么 `std::jthread` 如何与 stop token 协作？
1. 它直接包含一个默认构造的 `std::stop_source`。
- `.get_stop_source()` 获取一份拷贝；
- `.get_stop_token()` 获取与底层 source 关联的 token；
- 等价于 `underlying_source.get_token()`。
- 以及 `.request_stop()`，等价于 `underlying_source.request_stop()`。
2. 在 `std::jthread` 的析构函数中，若 source 尚未发出请求，则调用 `.request_stop()`；
- 在某种程度上也是 RAII。
3. 在可能的情况下，会把 `get_token()` 传给其函数对象。

# 停止令牌（Stop token）

- 例如：

`get_stop_token()` 会自动提供。

由于 `std::jthread` 的析构函数会自动 `.request_stop()`，此处或可省略。

注意：`std::cout` 在多线程中使用是安全的；其他流在 C++20 中应使用 `std::osyncstream`。

# 停止令牌（Stop token）

- 另一例：问题：这里能否省略 `t.request_stop()`？

不能！因为 `t2` 先被析构，故 `t2.join()` 发生在 `t` 的析构函数中的 `t.request_stop()` 之前。

于是 `t2` 中无限循环……`t2` 的函数对象未使用自身的 `.get_token()`；该函数对象与 `t` 共享同一状态。

# 停止令牌（Stop token）

- 最后，停止请求可与回调关联。
- 通过带 `std::stop_token` 的 `std::stop_callback`：
- 构造函数在状态上注册回调；

- 析构函数注销回调。
- 例如：

回调将在此处执行。

# 停止令牌（Stop token）

- 注意 1：这很像以线程安全的方式执行回调。
- 多次请求时回调恰好执行一次；
- 注册与注销是线程安全的；若并行发生，在一个线程中注销会等待另一线程中的调用完成。
- 注意 2：第一个调用 `.request_stop()` 的线程将执行所有回调；
- 若有多个回调，执行顺序未规定。
- 注意 3：若在注册之前已发出请求……
- 即在 `std::stop_callback` 的构造函数中 `token.stop_requested() == true`；
- 则回调会在当前线程的构造函数中立即执行。
- 注意 4：回调不允许抛出异常；若回调抛出异常则 `std::terminate()`（视为 `noexcept`）。

# 多线程（Multithreading）

- 线程（Thread）
- 计算机中的线程模型（Thread model in computers）
- `thread`
- `jthread`
- 杂项主题（Miscellaneous topics）

# 线程中的异常（Exception in threads）

- 我们知道，若在单线程程序中不捕获异常，则 `std::terminate`。
- 一般而言，任一线程在退出时若未捕获其异常，都会导致 `std::terminate`。
- 因此如下代码不起作用：

# 线程中的异常（Exception in threads）

- 如何把异常传出线程？
- 通过定义在 `<exception>` 中的 `std::exception_ptr`！
- 粗略地说，它是指向异常的共享指针。
- 仅当指向该异常对象的所有指针都析构后，对象才析构。
- 实际类型由实现定义……
- 标准规定暴露这些接口：

- 默认构造：如同空指针；
- `std::make_exception_ptr(Exception)`：生成拷贝 `Exception` 的指针；
- 并可像指针一样转换为 `bool`。
- `std::current_exception()`：在 `catch` 块中使用，如同对当前捕获的异常做 `make_exception_ptr`；
- `std::rethrow_exception(std::exception_ptr)`：重新抛出该异常对象。

# 线程中的异常（Exception in threads）

- 例如：

在主线程中继续抛出该异常。

通过参数将异常传出线程。

# 静态块变量（Static block variable）

- 过去我们可能用函数内的 `static` 变量在多次调用间共享。
- 但在多线程中使用是否安全？
- 是，也不是……
- 是：自 C++11 起，仅有一个线程会执行初始化，其他线程会等待。
- 否：若要在多线程中修改它，仍需要加锁。

# 一次性执行（Once-for-all）

- 更一般地，若我们希望在所有线程间某段代码只执行一次……
1. 借助 `static` 变量的技巧：
- 该技巧也常用于单线程程序……
2. 通过定义在 `<mutex>` 中的 `std::call_once` 与 `std::once_flag`：

# 一次性执行（Once-for-all）

- `std::once_flag` 仅有默认构造函数，表示“尚未调用”。
- 而 `std::call_once` 会设置标志；恰好一个线程会执行可调用对象，其他线程会等待其完成。
- 有何区别？
1. `std::call_once` 略更灵活、直观；
- 但返回值被忽略。
2. `static` 变量技巧可能有略好的性能。
- 详见 Stack Overflow。
3. `static` 变量的递归初始化是未定义行为（UB）；
- 而 `std::call_once` 会导致死锁。

# 一次性执行（Once-for-all）

- 有时我们只想在当前线程的调用之间共享变量；
- 例如每个线程有各自的“静态块变量”。
- 我们可以使用 `thread_local` 指定线程存储期！

这里的 `static` 可以省略；`thread_local` 块变量若未指定则隐含 `static`。见 C++ 标准。

- 当然，也可用于“全局”变量……

注意：此处不允许写 `static`。

# 一次性执行（Once-for-all）

- `thread_local` 全局变量在线程启动后创建，在线程退出时析构。

- 结语：若异常从以下情况抛出：
- 静态块变量的初始化；
- `std::call_once` 的函数；
- 则视为执行失败，下次会再次初始化/执行。
- 因此要注意是否存在不能执行两次的其他副作用。

# 多线程（Multithreading）

同步工具（Synchronization Utilities）

# 多线程（Multithreading）

- 同步工具（Synchronization Utilities）
- 信号量（semaphore）
- 互斥量与锁（mutex & lock）
- 条件变量（condition variable）
- 闩（latch）与屏障（barrier）

# 信号量（Semaphore）

- 我们在 ICS 中学过数据竞争。
- 当多于一个线程并发访问同一内存位置，且至少有一个在写入时，最终结果不可预期、不可预测。
- 例如：两个线程各执行 `a++` 共 100000 次，结果未必是 `a += 200000`。
- 我们把可能引起数据竞争的区域称为“临界区”（critical section，CS）。
- 最基本的方式是保证只有一个线程操作数据！
- Dijkstra 在 1962 年提出的信号量（Semaphore）即为此目的。
- 它有初始整型状态与两个操作。
- V，或称 up、release，表示状态加 1。
- P，或称 down、acquire，表示状态减 1。
- 状态不能为负；若已为 0 则等待直到被增加。
- 两个操作互斥且可安全并发执行。

由于初始状态为 1，只有一个线程

# 信号量（Semaphore）

能进入 CS 并执行 `a++`。

- `<semaphore>` 自 C++20 引入：
- 构造函数即初始整型状态。

- 更一般地，可使用 `std::counting_semaphore<LeastMaxValue>`。
- 而 `std::binary_semaphore` 就是 `std::counting_semaphore<1>`。
- 库可能基于 `LeastMaxValue` 做优化，例如当 `LeastMaxValue <= 255` 时用 `std::uint8_t`，否则 `std::uint32_t` 等。
- 因此信号量能表示的实际最大可能值未必等于 `LeastMaxValue`；
- 可使用静态成员 `constexpr std::ptrdiff_t max()` 检查实际最大可能值。

# 信号量（Semaphore）

- 相当简单，故仅列出方法如下：
- 构造函数：赋予初始整型状态；
- 不可复制，可移动，不可复制赋值，可移动赋值。
- 获取/递减：
- `.acquire()`：减少信号量；若不能（即当前状态为 0）则等待。
- `.try_acquire()`：尝试减少信号量并返回 `true`；若不能则返回 `false`。
- 允许虚假失败（spuriously fail），即当前状态非 0 但 `try_acquire` 仍返回 `false`。
- `.try_acquire_for`/`try_acquire_until(…)`：在至多一段时间内尝试减少信号量；失败则返回 `false`。
- 释放/递增：
- `.release(std::ptrdiff_t n = 1)`：将信号量增加 `n`。
- 析构函数：析构时不应有线程正在等待该信号量（否则 UB）。

# 多线程（Multithreading）

- 同步工具（Synchronization Utilities）
- 信号量（semaphore）
- 互斥量与锁（mutex & lock）
- 条件变量（condition variable）
- 闩（latch）与屏障（barrier）

# 互斥量（Mutex）

- 互斥（mutual exclusion）使线程以互斥方式执行某些操作。在 `<mutex>` 中。
- 有点像二元信号量。
- 对我们的 `++` 例子，也可以这样写：
- 同样是非常简单的 API，仅列于此。
- 默认构造函数，表示“未锁定”状态。
- 不可复制，可移动，不可赋值。
- `.lock()`/`.unlock()`；
- `.try_lock()`；
- 无与时间相关的 API，例如 `.try_lock_for`。
- 析构函数：析构时不应有线程正在等待该互斥量。
- `.native_handle()`：可选且取决于平台，例如在 POSIX 系统上返回 `pthread_mutex_t`。

# 互斥量（Mutex）

- 那么互斥量与信号量有何区别？
1. 信号量可用 0 或大于 1 的整型初始化，但互斥量始终像初始状态为 1 的二元信号量。
- 信号量的增减参数也可以不一致，例如可以 down 1 但 up 3。
2. 信号量可由未获取它的线程释放，但互斥量必须由加锁的线程解锁。
- 因此互斥量正用于互斥；
- 而信号量还可用于其他用途，例如有时充当高效的条件变量。

你将在操作系统课程中学习如何实现信号量与互斥量。

# 互斥量（Mutex）

- 同一线程对互斥量锁定两次则为 UB。
- 典型表现是死锁。
- 实现被鼓励抛出带错误码 `resource_deadlock_would_occur` 的异常 `std::system_error`。
- 若要在加锁线程中多次锁定，可使用 `std::recursive_mutex`。
- 需要匹配次数的 `unlock` 才会真正解锁。
- API 与 `std::mutex` 完全相同，不再重复列出。
- 例如：

- 通常，使用递归互斥量的代码可以在某种程度上改写为只使用普通互斥量。
- 注意：为了能在线程安全的 `const` 成员函数中加锁与解锁，互斥量数据成员常声明为 `mutable`。

# 锁（Lock）

- 互斥量不遵循 RAII……
- 若你在许多路径返回（包括可能的异常），手写 `unlock` 既痛苦又危险。
- 更好的方式是在构造函数中加锁，在析构函数中解锁。
- C++ 中的锁（lock）即为此而生！
- 最基本的是 `<mutex>` 中的 `std::lock_guard<MutexType>`：
- 对传入的 `MutexType`，构造函数会调用 `.lock()`；
- 析构函数中调用 `.unlock()`。注意：
1. 此处有 CTAD，故可省略 `MutexType`。
2. `_` 是必要的，否则为临时对象，会在该语句结束后立即析构（即解锁）。

# 锁（Lock）

- 注意 1：若想在 `if` 子句中加锁，可使用初始化语句（initializer if）：
- 它会在整个子句期间保持锁定，例如包括此处的 `else`。

- 注意 2：`std::lock_guard` 没有其他 API；在析构前不能通过 `lock_guard` 解锁。
- 注意 3：若要用 `std::lock_guard` 管理已锁定的互斥量，应添加标签参数 `std::adopt_lock`。
- 一个简单的例子：

# 锁（Lock）

- 更一般的 RAII 类型是 `std::unique_lock`。
- 它“拥有”某互斥量，也可放弃该所有权。
- 具体而言，所有权有两方面：A. 是否持有（关联）互斥量；B. 是否已锁定互斥量。

- 以构造函数为例：
- 默认构造函数：无互斥量，未锁定 → `!A & !B`
- `(MutexType& m)`：取得互斥量并锁定（调用 `m.lock()`）→ `A & B`
- `(MutexType& m, std::adopt_lock)`：取得已锁定的互斥量 → `A & B`
- `(MutexType& m, std::defer_lock)`：取得互斥量但不锁定 → `A & !B`
- `(MutexType& m, std::try_to_lock)`：调用 `m.try_lock()`；
- 若成功 → `A & B`；否则 → `A & !B`。

# 锁（Lock）

- 析构函数：`A & B` → 在析构中 `.unlock()`；否则什么也不做。
- 当 `std::unique_lock` 持有互斥量时……
- 若已锁定，可调用……
- `.unlock()`。
- 若尚未锁定，可调用……
- `.lock()`。
- `.try_lock()`。
- `.release()`：解除与互斥量的关联（不调用 `.unlock()`）。
- 当 `std::unique_lock` 没有互斥量时……
- 所有加锁相关操作，例如 `.unlock`/`.lock`/`.try_lock()`，都会抛出异常。
- 可使用移动赋值/`swap` 以关联新的互斥量；
- 移动赋值等价于调用析构函数 + 窃取新状态（移动构造）。

# 锁（Lock）

- 也可观察所有权：
- `.mutex()`：返回指向互斥量的指针；若无关联互斥量则为 `nullptr`；
- `.owns_lock()` / `operator bool()`：检查是否有关联互斥量且已锁定（`A & B`）。
- 例如：

# 锁（Lock）

- 另一例：

我们稍后会讲 `std::lock`；它在此处锁定两个锁。

# 锁（Lock）

|  | `!A & !B`（无互斥量） | `A & !B`（有互斥量，未锁定） | `A & B`（有互斥量，已锁定） |
| --- | --- | --- | --- |
| `.lock()` / `.try_lock()` | `std::system_error{ operation_not_permitted }` | → `A & B` | `std::system_error{ resource_deadlock_would_occur }` |
| `.unlock()` | `std::system_error{ operation_not_permitted }` | `std::system_error{ operation_not_permitted }` | → `A & !B` |
| `.release()` | → `!A & !B` | → `!A & !B` | → `!A & !B` |
| `.mutex()` | `nullptr` | 指向互斥量的指针 | 指向互斥量的指针 |
| `.owns_lock()` / `operator bool()` | `false` | `false` | `true` |

此处省略了例如 `mutex.lock()` 可能抛出的异常。

# 共享互斥量（Shared mutex）

- 前述所有互斥量在临界区内都是独占访问。
- 但事实上只有写需要独占；读可以同时进行。
- 也称为“读者-写者问题”（Readers-Writers Problem）。
- C++17 起的共享互斥量即为此……
- `<shared_mutex>` 中的 `std::shared_mutex`；
- 两种模式——独占模式与共享模式。
- 独占模式下，API 与行为与 `std::mutex` 相同。
- 互斥量只能被一个线程持有；其他线程会等待直到解锁。
- 共享模式下提供 `.lock_shared()`、`.unlock_shared()` 与 `.try_lock_shared()`。
- 互斥量可被多个线程持有；要以独占模式持有的线程会被阻塞，直到所有共享模式的线程都解锁。

不匹配的加锁与解锁（例如在独占模式加锁却在共享模式解锁）是 UB。

# 共享锁（Shared lock）

- 相应地，`<shared_mutex>` 中提供了用于 RAII 的 `std::shared_lock`（自 C++14，早于 `std::shared_mutex` 被接受）。
- 所有 API 与 `std::unique_lock` 完全相同；唯一区别是调用 `mutex.lock()`/`unlock()`/`try_lock()` 的函数会改为调用 `mutex.lock_shared()`/`unlock_shared()`/`try_lock_shared()`。

- 因此，要以 RAII 方式在独占模式下锁定 `std::shared_mutex`……
- 只需使用 `std::unique_lock _{ mut }` 或 `std::lock_guard _{ mut }`；
- 而要以 RAII 方式在共享模式下锁定 `std::shared_mutex`……
- 只需使用 `std::shared_lock _{ mut }`。

# 共享锁（Shared lock）

- 一个简单的例子：

# 共享互斥量实现*

- 本部分为选读。
- 在 ICS 中我们学过饥饿问题……
- 写者饥饿：若优先读者，则读者很多时互斥量会一直处于共享模式。
- 在某读者读完之前，新读者不断到来，写者会永远阻塞。
- 读者饥饿：类似地，若优先写者，则写者很多时互斥量会一直处于独占模式。
- 因此某些算法中可为锁指定“优先级”。
- 但 C++ 标准库似乎不这样做！
- Howard E. Hinnant 提出了一种参考实现，由 Alexander Terekhov 发明。

抱歉，我找不到 Alexander Terekhov 的确切博客/论文/……。似乎是在约 2003 年 `pthread-win32` 开发期间发明的。

# 共享互斥量实现*

- 该算法使读者与写者之间相对“公平”。
- 简单说有两道门：
- 门 1：读者与写者公平竞争；
- 读者通过门 1 后，可以以共享模式获得互斥量。
- 写者通过门 1 后，需要等待门 2。
- 门 2：当写者在门 2 等待时，门 1 应阻塞所有请求；
- 写者会等待所有已通过门 1 的读者都解锁。
- 然后写者以独占模式获得互斥量。
- 写者解锁后，门 1 再次接受请求。
- libc++ 使用该算法；libstdc++ 有宏控制（要么此算法，要么原生 `pthread_rwlock_t`）；MS-STL 使用原生 SRW。

但 SRW 存在操作系统缺陷，可能在共享模式下授予独占所有权。该问题在 2024.5.9 修复，但包含该修复的 Windows 更新未知。

# 安全回收（Safe Reclamation）*

- 共享互斥量使并发读成为可能，但仍对写授予互斥。
- 是否可能让读与写“并发”发生？
- 通过读-拷贝-更新（RCU）或危险指针（hazard pointer）！
- 简单说来，RCU 通过分配新版本来允许并发写，从而先前的读仍可继续。
- 当没有读者在使用旧版本时再释放旧版本。
- 危险指针通过其他技术登记“删除某版本是危险的”，从而避免释放后使用问题。
- C++26 通过 `<rcu>` 与 `<hazard_pointer>` 引入它们。
- 相当复杂，此处不讨论。

# 死锁（Deadlock）

- 最后谈谈死锁。
- 当然，我们指的不是同一线程多次加锁导致的死锁。
- 那可以用 `std::recursive_mutex` 解决。
- 典型例子：不同线程以不同顺序加锁。

Worker1 持有 `mut1`，等待 `mut2`；但 Worker2 持有 `mut2`，等待 `mut1`。

# 死锁（Deadlock）

- 仅当以下四个条件同时成立时才会发生死锁：
1. 互斥，即只有有限数量的线程可使用该资源。
2. 占有并等待，即线程持有某些资源的同时请求其他资源。
3. 不可抢占，即资源只能自愿放弃，而某线程从不放弃；其他线程也不能强迫其放弃。
4. 循环等待，即占有并等待的序列形成环。

- 要防止死锁，需要打破其中之一。
- 1. 通常不易打破，因为它由共享数据的性质决定。
- 2. 使用多把锁时往往也不可避免；若能只用一把锁则没问题。

# 死锁（Deadlock）

- 一般而言，有三种实用的解决死锁途径：
- 死锁预防：预先设计使死锁不会发生。
- 例如始终以相同顺序锁定所有互斥量；
- 死锁恢复：任由死锁发生；但当某资源被占用过久，所有者会被迫放弃。
- 这打破了“3. 不可抢占”。
- 死锁避免：预先检查后续资源请求是否可能导致死锁，并在必要时动态推迟。
- 这打破了“4. 循环等待”，即若可能出现循环等待，则推迟后续请求。
- 死锁避免有许多精巧算法。
- C++ 将死锁避免封装为函数 `std::lock`。

# 死锁避免（Deadlock Avoidance）

- 例如：

因加锁顺序相反而产生死锁。

为使死锁必然发生。

# 死锁避免（Deadlock Avoidance）

- 若我们使用 `std::lock(…)`：
- 注意互斥量仍以相反顺序传入 `std::lock`。

- 于是你现在可以理解这段代码……
- `mutex` 或 `unique_lock` 均可，因为二者都提供 `.lock`/`.unlock`/`.try_lock()`；不能传入 `lock_guard`。

# 死锁避免（Deadlock Avoidance）

- 自 C++17 起还提供 `std::scoped_lock`，以 RAII 方式做死锁避免。

- 与 `std::lock_guard` 类似，仅有构造函数与析构函数。构造函数实际调用 `std::lock`。
- 但带 `std::adopt_lock` 的重载把其作为第一个参数，因为可变参数只能放在最后。

# 死锁避免（Deadlock Avoidance）

- 注意 1：多个 `std::lock`/`scoped_lock` 仍可能导致死锁；仅当所有锁都在一个实例中提供时，避免算法才能生效。

- 注意 2：还存在函数 `std::try_lock`，它不是死锁避免，而只是“全有或全无”的包装。
- 按参数顺序恰好锁定所提供的锁；
- 任一次 `.try_lock()` 失败（包括抛出异常）即整体失败，会解锁先前已锁定的锁。
- 仅当所有锁都锁定成功才算成功。
- 返回未能锁定的互斥量的下标，成功则返回 `-1`。
- 等价于（忽略异常）：

# 定时互斥量（Timed mutex）

- 回到死锁恢复……
- 我们也可在等待过久时让所有者自愿放弃资源。
- 即我们需要类似信号量 `.try_acquire_for`/`try_acquire_until()` 的机制……
- 为此提供了定时互斥量！
- 所有互斥量类型都有定时版本：`std::timed_mutex`、`std::recursive_timed_mutex` 与 `std::shared_timed_mutex`。
- 通常定时互斥量比普通互斥量开销略高。
- 注意 `std::shared_timed_mutex` 在 C++14 引入，而为性能在 C++17 引入了 `std::shared_mutex`。
- 在原有 API 之外，它们还增加定时 API。
- 即 `.try_lock_for`/`try_lock_until()`；`.try_lock_shared_for`/`try_lock_shared_until()`。

# 定时互斥量（Timed mutex）

- 当提供的互斥量类型为定时类型时，`std::unique_lock` 与 `std::shared_lock` 也会有 `.try_lock_for`/`try_lock_until()`。
- 以及额外的构造函数重载：

- 再次注意 `std::shared_lock::try_lock_for()` 调用的是 `MutexType::try_lock_shared_for()`。
- 例如，用于死锁恢复：

# 死锁恢复（Deadlock Recovery）

此处两段基本等价。

# 多线程（Multithreading）

- 同步工具（Synchronization Utilities）
- 信号量（semaphore）
- 互斥量与锁（mutex & lock）
- 条件变量（condition variable）
- 闩（latch）与屏障（barrier）

# 条件变量（Condition Variable）

- 除互斥外，并发编程还需要同步。
- 即，在某条件满足后，我再做某事……
- 若只提供互斥，则只能忙等：
- 效率低，CPU 占用高。

- 或者我们可以做“不那么忙的等待”：
- 但睡眠的理想时长是？
- 太短：近似忙等
- 太长：吞吐低。
- 最好由操作系统决定，或由修改条件的一方通知……

# 条件变量（Condition Variable）

- 条件变量是惰性等待同步的原语。
- 它需要一个谓词来判断条件；
- 以及一把锁，在调用谓词时保证互斥。
- 它有两个操作：
- 等待（Wait），即等待某条件满足。
1. 等待前，当前线程应持有锁。
2. 开始等待时会释放锁；
3. 一旦解除阻塞，会重新获取锁以安全检查条件；
4. 若条件不满足，回到步骤 2；否则结束等待。
- 因此条件变量除在步骤 2 中阻塞外，应始终持有互斥量。
- 通知（Notify 或 Signal），即通知等待线程条件可能已满足。
- 于是等待条件变量的线程进入步骤 3 并检查。

条件变量在某些语言/资料中也称为 Monitor（管程）。

# 条件变量（Condition Variable）

- 在 C++ 中，我们可以直接使用定义在 `<condition_variable>` 中的 `std::condition_variable`。
- 等待：
- `.wait(std::unique_lock<std::mutex>& lock)`；
- `.wait_for`/`wait_until(lock, time)` -> `std::cv_status`；
- 要么 `std::cv_status::no_timeout`，要么 `std::cv_status::timeout`。
- 通知：
- `.notify_one()`：唤醒一个等待线程；
- `.notify_all()`：唤醒所有等待线程（也称 Broadcast）。
- 例如，在初始化完成后启动任务：

你将在操作系统课程中学习如何实现条件变量。

- 对等待线程：

1. 首先持有互斥量，以便安全检查条件；
2. 进入阻塞等待时会调用 `.unlock()`，从而让修改者能够改变条件；
3. 解除阻塞时会重新 `.lock()`，然后再次检查条件；
4. 条件满足时跳出循环（此时互斥量仍被锁定）。

- 对通知线程：

1. 持有互斥量并修改条件；
2. 通知等待线程去再次检查条件；
3. （等待线程侧）解除阻塞后会 `.lock()`，然后检查条件；
4. 条件满足时跳出循环（此时互斥量仍被锁定）。

条件访问始终在互斥量保护下互斥进行。

# 条件变量（Condition Variable）

- 那么为何需要循环而不是 `if (!complete)`？
- 似乎只有两种结果：
- `complete`，则初始化已完成，继续；
- `!complete`，则
- 在变量被通知（并解除阻塞）后，条件已满足，因此我们也继续；
- 似乎检查一次条件就够了。[1]

[1]：确切地说，这种行为称为 Mesa 监视器；立即调度称为 Hoare 监视器，后者通常不被当前操作系统实现，因为实现困难且开销大。

- 两个原因：
1. 在更复杂场景中，条件可能被多方修改。
- 等待线程不会立即被调度，因此在被调度前条件可能再次不成立（也称“ABA 问题”）。
2. C++ 允许条件变量虚假唤醒（spurious wakeup）。
- 即便未被通知，条件变量也可能解除阻塞。
- 此处指当 `complete` 仍为 `false` 时，`.wait()` 也可能返回。

# 条件变量（Condition Variable）

- 因此在 C++ 中，条件变量几乎总应与循环配合，以确保某条件已满足。
- 因此它们被直接封装在 `wait` 的重载中：
- `.wait(lock, condPred)`，等待直到 `condPred` 满足；
- 等价于：因此在 `condPred` 中，锁已被锁定。

- `.wait_for`/`wait_until(lock, time, condPred)` -> `bool`；
- 等价于：

- 例如：

# 条件变量（Condition Variable）

- 注意 1：不应依赖虚假唤醒；条件满足时总要通知。
- 我在嵌入式设备（树莓派）上的经验是除非被通知否则从不唤醒；但我的 Windows 机会发生虚假唤醒。
- 注意 2：通常最好在通知前先解锁。
- 原因：若先通知再解锁，可能发生：
1. 等待线程被唤醒（就操作系统调度而言）；
2. 它尝试锁定互斥量，但失败并再次阻塞。
3. 通知线程解锁互斥量；
4. 等待线程被唤醒，最终才能检查条件。
- 但若我们先解锁再通知，就给等待线程机会直接唤醒并持有互斥量，而无需再次阻塞。

# 条件变量（Condition Variable）

- 注意 3：即使不这样做也没有数据竞争（例如原子变量），仍必须在持有互斥量的情况下修改条件。
- 例如：
- 于是过程可能是：
1. 等待线程已检查 `!complete`；
2. 通知线程设置 `complete` 并通知 `condVar`；
3. `condVar` 正在等待（因而错过通知并可能永不唤醒！）。
- 当持有互斥量时，步骤 1 与步骤 3 不可分割（因此步骤 2 无法插入），从而保证正确。

# 条件变量（Condition Variable）

- 注意 4：最后列出省略的 API：
- 仅有默认构造函数，不可复制，可移动，可赋值。

- `.native_handle()`；
- 析构函数：析构时不应有人正在等待该 cv。

# 条件变量（Condition Variable）

- `std::condition_variable` 只能对 `std::unique_lock<std::mutex>` 等待以最大化效率。
- 有时我们需要更一般的条件变量，可对任意可锁定对象等待……
- 则可以使用 `std::condition_variable_any`！
- API 的唯一区别是 `wait` 为模板函数，可接受任意可能的锁：

- 例如，编写简单的系统 tick 模拟器……
- 即，`tick` 周期性全局递增；
- 对调用 `Sleep(tickNum)` 的线程，除非已经过 `tickNum` 个 tick，否则阻塞。

# 条件变量（Condition Variable）

- 似乎使用 `std::shared_mutex` 更好，因为那样所有线程可同时读取 `tick`。

# 条件变量（Condition Variable）

- 自 C++20 起，`std::condition_variable_any` 也增加了 stop token 处理。
- `.wait`/`wait_for`/`wait_until(lock, stop_token, condPred, …)` -> `bool`。
- 它们也会检查是否已请求停止；
- 并会在 stop state 上注册其通知，因此请求停止时会自动被通知（然后退出 `.wait`）。

- 注意：`std::condition_variable` 未引入 stop token 处理，因为（在相同优化前提下）不可能实现。
- 详见 Anthony Williams 的回答。

# 条件变量（Condition Variable）

我们稍后会更多讨论 `xx_at_thread_exit` 方法。

- 最后，若要通知其他线程当前线程已退出，可使用 `std::notify_all_at_thread_exit(std::condition_variable& cond, std::unique_lock<std::mutex> lock)`。
- `xx_at_thread_exit` 在当前线程中所有其他事情结束之后调用，甚至在 `thread_local` 变量析构之后。
- 然后 `lock.unlock()`，再 `cond.notify_all()`。

# 多线程（Multithreading）

- 同步工具（Synchronization Utilities）
- 信号量（semaphore）
- 互斥量与锁（mutex & lock）
- 条件变量（condition variable）
- 闩（latch）与屏障（barrier）

# 闩（Latch）

在 `<latch>` 中，自 C++20。
- 闩是一次性同步机制。
- 它有初始整型状态，支持两个操作：
- 到达（Arrive），使整型状态递减；
- `.count_down(std::ptrdiff_t n = 1)`；
- 等待（Wait），等待直到整型状态达到 0。
- `.wait()`/`.try_wait()`；`try_wait` 允许虚假返回 `false`。
- 也可通过 `.arrive_and_wait(n = 1)` 组合二者，先按 `n` 执行 `count_down` 再等待。
- 例如，我们调用多个线程做初始化……
- 仅当所有初始化完成，这些线程才能安全继续。
- 主线程也应等待初始化完成。
- 因此每个线程可 `arrive` 并 `wait`，主线程应只 `wait`。

# 闩（Latch）

若工作线程仅依赖自身初始化，则可调用 `.count_down()`，这样只有主线程会等待所有初始化完成。
- 于是我们可以这样编码：

仅当四个线程都到达后状态才变为 0，从而所有线程解除阻塞。

# 屏障（Barrier）

- 但有时我们可能要在多个阶段重复动作，于是需要“复用闩”。在 `<barrier>` 中，
- 屏障是可复用的同步机制。自 C++20。
- 类似地，它有初始整型状态，并支持：
- 到达（Arrive），使整型状态递减；
- `.arrive(std::ptrdiff_t n = 1)`；
- 等待（Wait），等待直到整型状态达到 0。
- `.wait()`（无 `try_wait`）；
- 也可通过 `.arrive_and_wait(n = 1)` 组合二者。
- 当整型状态达到 0 时，称一个阶段（phase）已完成；
- 所有等待线程解除阻塞，整型状态会恢复，从而可进行下一阶段的同步。

# 屏障（Barrier）

- 当一个阶段结束时，屏障还可执行某回调。
- 在构造函数中按值传入；默认回调什么也不做。
- 回调必须标为 `noexcept`。
- 回调发生在解除阻塞之前（即下一阶段开始之前）。

因此 `std::barrier` 实际上是模板类。

- 线程也可选择退出下一阶段：
- `.arrive_and_drop()`：将状态与其初始值各减 1。
- 玩具示例（仅展示用法）：打印乘法表。

# 屏障（Barrier）

# 屏障（Barrier）

- 最后列出一些省略的 API（对 `std::latch` 与 `std::barrier` 均适用）：
1. 仅有默认构造函数，不可复制/移动/赋值。
2. 析构函数：不应有线程正在等待。
3. 静态成员 `constexpr std::ptrdiff_t max()` 可用于检查支持的最大整型状态值。
4. 将状态设为 < 0（包括递减到 < 0），或 > `max()`，均为 UB。

# 多线程（Multithreading）

异步操作的高层抽象（High-Level Abstraction of Asynchronous Operations）

# 多线程（Multithreading）

- 异步操作的高层抽象（High-level abstraction of asynchronous operations）
- Future-Promise 模型（Future-Promise model）
- `packaged_task`
- `async`

# Future-Promise 模型（Future-promise model）

- 有时我们觉得使用 `std::thread` 太低层……
- 无返回值，异常管理复杂，只能靠引用参数；
- 若返回值设置较早（例如线程还需做额外清理，但返回值已准备好），主线程需要同步才能正确取得。
- 相反，我们希望屏蔽这些细节，更偏好基于任务的视角……
- 即，我们把任务交给线程，并希望在未来取得结果，而不关心任务何时完成或是否需要等待。

# Future-Promise 模型（Future-promise model）

- 这就是 Future-Promise 模型提供的。
- “future” 表示任务的生产方，意味着它期望在“未来”得到某结果。
- “promise” 表示任务的执行方，意味着它承诺提供某结果。
- 具体而言，Future-Promise 模型是一次性通道（one-shot channel）。
- promise 端工作并把结果传送到 future 端；
- 在通道销毁前，结果只能设置与获取各一次。
- 例如：

用基本原语实现会非常复杂……

# Future-Promise 模型（Future-promise model）

- 若我们使用 `<future>` 中的 `std::future<T>` 与 `std::promise<T>`：

1. 创建通道；
2. 将 `promise` 传给任务执行方；
3. 从通道异步取得结果（可等待也可不等待，取决于工作者）。

4. 工作者向通道设置结果（`future` 可较早取得结果，与 `DoCleanup()` 并行）。

# Future-Promise 模型（Future-promise model）

- 注意 1：对异常管理，可使用 `.set_exception(std::exception_ptr)`。
- 例如：
- 对 `future.get()`，则如同 `std::rethrow_exception(ptr)`。
- 注意 2：`std::future` 与 `std::promise` 仅可移动。
- 至多一个 `future` 与一个 `promise` 占据通道。
- 通道在双方之间共享，为防止释放后使用问题，也引入引用计数。
- 与 stop token 处理类似，该通道也称为共享状态（shared state），`future` 与 `promise` 各持有一个指向它的指针。
- 那么 Future-Promise 中的“一次性通道”究竟指什么？

# Future-Promise 模型（Future-promise model）

- 对 `std::future`，
- `.get()` 只能调用一次，随后切断与共享状态的关系。
- 之后你需要直接操作取回的结果，而不能反复调用 `.get()`。
- 它本质上将指向共享状态的指针置为 `nullptr`。
- 对 `std::promise`，
- `.get_future()` 只能调用一次，以唯一地共享通道；
- 否则抛出 `std::future_error`，错误码（`.code()`）为 `std::future_errc::future_already_retrieved`。
- `.set_xx()` 只能调用一次，意味着每个任务只有一个结果（某个值或某个异常）。
- 否则抛出 `std::future_error`，错误码为 `std::future_errc::promise_already_satisfied`。

# Future 详解（Future in detail）

- 具体而言，`std::future` 要么关联共享状态，要么不关联（即无状态）。
- 可用 `.valid()` 检查；当有效时可调用：
- `.get()` -> `T`，取得结果后变为无状态（无效）。
- 结果从共享状态 `std::move` 取出，因为只检索一次。
- `.wait()`，等待直到结果就绪（即随后 `.get()` 会立即返回而非等待）。
- `.wait_for`/`wait_until(time)` -> `std::future_status`；
- 返回值为有作用域枚举，要么 `std::future_status::ready`，要么 `std::future_status::timeout`。
- 若无效，上述所有操作导致 UB。
- 但标准鼓励抛出带错误码 `std::future_errc::no_state` 的 `std::future_error`。
- 默认构造或移动后的对象均为无效。

2023.10.6 @ 水立方

# 共享 Future（Shared future）

- 若你确实想多次检索结果，可使用 `std::shared_future`。
- 例如：共享 future 可复制且可多次 `.get()`。

将状态转移到共享 future（原 `future` 随后无效）；

# 共享 Future（Shared future）

- 因此可使用 `.share()` 或 `std::shared_future(std::future&&)` 得到 `std::shared_future`。
- 二者都把共享状态从唯一 `future` 转移到共享 `future`，使唯一 `future` 无效；
- 所有 API 与 `std::future` 相同，除了：
- 可复制，即可与其他 `std::shared_future` 共享通道；
- `.get()` -> `const T&`，因为结果不是一次性的，故不能移动。
- 可多次调用而不使自身无效。
- 本质上，每个 `std::shared_future` 对象都会增加共享状态的引用计数并持有指向它的指针。

# Promise 详解（Promise in detail）

- 对 `std::promise`，它在默认构造函数中创建通道。
- 并可用 `.get_future()` 与 `std::future` 共享通道。
- 除 `.set_value`/`set_exception(…)` 外，还可调用 `.set_value`/`set_exception_at_thread_exit(…)`。
- 结果在此调用中立即设置，但仅在线程退出后（且在线程局部对象析构后）才能被取得。
- 一个简单的例子：此处：引用参数 + `std::ref`；

先前：值参数 + `std::move`！

为何？

# Promise 详解（Promise in detail）

- 形式化地，通道（共享状态）有三种情况：
1. 既未设置结果，也未就绪：`promise` 尚未调用任何 `.set_xx()`。
2. 已设置结果但未就绪：`promise` 已调用 `.xx_at_thread_exit()`，但线程尚未退出。
3. 已设置结果且就绪：`promise` 已调用 `.set_value`/`set_exception()`，或线程在 `promise` 先前调用 `.xx_at_thread_exit()` 后退出。
- `promise` 在析构前必须就绪。
- 否则任务未完成且共享状态被抛弃。
- `promise` 的析构函数随后会自动设置异常（带错误码 `std::future_errc::broken_promise` 的 `std::future_error`），并使其就绪。
- 与 `future` 不同，`promise` 仅在析构时切断与共享状态的关系。

libstdc++ 与 libc++ 会在结果一旦设置后就让 `promise` 切断与共享状态的关系，在该问题上行为不同。

# Promise 详解（Promise in detail）

详见我在 Stack Overflow 上的完整分析！

- 因此若将 `std::promise` 移入线程并调用 `.xx_at_thread_exit()`……
- 则在线程退出前就会析构，因为局部变量在线程退出前析构。
- 于是尝试设置异常；但我们已设置结果，导致冲突（例如在 MS-STL 中，`std::terminate`）。
- 练习：这样对吗？对，因为 `promise` 在线程退出后析构（因而已就绪）。

且 `future` 仍引用共享状态（即引用计数为 1），故 `.get()` 安全。

`.get()` 之后，共享状态被销毁。

# Future-Promise 模型（Future-promise model）

- 最后，`std::(shared_)future<T>` 与 `std::promise<T>` 对 `T&` 与 `void` 有特化。
- 对 `T&`，如同存储 `T*`；因此差异为：
- `std::promise::set_value(_at_thread_exit)` 接受 `T&` 并存储地址；
- 对 `T` 它接受 `const T&` 或 `T&&` 并转发。
- `std::future::get()` 返回 `T&` 而不是 `std::move` 到 `T`。
- 对 `void`，`get` 无返回值，`set` 无参数。
- 它有时用作互斥量 + 条件变量 + 标志的包装。
- 例如：

# 多线程（Multithreading）

- 异步操作的高层抽象（High-level abstraction of asynchronous operations）
- Future-Promise 模型（Future-Promise model）
- `packaged_task`
- `async`

# `packaged_task`

- 有时仍觉得使用 future-promise 不便……
- 毕竟工作者必须接受 `promise` 作为参数，且仍需手动设置结果。
- 为何不能像普通函数那样直接返回或抛出异常？
- 这就是 `std::packaged_task` 的作用！就像普通函数一样返回或抛出异常。

1. 用函数创建任务（类似 `std::move_only_function`）；注意目前 MS-STL 存在一个缺陷：不能用仅可移动的函数对象构造 `std::packaged_task`；但为维护 ABI，长期未解决。
2. 取得异步结果。
3. 将函数传给工作者；

# `packaged_task`

- 本质上，它包装了一个 `std::promise` 与一个函数对象。
- 它定义了 `operator()`，因此可作为线程的第一个参数作为函数对象。
- 调用它等价于用返回值（或抛出的异常）设置底层 `promise`。
- 若你想与 `DoCleanUp()` 并行：

由于 `operator()` 非 `const`，因为它会修改底层 `promise`。

# `packaged_task`

- 当然，可以 `xx_at_thread_exit`：
- `operator()` 等价于 `.set_value`/`set_exception()`；
- `.make_ready_at_thread_exit()` 等价于 `.set_value`/`set_exception_at_thread_exit()`；
- 注意 `packaged_task` 的析构函数也会抛弃共享状态（尽管此处所有标准库实现检查的是“是否已设置结果”而非“是否就绪”）。
- 注意：调用 `std::packaged_task` 不会释放共享状态。
- 仅默认构造/移动后会使其不关联状态。
- 可用 `.valid()` 检查。
- 无共享状态时的任何函数调用导致异常（`no_state`）。
- 但不能调用 `packaged_task` 两次（`promise_already_satisfied`）。
- 也不能两次调用 `.get_future()`（`future_already_retrieved`）。

# `packaged_task`

- 更方便地，任务调用后可用 `.reset()` 恢复共享状态。
- 确切地说，它会先抛弃原状态，再建立新状态。因此：
1. 若无初始状态，则为 `no_state`。
- 因此此处不能 `std::move(task)`，因为移动后的 `task` 无状态，无法 `.reset()`。
2. 重置可能未完成的任务是错误的。
- `.reset` 会抛弃状态（即对未就绪的 `promise` 调用带 `broken_promise` 的 `set_exception()`）；但若任务稍早完成，又会设置另一结果，从而导致 `promise_already_satisfied`。
- 此外，若无 `future` 引用原状态，抛弃会释放它并可能导致非法内存访问。
3. 应再次调用 `.get_future()` 以取得新状态的 `future`；原 `future` 仍关联旧状态。

# 多线程（Multithreading）

- 异步操作的高层抽象（High-level abstraction of asynchronous operations）
- Future-Promise 模型（Future-Promise model）
- `packaged_task`
- `async`

# `async`

- 有时仍觉得 `std::packaged_task` 不便……
- 毕竟我们需要把任务分派到线程。
- 我们可能想要真正高层的基于任务的视角：我们有一个任务，希望在未来取得结果。我们不关心如何、何时执行，只需在需要时取得结果。
- 这就是 `std::async()` 所做的！
- 只需两行：
- 它几乎屏蔽了我们在 future-promise 或 `std::packaged_task` 中关心的所有细节；它完整表达了任务的概念。
- 你最多能做的是设置策略：

# `async`

- 它只是一个有作用域的枚举：
- `std::launch::async`：强制在新线程上执行任务；
- 若无法启动新线程，则抛出带错误码 `std::errc::resource_unavailable_try_again` 的 `std::system_error`。
- `std::launch::deferred`：推迟执行，仍在同一线程执行。
- 即，仅当你对返回的 `future` 调用 `.get`/`wait()` 时才执行。
- 注意：由于推迟的任务在 `.get`/`wait()` 之前永不执行，`.wait_for`/`wait_until()` 会立即返回并返回 `std::future_status::deferred`。
- 对其他 `future` 来源（`std::promise` 或 `std::packaged_task`），只能返回 `std::future_status::ready`/`timeout`。
- 默认（重载 (1)）为 `std::launch::async | std::launch::deferred`。
- 意味着由系统决定使用 `async` 还是 `deferred`。
- 对其他由实现定义的标志，行为也由实现定义。

# `async`

- 此外，`async` 任务只有两种情况：全有或全无。
- 即，一旦任务开始执行，就会一直执行到完成。
- 对推迟策略，这很自然：
- 若从不调用 `.get`/`wait()`，则为“无”；
- 若调用它们，则为“全”。
- 对 `async` 策略，由于任务会立即执行……
- 其 `future` 的析构会延迟到任务完成之后。
- 练习：这两个 `Work` 会并行执行吗？
- 不会！返回的 `future` 被忽略，作为临时对象，它在语句结束后立即析构。
- 因此 `Work(2)` 仅在 `Work(1)` 完成后才开始。
- 应改为：

# `async`

- 注意 1：在实现中，不同 `future` 行为源于共享状态。
- 尽管都叫“共享状态”，实际上是多态的；`future` 分配状态（通常在堆上 `new`）并存储指向基类的指针。
- 注意 2：带 `async` 策略的 `std::async` 更适合玩具示例，因为所有细节都被屏蔽。
- 在 MS-STL 中，任务被高效分派到内部线程池；
- 但在 libstdc++ 中，每个新任务都会启动新线程，因此大量任务会显著拖累性能。

# 小结（Summary）

- 线程（Threads） | - 条件变量（Condition variable）
- 抽象线程模型（Abstract thread model）。 | - 闩与屏障（Latch & barrier）
- `std::thread` | - 异步操作的高层抽象（High-level abstraction of asynchronous operations handling）。
- `std::jthread` 与 stop token | - Future-Promise 模型（Future-promise model）。
- `std::exception_ptr`、`thread_local`、 | - `std::packaged_task`
- `static`/`std::call_once` | - `std::async`
- 同步工具（Synchronization Utilities）
- 信号量（Semaphore）
- 互斥量与锁（Mutex and lock）
- 共享、递归、定时（Shared, recursive, timed）
- 死锁避免——`std::scoped_lock`/`std::lock`

# 下一讲……（Next lecture…）

- 我们将讨论高级并发。
- 内存模型（memory order）
- 对自学者相当困难。
- 原子变量（Atomic variables）
- 协程（Coroutine）
