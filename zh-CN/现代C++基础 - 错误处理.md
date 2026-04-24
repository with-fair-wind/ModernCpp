错误处理 Error Handling

## 现代C++基础

## Modern C++ Basics

梁嘉铭，北京大学本科生

### • 错误码扩展

### • 异常

### • 断言

### • 调试辅助

### • 单元测试

# 错误码

- 在 C 语言中，若函数可能无法完成任务，调用者通常通过错误码得知这一点。
- 但错误码有许多变体：
- 通过返回值：成功返回 0，内存不足失败返回 1，网络连接失败返回 2，等等。
- 通过特殊值：例如 `scanf` 返回 `EOF`，`fopen` 返回 `nullptr`。
- 通过指针：例如 `strtod(begin, &end)` 且 `end == begin`。
- 它们有时还会与 `errno` 配合，即一个全局错误号，并可用一些函数得到明确的错误信息。
- 例如，若数值不可表示，`strtod` 可能会设置 `ERANGE`。
- 顺便一提，自 C11 起，`errno` 在每个线程上是独立的，因此使用它不会有数据竞争问题。

# 错误码

- 但错误码也有局限：
- 用户常常不得不传入指针来接收“真正的”返回值。
- 用户必须从文档中了解不同函数的习惯，例如有的函数以非零表示成功，有的则以零表示成功。
- 若用户无法就地处理错误，就只能把错误码再返回给上层调用者，这需要手动逐级传递。
- 有时你只需要用 `-1` 表示非法，这会“吃掉”无符号类型取值范围的一半。
- 错误信息通常也很有限。
- ……

# 错误处理

- 那么其他语言如何处理错误呢？
- Go：支持多个返回值，第二个通常是 `err`。
- 你需要通过 `result, _ = xx` 显式忽略它。
- Rust：通过两种机制
- `Result<T, Err>` 用于可恢复错误。
- `panic!` 用于不可恢复错误（带着栈回溯退出）。

- Python 及许多其他语言：异常。
- C++ 提供了类似的方案。

# 错误处理

错误码扩展

# 错误处理

- 错误码扩展
- `optional`
- `expected`

定义于 `<optional>`，自 C++17 起

# Optional

- 首先，`std::optional<T>` 并不严格算是“错误处理”的手段，而是表示“允许且合法地不存在 `T`”。
- 在 C++ 里甚至并不那么推荐用它来处理错误，但它与 `std::expected` 有些相似，所以这里一并介绍。
- 它用一个额外的 `bool` 表示“有值/无值”；对某些本不该为空的类型（例如整数），你可以利用取值空间中的某个值，而不必再引入一个专门的“错误哨兵值”。
- 无值用 `std::nullopt` 表示，本质上让底层那个 `bool` 变为 `false`。
- 例如，你可以给 `std::map` 增加一个 `Get()` 方法，在“键不存在”时返回 `std::optional<T>`，而不必用 `.find()` 再判断迭代器是否等于 `end()`。
- 这正是 Java 的 `Map` / Python 的 `dict` 的做法。

# Optional

- 下面看看它的基本接口！
- 构造函数 / `operator=` / `swap` / `emplace` / `std::swap` / `std::make_optional`。
- 构造函数也可以接受 `(std::in_place, 用于构造 T 的参数…)`。
- 由于类型是确定的，它不会像 `variant` / `any` 那样使用 `std::in_place_type<T>`。
- 默认构造为 `std::nullopt`（类似 `nullptr`，表示“没有值”）。
- 你也可以用 `= std::nullopt`，或 `.reset()` 让 optional 变为空。
- `operator<=>`；类似地，无值被视为最小。
- `std::hash`；与 `std::variant` 不同，`std::optional` 保证：若非 `std::nullopt`，其哈希与 `std::hash<T>` 一致。

# Optional

- 你可以把 `std::optional` 当作可空指针来用：
- `operator->` / `operator*` / `operator bool`，就像 `T*` 一样。
- 若实际是 `std::nullopt`，对 `->` / `*` 的行为是未定义的。
- 你也可以使用 `.has_value()`（与 `operator bool` 相同）和 `.value()`（若为 `std::nullopt` 会抛出 `std::bad_optional_access`）。
- `.value_or(xx)` 可以提供默认值。
- 例如 `std::optional<double> opt{1.0}`，则 `opt.value_or(3.0)` 仍是 `1.0`；若 `opt` 为 `std::nullopt`，则返回 `3.0`。
- 相比指针，optional 本身语义自洽，并且可以绑定临时对象。
- 这仍然很朴素，我们需要更方便的操作！

# Monad

- C++23 的单子（monadic）操作正是为此而来！
- 它来自数学中的范畴论（category theory），并由函数式编程广泛传播。
- 基本上，单子操作会把 `Maybe<T>` 变成 `Maybe<U>`，从而无论是否有值都能把操作链式串起来。
- 这里的 `Maybe` 就是 `std::optional`。
- 有两种操作：一种处理有值情况，另一种处理空值情况；若不匹配，则跳过该步。
- 对有值情况：
- `.and_then(F)`，其中 `F` 接受 `T` 或其引用，并返回 `optional<U>`；
- `.transform(F)`，其中 `F` 接受 `T` 或其引用，并返回 `U`（会自动包装为 `optional<U>{retval};`）。
- 对可空（无值）情况：
- `.or_else(F)`；由于只在 `nullopt` 时调用，`F` 不需要参数，只返回 `optional<T>`。

# Monad

- 例如：

Credit: CppStories.

# Optional

- 注 1：Java 和 C# 里大多数类型默认可空，这使它们“自动 optional”。
- 然而这在许多场景下会带来效率问题。
- 注 2：`std::optional`（以及 `std::expected`）、`std::any` 和 `std::variant` 有时被称为“词汇类型（vocabulary type）”。
- 它们都不能把引用类型作为模板参数（例如 `std::optional<int&>`）。
- 除非 P2988？
- 注 3：虽然 `std::optional` 只多存一个 `bool`，但对齐与填充会让对象实际更大。
- 因此在一个结构体里堆很多 `std::optional` 可能并不理想；更好的做法是把多个 `bool` 与值分开存放。

# 容器

- 错误码扩展
- `optional`
- `expected`（自 C++23 起）

# Expected

- `std::expected` 与 `std::optional` 非常相似，区别在于：
- 它用错误类型（即 `std::expected<T, E>`）而不是“空值”来表示“没有期望的成功值”。
- 并且更推荐用它来表达错误，即“无成功值”意味着非法/失败结果，因此它才是处理错误的正道。
- 就像 Rust 的 `Result`。
- 你可以这样构造：
- 对正常值，直接用 `T`，或 `std::in_place` 加参数。
- 对错误值，可以用 `std::unexpected{xx}`，或 `std::unexpect` 及其参数。
- 也可以使用 `operator=` / `.emplace()` / `.swap()` / `std::swap()`。

# Expected

- 例如：

# Expected

- 注意它只支持 `operator==` / `!=`，不支持 `std::hash`。
- 其他操作：只额外增加了 `.error()` 用于取得错误。
- 而 `.value()` 可能抛出 `std::bad_expected_access`。
- 单子操作：只额外增加了 `.transform_error(Err)`，它把一种错误映射成另一种错误（是的，不是 `E1 -> T2`，而是 `E1 -> E2`）。

# Monad

- 总结如下：
- `<T1,E1>.and_then(T1)` 需要返回 `<T2,E1>`；
- 对 `std::optional`，必须返回 `std::optional<T2>`。
- `<T1,E1>.transform(T1)` 需要返回 `T2`，并自动构造 `<T2,E1>`。
- 对 `std::optional`，必须返回 `T2`，并自动构造 `optional<T2>`。

- `<T1,E1>.or_else(E1)` 需要返回 `<T1,E2>`；
- 对 `std::optional`，必须返回 `std::optional<T1>`。
- `<T1,E1>.transform_error(E1)` 需要返回 `E2`，并自动构造 `<T1, E2>`。

# Expected

- 例如：

# 模式匹配*

- C++ 的匹配能力很弱（基本只能对整数 `switch`），因此难以匹配 `expected` / `optional` / `variant` / ……
- P1371 或 P2688 会解决这个问题。
- 若其中某个被采纳，你就可以例如
- 它们还有更多工具；由于截至目前（2024 年 5 月）尚不确定是否进入 C++26，这里不展开。

# 错误处理

异常

# 错误处理

- 异常
- 基础
- 异常安全
- `noexcept`

# 异常

- 看看 `std::expected` 解决了哪些问题：
- 用户常常不得不传入指针来接收“真正的”返回值。
- 用户必须从文档中了解不同函数的习惯，例如有的函数以非零表示成功，有的则以零表示成功。
- 基本解决：错误类型通常自解释，比“不知从哪来的整数错误码”更友好。
- 有时你只需要用 `-1` 表示非法，这会“吃掉”无符号类型取值范围的一半。
- 错误信息通常也很有限。
- 若用户无法就地处理错误，就只能把错误码再返回给上层调用者，这需要手动逐级传递。
- 仍然需要手动传递！

# 异常

- 异常是一种若未被捕获会自动向上传播给调用者的机制。
- 例如函数调用链 `A -> B -> C -> D`，若 `D` 抛出异常且 `D` 不捕获，则 `C` 需要捕获；若 `C` 也不捕获，则 `B` 需要；依此类推。
- 若 `main` 也不处理，则程序会终止，这等价于调用 `std::terminate()`。
- 这一传播过程称为栈展开（stack unwinding），即栈一层层回到更外层。析构函数会像正常返回那样被调用。

# 异常

- 用 `try`–`catch` 块捕获异常：

若有多种异常需要捕获，可以写多个 `catch`：

1. `.what()` 是 `std::exception` 的虚函数（见后文）。
2. `catch(…)` 表示捕获所有异常，但你无法知道异常的真实类型。

# 异常

- 注 1：只有当你确实能处理异常时才应捕获。
- 也就是说，捕获之后程序仍处于有效状态并能继续正常运行；否则就应终止程序！
- 例如，内存不足会抛出 `std::bad_alloc`；这往往不是你能在局部修复的，那就别捕获。
- 但若你有可能获得更多内存（例如把数据刷回磁盘并释放部分已有内存），那就可以捕获并如此处理。
- 因此通常不鼓励使用 `catch(…)`，因为你几乎只能处理某些特定异常，而不是所有异常。

# 异常

- 注 2：虽然可以抛出任意类型，但推荐抛出继承自 `std::exception` 的类型。
- `<stdexcept>` 与 `<exception>` 中定义了许多通用异常；构造它们通常需要错误字符串。
- `std::bad_optional_access` / `std::bad_alloc` / …… 都继承自 `std::exception`。

继承自 `logic_error`。

继承自 `runtime_error`。

# 异常

- 标准异常的总体视图（截至 C++20）：

注：并非所有异常都只有 `.what()`；例如 `std::future_error` 还会有 `<system_error>` 中定义的错误码。

th Credit: Professional C++ 5ed., Marc Gregoire

# 异常

- 原因：基类也能匹配派生类异常并捕获，因此你总能 `catch(const std::exception&)` 并打印 `.what()` 获取信息。
- 匹配规则略复杂，但若遵守注 2，你通常不必深究细节。

- 自定义异常示例：
- 如有需要也可以重写 `.what()`。

# 异常

- 注 3：`catch` 块按顺序逐一匹配，注意前面的 `catch` 是否“盖住”了后面的分支。
- 通常编译器会给出警告。

- 注 4：虽然允许按值或按引用、`const` 与否来捕获，但异常几乎总应按 `const Type&` 捕获。
- 例如，若按值捕获而不是引用，会发生切片（slicing）问题。也就是说，若你的自定义异常带有更多数据，复制到基类对象会让这些信息丢失；虚函数机制也会失效。

# 异常

- 注 5：若在 `catch` 块中发现仍无法处理已捕获的异常，可以用单独的 `throw;` 重新抛出当前异常。
- 这比 `catch(const T& ex) { throw ex; }` 更好，因为后者可能丢失动态类型信息。

`throw;` -> 位置 1。`throw ex;` -> 位置 2。

- 注 6：若在异常处理内部又抛出异常（例如在栈展开过程中析构函数抛出异常），也会调用 `std::terminate`。

# 错误处理

- 异常
- 基础
- 异常安全
- `noexcept`

# 异常安全

- 异常安全指：当异常被抛出并被捕获后，程序仍处于有效状态并能正确继续运行。
- 按保证强弱，可分为四类异常安全。
- 无保证：糟糕——抛出异常后程序半死不活；可能：
- 泄漏一些资源，例如内存泄漏。
- 破坏程序的不变式假设，例如某函数本应把某个值递增到 32，但抛异常会让状态损坏。
- 破坏内存，例如只写了一部分内存就抛异常。
- 在无保证时，绝不应让程序继续运行；应直接终止。

# 异常安全

- 基本保证：至少程序还能正常运行，无资源泄漏，不变式得以维持，等等。
- RAII 对基本保证至关重要！
- 例如：若 `calculate` 永不抛异常，是否就安全了？是否有基本保证？不——若第二个 `new` 在 `calculate` 之后抛出 `std::bad_alloc`，仍会发生内存泄漏。

Credit: CppCon 2022, Back to Basics: C++ Smart Pointers by David Olsen

# 异常安全

- 另一个例子：

- 这些例子可能很朴素，但真实函数复杂得多，人脑很难穷举所有情况。
- 例如，若有 4 把锁且可能在任意位置返回，你就需要在不同位置分别解锁 4/3/2/1 把锁。

# 异常安全

- 解法：用析构函数！
- 无论何时抛异常，已构造完成的对象总会调用其析构函数。
- RAII（资源获取即初始化）：在构造函数中获取资源，在析构函数中释放。
- 用 `std::unique_ptr` 管理堆内存，而不是裸 `new` / `delete`。
- 用 `std::lock_guard` 管理互斥量，而不是手动 `lock` / `unlock`。
- 用 `std::fstream` 管理文件，而不是 `FILE*` 的 `fopen` / `fclose`。
- 你的类也应遵守这一规则！
- 有问题吗？
- 若构造函数抛异常，析构函数还会被调用吗？
- 还没活，又怎么能死呢？

# 异常安全

- 总之：所有已完全构造完成的成员都会被析构，但对象自身的析构函数不会被调用。
- 注意：成员按声明顺序构造，而不是按成员初始化列表的书写顺序（初始化列表会被重排）。
- 例如：初始化顺序是 `id -> sth -> name`，即便你把 `sth{}` 与 `name{init_name}` 在初始化列表中对调。若 `sth` 的构造函数抛异常，则只会析构 `id`（实际上可能什么也不做）

若 `name` 的构造函数抛异常，则 `sth` 也会被析构。

若 `std::cout <<` 抛异常，则 `sth`、`name` 都会被析构，但 `Person` 的析构函数仍然不会被调用。。

# 异常安全

- 类似地，若基类已完全构造，则基类析构函数也会被调用。
- 不调用当前对象析构函数也可能以微妙方式违反异常安全。
- 例如：

现在正确吗？

若 `someData` 的 `new` 抛异常，则 `ptr1` 会泄漏，因为它的 `delete` 不会被调用。

最佳方案：使用 `std::unique_ptr<int> ptr1`。

问题何在？RAII！

# 异常安全

- 若你必须持有一个表示所有权的裸指针（这本身就很奇怪），就不要在初始化列表里初始化它。
- 仍然要利用 RAII！

- 顺便：`new` 若分配失败并抛出 `std::bad_alloc`（或对 `new[]` 有时是继承自 `std::bad_alloc` 的 `std::bad_array_new_length`），则分配不会发生，因此不会泄漏那块内存。
- 顺便 2：`new(std::nothrow)` 会返回 `nullptr` 而不是抛异常，例如 `new(std::nothrow) int{id}`。
- 但你仍需要检查它（`nothrow` ≠ 无错误）！

# 异常安全

- 容器通过把指针包进基类 / 成员里，使用类似技巧。
- 还记得吗？`std::vector` 有三个指针：`first`、`last`、`end`。
- 许多构造函数需要构造新对象填入已分配内存。
- 若其中之一抛异常，必须确保内存被释放。
- 因此 `std::vector` 把这三个指针包到一个基类里，例如 `std::vector_base`，这样构造函数失败时会调用父类析构函数并释放内存。
- 注意真实实现常见为 `vector_base -> vector_impl`，而 `std::vector` 拥有一个 `vector_impl` 成员。
- 这是为了压缩分配器尺寸，同时让分配器不必作为 `vector` 的基类；细节以后你会更清楚。

- 第三级异常安全是强异常安全。
- 即：若函数抛异常，程序状态会回滚到调用该函数之前的状态。
- STL 的大多数方法遵守强异常安全。
- 例如 `vector::push_back()`：即便在 `push` 第 4 个元素时抛异常，`vector` 的元素集合仍与之前相同（`{1,2,3}` 仍是 `{1,2,3}`）。
- 还记得我们之前的问题吗？

- MSVC 希望在 `.insert()` 上提供一种介于基本与强之间的异常安全（当复制构造函数 / “移动构造函数”可能抛时）；
- 也就是说，若在尾部插入且旋转之前发生异常，则提供强保证。
- 这会略微增加时间成本；gcc 的 libstdc++ 不这么做，只用基本保证换取更好性能。

# 复制并交换（copy-and-swap）惯用法

- 在赋值运算符中维持强异常安全的一种技巧是 copy-and-swap 惯用法。我们假设 `T` 可以
- 例如，实现一个简单的 `vector`。这里默认构造。
- 那么如何实现 `operator=`？
- 正确吗？

# 复制并交换（copy-and-swap）惯用法

- 若 `new` 抛异常呢？
- 虽然没有内存泄漏，但不变式已被破坏！`first_` 与 `end_` 之间的内存被释放了。
- 例如 `{1,2,3}` 被释放，但 `.size()` 仍是 3。
- 现在正确吗？

- 仍然不对！若 `copy(…)` 抛异常（即复制类型 `T` 时抛异常）呢？
- `first_` 变了，但 `last_` 与 `end_` 仍是 `nullptr`，因此 `.size()` 是垃圾值。
- 更重要的是，我们过早 `delete[]` 了 `arr`，内存仍会泄漏……

# 复制并交换（copy-and-swap）惯用法

- 现在这样呢？

- 好，至少满足基本保证。
- 然而，一旦抛异常，原先内容全部丢失，因此没有强保证。
- 有办法吗？
- 只有在所有可能抛异常的路径都安全之后，再 `delete[]`！

# 复制并交换（copy-and-swap）惯用法

- 最终：

- 但……这不是很像复制构造函数吗？
- 解法：copy-and-swap 惯用法

1. 虽定义在类内，ADL 仍能确保它在各处可调用。
2. 这里 `std::swap` 与 `std::ranges::swap` 等价，因为它们只是交换指针。但 `std::ranges::swap` 可以在这里调用自定义 `swap`，而 `std::swap` 不能。
3. 这里的 `noexcept` 是必要的。

# 复制并交换（copy-and-swap）惯用法

- 优点：
- 提供强异常安全。
- 增加 `swap()` 方法，可供用户使用（可能通过间接的 `std::ranges::swap`）。
- 提高代码复用，减少冗余。
- 缺点：
- 先分配再释放，会提高峰值内存。
- 交换成本略高于直接赋值。
- 对性能未必最优，例如若此处 `first_`～`end_` 已有足够容量，则直接复制可能更好。
- 否则我们需要额外分配却得到更小的容量！
- 因此标准容器不采用它，只做基本保证。
- 是否使用 copy-and-swap 惯用法由你的设计决定。

# 异常安全

- 构造函数里还有一点……
- 似乎无法在初始化列表捕获其中的异常。
- 即这里 `str{ init_str }` 抛异常；
- C++ 提供函数 try 块（function-try-block）来捕获它们。

- 与普通 `try`–`catch` 的区别：`catch` 必须重新抛出当前异常或抛出新的异常。若 `catch` 语句不这么做，运行时会自动重新抛出当前异常。
- 原因：成员构造失败意味着对象仍不完整。
- 在 `catch` 块中，你也不应使用任何未初始化的成员。

总之，让你知道一下；由于限制较多，我从不使用这个特性。

- 总之，容器的异常安全可概括为：
- 所有只读操作与 `.swap()` 完全不抛。
- 这不包括刻意抛异常的情况，例如 `vector.at(index)` 在 `index >= size` 时抛 `std::out_of_range`。
- 对 `std::vector`，`.push_back` / `emplace_back()`，或 `.insert` / `emplace` / `insert_range` / `append_range()` 仅在尾部插入单个元素时提供强异常安全。
- 对 `.insert` / `emplace` / ……，若你能保证复制 / 移动构造与赋值 / 迭代器移动不抛，则仍是强异常安全。（为什么？）
- 类似地，`.shrink_to_fit` / `reserve` / `resize()` 只需要移动构造函数不抛。
- 否则只有基本异常安全。
- 类似地，你能判断 `.pop_back()` 或 `.erase()` 吗？
- 对 `std::list` / `forward_list`，全部是强异常安全。
- 对 `std::deque`，与 `std::vector` 类似，并增加在前端 `push`。
- 对关联容器，`.insert` / …… 插入一个结点 / 仅单个元素具有强异常安全。（`erase` 呢？）
- 无序容器的 `.rehash()` 也有强保证。
- 其他情况为基本异常安全。

# 错误处理

- 异常
- 基础
- 异常安全
- `noexcept`

# noexcept

- 最强的异常安全当然是不抛异常保证（nothrow exception guarantee）。
- 若一个方法被认为永不抛异常，则可以加上 `noexcept` 说明符。
- 例如：

- 若函数标了 `noexcept` 却仍抛异常，则会调用 `std::terminate`。
- `noexcept` 也是运算符，例如 `noexcept(v1.push_back(xx))` 求值为 `false`，因为它可能因重分配而抛异常。
- `noexcept` 有助于某些优化；我们会在移动语义部分更详细讨论 `noexcept`。

# noexcept

- 重要：标准库默认假定析构函数与释放操作是 `noexcept` 的；你必须遵守。
- 若所有成员的析构函数都是 `noexcept`，则析构函数是默认 `noexcept` 的唯一一类函数（无需显式写说明符）。
- 若所有对应成员的特殊成员函数也是 `noexcept`，编译器生成的构造 / 赋值运算符也会是 `noexcept`。
- 但非默认版本需要显式 `noexcept`。
- 这里指的是 `=default` 或仅使用类内成员初始化器；`A(){}` 并不是编译器生成版本。
- 你可以用 `std::is_nothrow_constructible_v<A, Args…>` 来测试。
- 自 C++17 起也支持 `noexcept` 函数指针，例如 `using Ptr = void(*)(int) noexcept; Ptr ptr = square;`
- 它可以隐式转换为 `void(*)(int)`，但反向不行。

# noexcept

- 虽然 `noexcept` 可能带来一些优化，你也不必到处乱加。
- 一般规则：对普通方法，只有操作显然不抛时才加 `noexcept`。
- 例如容器中的只读方法（如 `.size()`）会标 `noexcept`。
- 甚至 `.pop_back()` 虽然从不抛，也不标……
- 那 `map.find()` 呢？取决于比较是否可能抛！因此仍不标 `noexcept`。

- 除此之外，`swap` 也几乎总应是 `noexcept`。
- 还有一些更特殊的情形，以后再说。

# 何时使用异常？

- 看起来异常全是优点，那是否该到处用？
- Java / C# 一类语言往往就是这样！
- 那么古尔丹，代价是什么呢？
- 与其他错误处理机制相比，异常相对昂贵，例如它往往依赖堆分配等。
- 因此你需要确保抛异常是罕见路径。
- 以游戏为例：运行时玩家不喜欢卡顿；因此对用户错误输入最好不要抛异常。
- 但对初始化阶段则可以，因为玩家本来就会等待；若配置文件不存在之类，用异常很方便。
- 一句话：不要让热路径依赖异常。

特别提醒：异常对“快乐路径（happy path）”通常没有额外开销。参见：C++中异常的额外开销体现在哪些方面？- 江东某人的回答 - 知乎。

# 何时使用异常？

- 此外，在当前典型实现里，异常的栈展开往往需要全局锁，这对多线程程序很不友好。
- GCC 13 在这方面有所改进，归功于 Thomas Neumann（他批评异常机制但仍改进了它）；通过 B-tree 让所有线程可以并行操作。
- Clang 与 MSVC 目前似乎尚未如此。
- 也有“确定性异常（deterministic exception）”提案以提升性能，但那是巨大的语言层面变化，近期未必能被接受。
- 另外，异常高度依赖平台（就像 RTTI）；若你希望捕获来自共享库抛出的异常，需要确保工具链一致。

# 何时使用异常？

- 总结：
- 优点：
- 通过栈展开传播；只在方法能处理时才处理异常。
- 迫使程序员关注错误（否则终止程序）。
- 缺点：
- 不适合性能关键段；不适合热路径。
- 跨模块 `try`–`catch` 不方便。
- 许多编译器对多线程场景的异常优化不足。
- 其实还有一点：代码体积可能膨胀（不过目前通常不太在意）。
- 顺便：
- 若你从事嵌入式 / 操作系统开发，往往没有“栈展开”这回事，因为它依赖操作系统，因此通常应始终关闭异常。
- 例如在 gcc 使用 `-fno-exceptions`。
- 若你在写供其他语言（如 C）使用的库，也需要在接口边界捕获所有异常，因为它们多半无法处理 C++ 异常。

我们不展开 `<exception>` 里的 `std::uncaught_exceptions` 与 `std::nested_exception`。需要时再查。

# 错误处理

断言

# assertion

- 断言是一种在调试阶段检查函数或代码段是否符合预期行为的手段。
- 例如，在 Tensorflow 代码中：
- 当参数求值为 `false`，程序会中止。
- `std::abort()` 是 `std::terminate()` 的默认行为，但后者可用 `std::set_terminate_handler(…)` 改变行为。
- 它是 `<cassert>` 中定义的宏；只接受一个参数，因此多个布尔条件应用 `&&` 连接。

### • 还记得吗？宏会直接按逗号切分参数（例如 `Func<int,double>(xx)`），所以

有时你会额外加一层括号，例如 `assert((Func<int,double>(xx) && "Error: xxx"))`。

# assertion

- 有时你也会看到 `assert(false);` 这表示当前代码路径永远不应被执行。
- 特别地，该检查仅在未定义宏 `NDEBUG` 时进行（类似 VS 的 Debug 模式）；否则该宏什么也不做（等价于 `(void)0`）。
- 例如，你不应写 `assert(SomeImportantWork(xx))` 即便 `SomeImportantWork` 返回 `bool`；当宏启用时，这个函数根本不会被执行。
- 你只应放入无论是否执行都没有副作用的条件检查。
- `assert` 在运行时检查；若要在编译期判定，可用关键字 `static_assert(xx, msg)`。
- 例如 `static_assert(sizeof(int) == 4, "SomeInfo");` 违反则编译错误。
- 自 C++17 起 `msg` 可省略。

# assertion

- 何时使用断言？
- 对任何可能的输入，都不该用断言检查；而应通过 `std::expected` / 异常向调用者报告。
- 对某些内部方法（通常用户不能调用），若非法输入本应已被过滤，则可用断言防止你无意中违反这些方法的假设。
- `assert` 帮你在 Debug 模式尽早发现问题！
- 还记得我们说过的 `[[assume(xx)]]` 吗？你可以在它的前面加断言以确保成立。
- 注意 C++23 在 `<utility>` 增加了 `std::unreachable()`，功能与 `[[assume(false)]]` 相同。

# Contract*

- 断言用于安全检查其实很有限。
- C++26 很可能加入契约（contracts），会丰富很多。
- P2900；事实上契约从 2006 年起就在 C++ 委员会讨论。
- 三部分：
- 前置条件、后置条件、断言（assertion）
- 进入函数前检查前置条件；离开函数后检查后置条件。
- 用户可用例如编译器选项控制行为。
- Ignore——不检查，零开销；
- Observe——违反时调用处理程序，但继续运行。编译期违反会给出警告。
- Enforce——违反时调用处理程序然后中止。编译期违反会产生编译错误。

# 错误处理

调试辅助

# 错误处理

- 调试辅助
- `source_location`
- `stacktrace`
- 调试

# source_location

- 通常我们希望记录错误信息时带上源码位置，从而快速定位问题。
- 在 C++20 之前，可能用宏 `__FILE__` 与 `__LINE__`，它们会被替换为源文件名与行号。

# source_location

- 自 C++20 起，增加 `<source_location>` 来解决。
- 可以直接用默认值；非常方便！

- 一个 source location 有四个方法：

# source_location

- 例如：

# 错误处理

- 调试辅助
- `source_location`
- `stacktrace`
- 调试

# stacktrace

- 自 C++23 起，位于 `<stacktrace>`。
- 类似 `source_location`，你需要 `std::stacktrace::current` 来获取；但你可以直接打印它！完整参数：`current(skip = 0, max_depth = total_size(), allocator = default)`，只保留 `[skip, max_depth)` 区间内的条目。

注：有些函数可能被优化掉，因此在 `-O3` 下可能没有名字甚至没有条目。

# stacktrace

- `stacktrace` 动态分配，因此也可指定分配器（`std::basic_stacktrace<alloc>`；`stacktrace` 使用默认分配器）。
- 它是一个受限的顺序容器，元素类型为 `std::stacktrace_entry`；
- 但它作为“容器”未必好用，若感兴趣请看其方法文档。
- `stacktrace_entry` 有 `description()`、`source_line()`、`source_file()` 用于查看信息。
- `stacktrace_entry` 与 `stacktrace` 也支持哈希、比较以及 `std::to_string()`。

# 错误处理

- 调试辅助
- `source_location`
- `stacktrace`
- 调试

# 调试*

- 自 C++26 起，位于 `<debugging>`。

- `breakpoint()` 会设置一个隐藏断点；调试器会识别并在该处停下。
- 当条件非常复杂时很有用，例如 `CheckThis() % 2 == 0 && CheckThat() && …`。
- 调试器也能在条件上暂停，但复杂条件很麻烦，写进代码往往更好。

# 错误处理

单元测试

# 单元测试

- 测试是编程中最重要的技术之一。
- 随着复杂度上升，很难保证每一部分代码都正确；我们用测试来验证。
- TDD——测试驱动开发（Test-Driven Development）。
- 通常更偏好结对编程：一名程序员实现功能，另一名写测试并评审代码。
- 作为学生，我们可能无法结对编程；不过你可以利用 AI 编程助手来帮忙。

# 关于 GPT*

- 我对 GPT 或 AI 编程助手（如 Copilot）的态度：
- 学习新知识时，例如做本课程的作业，非常不鼓励用 AI 代劳。
- Stephen Krashan 与 Noam Chomsky：“使用 ChatGPT 是一种逃避学习的方式”。
- 若你已很熟练地写 C++，那另当别论。
- 我个人不常用 AI 编程助手，因为它会打断思路；但我愿意用它生成测试。
- GPT 也可能误导甚至完全错误，尤其对最新的 C++ 标准。

# 单元测试

- 测试种类很多；
- 通常我们应为每个单元（通常每个类 / 每个源文件）编写单元测试。
- 之后需要集成测试把单元组合起来。
- 最后可能还要做系统测试覆盖整个系统功能。

- 本课程会介绍一个 C++ 单元测试框架——Catch2。
- 它目前是第二流行的框架。
- 还有很多类似框架，如 Google Test、Boost Test、Visual C++ Testing Framework 等。需要时自行学习。

# Catch2

- `xrepo install Catch2`。本项目要求用户安装 Catch2。
- `xmake.lua`：在每个 target 中使用 catch2

当我们不需要用户提供 `int main` 时，需要在 MSVC 中显式指定。

# Catch2

- `test.cpp`：

- 若改成 `1 + 2 != 3`：

# Catch2

- 你也可以在命令行使用：
- 这表示运行名为 “Test Name” 的测试。

# Catch2

- 现在介绍一些细节。
- `REQUIRE` / `CHECK`：前者会停止当前区域内后续测试，后者不会。
- 当前面的内容是后面内容的前置条件时，应使用 `REQUIRE`。

- 还有 `REQUIRE_FALSE` / `CHECK_FALSE` 用于断言为假。
- 也可用 `REQUIRE_NOTHROW` / `CHECK_NOTHROW` 检查表达式永不抛异常。

# Catch2

- 甚至可用 `REQUIRE_THROW_AS` / `CHECK_THROW_AS(exp, ExceptionType)` 检查抛出的类型。

- `TEST_CASE` 也可使用两个参数 `(name, tag)`。
- 标签可用于把不同 `TEST_CASE` 分组。
- `[]` 是必需的。

# Catch2

- 你也可以给测试用例附加多个标签，例如
- `[tag1],[tag2]` 表示按或过滤，`[tag1][tag2]` 表示按与过滤。
- 这里 `[physics],[math]` 会测试所有用例，而 `[physics][math]` 只会测试 “Test4”。
- 注：标签应以字母开头（即 `a-z` `A-Z`）。
- 否则保留给特殊过滤器。

# Catch2

- 有时一个测试用例可分成多个 `SECTION`，以共享相同初始化。
- 外层语句会再次执行。
- 就像栈：执行

执行 `CHECK(i==1)``CHECK(i==2)`

`CHECK int i = 1;``CHECK(i==1)` `(i++==1)`

TestCaseSub1Sub2

- `SECTION` 也可嵌套；原理仍像栈。

# Catch2

- 注 1：Catch2 使用宏，因此仍记得为逗号加括号。
- 注 2：Catch2 重载了 `operator&&` 与 `operator||`，使用它们不会短路求值。
- 因此你可能需要拆成多个 `REQUIRE`。
- 注 3：`SECTION` 可动态生成；只需确保名字不同。例如：

# Catch2

- 你也可以用日志宏打印信息。
- 测试失败时，会打印当前作用域内所有日志。
- 4 个级别：`INFO("xx")` / `WARN("xx")` / `FAIL_CHECK("xx")` / `FAIL("xx")`；
- `INFO` / `WARN`：只是记录信息。
- `FAIL_CHECK`：记录错误并视为测试失败（类似 `CHECK(false)`）。
- `FAIL`：同 `FAIL_CHECK`，并终止当前区域（类似 `REQUIRE(false)`）。。
- 还有两个特殊宏：
- `SUCCESS("xx")`：记录成功并把当前视为成功测试。
- `SKIP("xx")`：记录并跳过当前作用域。
- 这些宏本质上是流，因此你也可以 `INFO(1 << "wow")`。

# Catch2

- 你也可以用不同输入批量测试：
- 针对不同模板参数的模板测试，定义在 `<catch_template_test_macros.hpp>`。

- 你需要用 `"Name - Type"` 指定名字（空格很重要），例如 `"TemplateTest - int"`；因此标签在这里很重要。

# Catch2

`SECTION("sub1")` 等同于：
- 或生成大量数据：
- Generator 定义在 `<generators/catch_generators_all.hpp>`
- Generator 可看作虚拟 `SECTION`，直到当前作用域结束；在每条执行路径的栈上，生成器初始化为单个值。

# Catch2

- Generator 也可从容器或迭代器对构造，通过 `Catch::Generators::from_range(…)`。
- 你不能直接传入 C++20 range；用 `std::ranges::to` 转换。
- Catch2 提供类似 ranges 的生成器，但既然 C++ 标准库已提供，这里就不赘述。

- 注意：若你想使用当前作用域里的变量，也可以用 `GENERATE_REF`。

# Catch2

- 最后，你也可以做基准测试：
- 基准测试通常需要预热，使代码段处于更接近真实环境的状态（否则缓存冷启动等会影响性能）。
- 你也可能需要重复运行基准测试取平均结果。
- Catch2 的基准测试会自动做这些事！

`<benchmark/catch_benchmark.hpp>`

# Final word

- 注 1：若你真的想要 `main` 做其他事，可以用 `<catch_session.hpp>` 里的 `Catch::Session`。或者你想提供命令行参数：

- 注 2：这些头文件也可用一个总头 `<catch_all.hpp>` 引入。
- 注 3：Catch2 还提供随机种子；可用 `Catch::getSeed()` 获取。

# Summary

- 错误码扩展；异常应极少发生。
- `std::optional` / `std::expected`；断言（Assertion）
- 单子（Monad）操作；`assert` 与 `static_assert`
- 异常；调试辅助
- 栈展开；`source_location` 与 `stacktrace`
- `try`–`catch` 块
- 单元测试
- `std::exception`
- Catch2
- 异常安全
- copy-and-swap 惯用法
- `noexcept`
- 析构函数不应抛出异常。
- 何时使用异常——确保它很少发生

# 下一讲……

- 下一讲将覆盖字符串与流。
- 包括 `std::string` 与 string view，以及 C++ 中的 Unicode 支持
- 更重要的是 `std::format` 与 `std::print`！
- 深入流……
- 最后是正则表达式！
