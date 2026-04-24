容器、ranges 与算法 Container, Ranges and Algorithm

## 现代C++基础

## Modern C++ Basics

梁嘉铭，北京大学本科生

### • 第二部分

### • Ranges

### • Generator

### • Function

### • Algorithms

# 在那之前……

- 我们需要先谈谈 `std::unique_ptr`。
- 要彻底理解它，你需要学习移动语义与内存管理；这里先给一个简单介绍。
- 它是“智能指针”之一，也就是利用 RAII 来管理资源。
- 以前若要在堆上分配对象，你会使用 `new`。
- 然而很容易忘记 `delete`，尤其当函数里有很多返回点时……
- 你可以把 `std::unique_ptr` 想成：构造函数里 `new`、析构函数里 `delete`，这样离开函数时会自动触发。
- 顾名思义，它对资源拥有唯一所有权。
- 也就是说，你不能把它拷贝到另一个 `std::unique_ptr`；否则析构会 `delete` 两次指针，这是未定义行为（UB）。
- 你需要移动它：一方放弃资源所有权，并把它交给另一方。
- `std::unique_ptr<T> ptr{ std::make_unique<T>(params) }, ptr2;`
- `ptr2 = std::move(ptr);`

# 容器、ranges 与算法

Ranges

# Ranges

例如：为什么在 `std::views::join` 上迭代会那么慢——Stack Overflow；递归的 `join` 非常慢。
- 你可能会觉得烦，当：
- 在基于范围的 `for` 里不能指定其它操作，也就是你必须遍历所有元素、遍历多个容器。
- 在例如 `std::sort()` 里不得不一遍遍写 `xx.begin()`、`xx.end()`……
- Ranges 会解决这些问题！
- 使用 ranges 很像函数式编程。
- 它现在仍有不少问题：
- 优化并不完美，也就是可能比手写普通循环稍慢。
- 与 ranges 相关的基础设施支持有限。
- 用户自定义比较困难。
- 它很有用，但我认为在 C++26 之前未必足够成熟到可以广泛采用。不过你目前可以在自己的项目里尝试！
- 注意：我们讲授的内容基于 C++23；C++20 的 Ranges 太受限且 bug 很多。

# Ranges

- Ranges 里有三个重要组成部分：
- Range：提供 `begin` 迭代器与 `end` 哨兵（sentinel）的类型，从而可以被遍历。
- `std::begin/end()`，语义上复杂度摊还为 𝑂(1)。
- 容器显然是一个 range。
- View：可在 𝑂(1) 内移动、在 𝑂(1) 内拷贝（或不可拷贝）、在 𝑂(1) 内析构的 range（不考虑元素的析构）。
- 顾名思义，“view”通常是一种查看元素的类型（像我们讲过的 `span`）。
- 它也是 range 的一种，但相对容器通常是非拥有的（non-owning）。
- Range adaptor（范围适配器）：
- 能把一个 range/多个 range 变换成 view 的函数对象。由于 view 也是 range，它可以把 view 变成 view，并继续用新的 adaptor 变换。
- 你可以用运算符 `|` 把 range 与 range adaptor 连接起来，构成一条流水线。

# Ranges

- 注 1：按迭代器划分，range 也有 input/output/forward/bidirectional/random_access/contiguous_range 等概念。
- 注 2：由 range adaptor 生成的 view 的一个重要特征是：计算是惰性的（lazy）。
- 也就是说，只有当你想要新 view 的下一个值时，计算才会发生。
- 例如你想从一个 `vector` 里取出所有小于 5 的元素。
- 如果你先把所有元素都检查一遍，写进新的 `vector`，再用新 `vector` 做别的事，那就是急切（eager）求值。
- 但 range 只提供一种遍历方式：当用户要第一个小于 5 的元素时，你可以扫过 `vector` 并在第一个满足条件的元素处停下；当用户要下一个时，再继续扫。
- 这会给用户一种错觉：仿佛你已经拥有所有满足条件的元素。

# Ranges

- 注意：range 与 view 都是 concept；也就是说，它们只规定类型应当满足什么条件，本身并不是某个具体类型。
- 并不存在 `range a;` 这种东西；我们只能说例如 `vector` 是一个 range。
- 它们都定义在 `<range>`；所有 view 都定义为 `std::ranges::xx_view`，所有 range adaptor 都定义为 `std::views::xx`。
- 个人习惯：用别名 `namespace stdr = std::ranges`、`namespace stdv = std::views`。
- 也有一些 range factory，例如：
- `stdr::iota_view{lower, upper = INF}` / `stdv::iota(lower, upper=INF)`：从 `lower` 开始不断 `++` 直到 `upper`（即 `[lower, upper)`）。
- 如果你用过 Python，它很像 `range(a, b)`，只不过在 C++ 里 `stdv::iota(a)` 表示 `[a, INF)` 而不是 `[0, a)`。
- 我们稍后会再讲更多……

# Ranges

- 那么，先看看 ranges 能怎么用！

- 下面产生一个 view，它：
- 先取一个从 1 递增到 10（不含）的数列。
- 然后过滤，只保留奇数。
- 若是偶数，就不会进入下一个 range adaptor。
- 最后只取前 3 个结果。
- 注意任意 range 都合法；例如你可以把 `stdv::iota` 换成 `std::vector<int>`。

# Ranges

- 有些 view 是只读的，也就是你不能修改 view 所引用的元素。
- `std::iota_view` 就是这样，因为它只存一个整数，并没有真正的“整数序列”。
- 因此你不能用 `auto&` 去取元素。
- 有些 view 是可写的。
- 例如若改成 `v | xx`，其中 `v` 是 `std::vector<int>`，由于整数序列真实存在，指向它的 view 是可写的。
- 你可以用 `auto& num`，修改 `num` 也就是修改 `vector` 里的原元素。
- 但 `const std::vector` 仍然是只读的。
- 单个只读 view 会让后续 view 与用户只能得到只读元素。

- 下面介绍一些 range adaptor！
- 可写：
- `stdv::filter(Pred)`：若谓词函数 `Pred` 返回 `false` 就丢弃该元素。
- `stdv::take(x)`：取前 `x` 个元素（但不会越过 `end`）。
- `stdv::take_while(Pred)`：一直取元素直到 `Pred` 返回 `false`（或到达 `end`）。
- `stdv::drop(x)`：丢弃前 `x` 个元素。
- `stdv::drop_while(Pred)`：一直丢弃元素直到 `Pred` 返回 `false`。
- `stdv::reverse`：反转元素；要求 range 是 `bidirectional_range`。
- `stdv::keys`：从类元组（tuple-like）对象中取第一个元素。
- 特别地，当你遍历 `map` 时，由于 key 是 `const`，view 仍然是只读的。
- `stdv::values`：从类元组对象中取第二个元素。
- `stdv::elements<i>`：从类元组对象中取第 `i` 个元素。
- `stdv::stride(k)`：以 `k` 为步长迭代。
- 例如 `std::iota(1, 10) | std::stride(2)` 得到 `{1, 3, 5, 7, 9}`。
- 这是 Python `range(a, b, k)` 的推广，后者只对整数，而这里可对任意 range。

- 还有一些 adaptor 会组合多个 range 和/或返回元组（仍可写）。
- `stdv::join`：若 range `R` 的元素本身也是 range；这要求它们的元素类型相同，并把多层 range 压平成单一 range。
- 例如 `std::vector<std::vector<int>> v{ {1, 2}, {3, 4, 5}, {6}, {7, 8, 9}};`，`v | stdv::join` 得到 `{1, 2, 3, 4, 5, 6, 7, 8, 9}`。
- `stdv::join_with(xx)`：类似 `join`，但在间隔处填入 `xx`。
- 例如 `std::vector<std::vector<int>> v{ {1, 2}, {3, 4, 5}, {6}, {7, 8, 9}};`，`v | stdv::join_with(10)` 得到 `{1, 2, 10, 3, 4, 5, 10, 6, 10, 7, 8, 9}`。
- 这在拼接字符串时更有用，例如 `std::vector<std::string> v{"Are", "you", "Okay?"}`，`v | stdv::join_with(' ')` 得到 `"Are you Okay?"`。
- `stdv::zip(r1, r2, …)`：把多个 range 的值 zip 成元组，例如 `auto [a, b, c] : stdv::zip(v1, v2, v3)`。
- 与 `join` 不同，它以参数形式接受多个 range，而不是 `operator|` 接一个“range 的 range”。
- 若被 zip 的 range 长度不同，会在最短的那个到达末尾时结束。
- `stdv::cartesian_product(r1, r2, …)`：返回这些 range 的笛卡尔积中元素组成的元组。
- 例如 `1, 2 × 3, 4 = { {1,3}, {1,4}, {2,3}, {2,4} }`。

- 于是你可以像这样遍历多个容器：

- 注意：这里虽然用 `auto`，`a, b, c` 都是对各自 `vector` 元素的引用，因为返回类型是 `std::tuple<T&, U&, …>`。
- 你不能用 `auto&`，因为它返回的是临时 `tuple`（尽管 `tuple` 里装着引用……）！
- 你也不能用例如 `const auto`、`const auto&` 让它们只读。
- 你得到的是 `const std::tuple<T&, U&, …>`！
- 但由于引用本身不变（指向的对象相同），`const tuple` 并不会禁止写入，你仍然可以写它们。
- 可以类比指针：`const T*` 会让内容只读，`T* const` 会让指针本身不可变（而引用就像后者）。
- 你需要的是 `std::tuple<const T&, const U&, …>`。
- 你应该额外加上 `| stdv::as_const`。
- 所以事实上，所谓“只读”就是返回 `const T&` 或 `T`，或 `const T&`/`T` 的 `tuple`；“可写”就是 `T&` 或 `T&` 的 `tuple`。
- 记住这一点，你就能在结构化绑定时知道每个元素到底代表什么。

# Ranges

- `stdv::enumerate`：返回 `std::tuple<Integer, T&>`；其中 `Integer` 是索引，其类型与迭代器的 `difference_type` 相关。
- 例如 `std::list l{7, 3};`，`l | stdv::enumerate` 得到 `[0, 7]`、`[1, 3]`。
- 因此 `auto[index, ele]` 仍然会绑定到 `T&`。
- 还有一些 range adaptor 会产生多个 view，而不是单个 view（每个 view 里的元素仍可写）：
- 你需要 `for(auto v : …)` 从结果里取出一个 view，再用 `for(auto& ele:v)` 遍历该 view。
- `stdv::slide(width)`：用宽度为 `width` 的滑动窗口在 range 上滑动。
- 例如 `std::vector v{1, 2, 3, 4, 5};`，`v | stdv::slide(3)` 得到 `{{1, 2, 3}, {2, 3, 4}, {3, 4, 5}}`。
- `stdv::adjacent<width>`：与 `stdv::slide(width)` 相同；
- 差别在于它用模板参数指定宽度，因此略更高效；并且应在编译期可确定。
- 例如 `int a = xx;`，`stdv::adjacent<a>` 是错的，而 `stdv::slide(a)` 是对的。
- `stdv::pairwise`：也就是 `stdv::adjacent<2>`。

# Ranges

- `stdv::chunk(width)`：按宽度 `width` 对 range 分块。
- 例如 `std::vector v{1, 2, 3, 4, 5, 6};`，`v | stdv::chunk(2)` 得到 `{{1, 2}, {3, 4}, {5, 6}}`。
- 若 `size % chunk_size != 0`，最后一块的大小是余数。
- `stdv::chunk_by(pred2)`：按 `pred2` 分块，也就是当 `pred2(*it, *next(it))` 返回 `false` 时结束当前 view。
- 例如 `std::vector v{1, 2, 5, 4, 3, 4};`，`v | stdv::chunk_by(std::less<int>{})` 得到 `{{1, 2, 5}, {4}, {3, 4}}`。
- `stdv::split(xx)`：用分隔符 `xx` 切分 range，是 `join` 的逆操作。
- 例如 `std::string str{ "Are you Okay?" };`，`str | stdv::split(' ')` 得到 `{ "Are", "You", "Okay?"}`。
- `stdv::lazy_split(xx)`：类似 `split`，但 `split` 只接受 `random_access_range` 且会尽量保持 range 类别；而 `lazy_split` 的结果总是 `forward_range`。
- 因此对 `lazy_split` 生成的 view，不能调用 `.size()`，也不能转换成随机访问 range（例如用于 `sort`）。

# View 的缓存行为

- 有些 view 在你调用 `begin()` 后可能会缓存 `begin` 的提示信息。
- 显然，这是给那些无法直接拿到 `begin` 的情况用的，例如 `filter` / `drop_while` / `split` / `chunk_by`，以满足摊还 𝑂(1) 的要求。
- 特别地，`drop` 只对非随机访问或无法确定大小（也就是没有 `.size()`）的 range 才缓存，因为它需要多次 `++` 才能到达起点。
- `slide(n)` 类似，但它只需维护 `[begin, end)` 长度为 `n`，因此会对某些双向 range 做优化，转而缓存 `end` 而不是 `begin`。
- 我们稍后会解释“某些”指什么。
- 这意味着：带这些缓存型 adaptor 的 view，在调用 `begin` 前后行为可能不同，有时违反直觉！注意 `lazy_split` 不会像 `split` 那样缓存整个 range 的 `*begin`；它把比较推迟到对 `*begin` 的迭代（这也是它只能是 `forward_range` 的原因！）。

# View 的缓存行为

- 本质上，它们缓存的是迭代器（至少对 forward range 如此），但这并非强制要求，因此不少实现对随机访问 range 会缓存下标。
- 所以有时建议在调用 `begin` 之后不要再修改容器。
- 对写入：若我们在第一次循环后加上 `v[1] = 1`，那么即使 `1 % 2 != 0`，它也可能不会被过滤掉，因为 `begin` 处曾被判定为满足条件，并且已经被缓存。
- 对插入：若缓存的是下标，在缓存位置之前插入也可能得到错误结果。但其它 range 只要插入后缓存迭代器不失效，仍可能正确。
- 对删除：与插入类似；更严重的是，删除可能导致 `size < offset` 或迭代器被删，从而出现非法内存访问！
- 还要注意迭代器可能失效，例如无序容器是 forward range，因此会缓存迭代器，但 rehash 可能使所有迭代器失效。

# View 的缓存行为

- 此外，缓存使并行更脆弱，因为两个并行 `.begin()`（例如两个循环）可能同时改写缓存结果，造成数据竞争。
- 更重要的是：广泛用来表示只读容器的 `const auto&` 对 view 未必合适。
- 因为 `.begin()` 可能会写入其成员，所以 `const view` 可能根本不能迭代。
- 此外，这种 `const` 是浅层的，因为它只是 view；若你想让底层 range 也只读，应使用 `stdv::as_const`（类似用 `std::span<const T>` 而不是 `const std::span<T>`）。
- 最后，缓存可能被拷贝也可能不会，因此拷贝后的 view 可能与原 view 行为不同。
- 例如若把 `std::list` 换成 `std::vector`，在 MSVC 上缓存会被保留……

# View 的缓存行为

- 总之，view 只保证：在 range 未被修改、且谓词函数不改变任何外部状态（例如打印）时，你会得到相同的元素序列。
- 通常鼓励“就地临时”使用 view，也就是像循环里那样用一次就立刻用完。
- 这样写入元素（若可写 view 需要）才是安全的。
- 或者至少你只读取 view，并保持底层容器不变，也不改变外部状态。
- 否则你需要确保 `.begin()` 尚未被调用，或者非常仔细地关注缓存行为。

# Ranges

- 只读：
- 要么把 view 设为 `const`，也就是 `std::as_const`；这会返回 `const T&` 或 `tuple<const T&, …>`。
- 要么返回值，也就是与 `transform` 相关，会返回 `T` 或 `tuple<T, …>`。
- `stdv::zip_transform(TransformN, r1, r2, …)`：返回 `TransformN(ele1, ele2, …)` 的 view。
- 它只能作为 `operator|` 的起点。
- `stdv::adjacent_transform<N>(TransformN)`：返回 `TransformN(…)` 的 view，其中 `…` 是滑动窗口里的元素（即拆包后的 view）。
- `stdv::transform(Transform)`：把元素变换成另一个元素。
- 这里的 `Transform` 指 `Ret(T)` 或 `Ret(const T&)`；你应当通过返回值变换，例如 `return a + 1`，而不是例如 `T& a → a++`！
- 需要缓存的可写 range 也不应通过 `&` 去改值。
- 对 transform 相关的情形，需要特别小心！
- 结果不缓存，因此需要其值时，可能被调用多次……

# 关于 transform 的注意事项

- 例如：

- 你可能期望输出是 `cnt=0,ele=1` 等，直到 `cnt=4,ele=5`，因为 `4+5+1<10` 为假，所以 `take_while` 会取前四个元素。
- 但不幸的是，实际更像：
- 只取了前三个元素……

# 关于 transform 的注意事项

- 令人惊讶的是：惰性求值并没有那么“惰”……
- 每个 `transform` 会被调用两次；为什么？
- 试试：

- 于是 `++` 会触发 `transform`，因为 `take_while` 需要变换后的值来推进迭代器（也就是通过 `Pred(*it)`）。
- `const auto& ele = *it` 会再触发一次！
- 若你只用 `take(2)`，由于 `take` 不需要变换后的值，`transform` 就恰好执行一次。
- 这是因为结果并不引用某些已有元素，而是从临时量生成；每次需要它时，惰性求值都会再生成一次。
- 你可以认为其它 range adaptor 能缓存原始迭代器，因此求值恰好一次。
- 你需要特别留意 transform 相关的 adaptor，它们的性能可能不理想，因为函数体可能被调用两次！

# Ranges

- 有时你想把一个 range 转换成例如容器，这就需要急切求出全部元素。
- 这时可以用 `stdr::to`，例如 `stdr::to<std::vector>()`。
- 注意这里提供的是模板参数，而不是例如 `std::vector<int>`。
- 自 C++23 起，容器通过支持接受 `(std::from_range_t, Range)` 或 `(Range)` 的构造函数来实现。
- 还有一些朴素的 range factory，这里只列一下。
- `stdv::single(obj)`：生成一个拷贝/移动对象的 view，也就是 view 只拥有一个元素。
- `stdv::empty<T>`：创建空 view。
- `stdv::repeat(r, k = INF)`：把 range `r` 重复 `k` 次，也就是像 `[r, r, r, …]`。
- `stdv::istream<xx>(stream)`：类似 `istream_iterator`；它会缓存值。
- 其它内容以后再讲。

# Ranges

- 你也可以从 range 得到子区间：`stdr::subrange(first, last)`。
- 若迭代器不是随机访问，你也可以把 `(size)` 作为最后一个参数加上，这样以后用 `size()` 时是 𝑂(1) 而不是 𝑂(𝑛)。
- 你必须自己保证 `std::distance(first, last) == size`！
- 能在 𝑂(1) 得到 `size()` 的 range 具有 `sized_range` 概念。
- 具体地，我们说过可以用例如 `std::size()`、`std::begin()` 获取容器大小/迭代器。
- 广义上，`stdr::size/begin()` 更强大。
- 更安全：你不能从临时 range 取迭代器（其迭代器会悬空！）。
- 而且你能可靠地得到迭代器；例如 `std::begin(c)` 只调用 `c.begin()`，但若 `c.begin()` 返回整数而不是迭代器，它仍可能“看起来对”……
- `stdr::begin` 在这种情况下会让编译失败。
- 此外，有些 range 只能用 `stdr::` 版本。

# Ranges

- 注 1：有些 range 没有 `.size()`，若它们无法在 𝑂(1) 确定大小。
- 也就是它们不是 `sized_range`。
- 例如 `iota` 可能是无界的。
- 或例如 `filter` / `take_while` / `lazy_split` / `split` / `join` / `join_with` / ……
- 或某些本身没有 `size` 的 range，例如 `std::forward_list`。
- 注 2：只有 `contiguous_range` 才能调用 `data()`；否则编译错误。
- 这会区分 `split_view` 与 `lazy_split_view`；后者在新 view 上不能调用 `data()`，因为它产生 `forward_range`。

# Ranges

- 由 range factory 与 adaptor 生成的 view 也有诸如以下方法：

CPO 事实上更复杂，因为它与 ADL 有关（本课不讲）；这里只给非常粗略的概念。更多细节见知乎问题《如何理解 C++ 中的定制点对象这一概念？为什么要这样设计？》中 Mick235711 的回答。

# Ranges*

我们不讲 ADL 的原因是它可能在 C++26 完全演变，而此前的 ADL 对新手略显晦涩。若你有兴趣，请参阅《C++ Templates 第二版》第 13 章。
- 现在学习一些 ranges 里的基础概念（可选）。
- Customization Point Object（CPO，定制点对象）：可默认初始化且可拷贝的 `const` 函数对象。
- 同一类里的所有 CPO 彼此等价：也就是 `operator()` 在相同参数下得到相同结果，任意拷贝都不会改变 CPO 的功能。
- Range Adaptor Object（RAO）：把 range 作为首参并接受 range，把它/它们变换成 view 的 CPO。
- Range Adaptor Closure Object（RACO）：支持管道运算符的 RAO，也就是 `RACO(Range)` 等价于 `Range | RACO`。
- 例如 `zip` 不是 RACO，只是 RAO。

# Ranges*

- 关于 RAO 有一些重要事实：
- 它会存储 decay 后的实参，并在调用时“转发”它们。
- decay 会在下一讲（类型安全）细讲，但你已经知道 C 数组会 decay 成指针。
- 引用会 decay 成值。
- 所以首先，RAO 按值保存参数；这会让某些对象在调用时变成“右值”。
- 有些 range 只有在不是右值时才能作为 view，例如 `std::initializer_list`。
- 此外，指针不是 range 但数组是，decay 会造成一些惊人现象……
- 总之，若你现在不完全理解也没关系；但要记住它的后果：

- 1. 数组 decay 问题：

- 但我们不能用 `join_with("  ")`（双空格）；可以用 `join_with(std::string{" "})`！
- 使用 `namespace std::literals` 时可以写成 `" "s` 或 `" "sv`；以后再讲。
- `" "` 是 `char[3]`，它是 range，因为有 `begin` 和 `end`；但 decay 成 `char*` 后，`end` 就消失了！
- 但 `string` 不会 decay，所以没问题。
- 2. 右值问题：
- 类似地，若你对 `vector<vector<int>> v` 做 `join_with`，你不能写 `v | join_with(k)`，其中 `k` 是 `initializer_list<int>`！
- 但你可以写 `join_with(v, k)`，因为它不是调用函数，而是直接构造新 view，不会把它“转发”成右值。
- 也就是 `join_with(v, k)` 是一个 view；`join_with(k)` 是 RACO（随后需要“转发”），而 `join_with(k)(v)` 应产生 `join_with(v, k)`。
- 你也可以让 `k` 成为容器，例如 `array`、`vector` 等都可以。

# 哨兵（Sentinels）

- 最后介绍哨兵（sentinels）。
- 我们说 range 是一对迭代器；但事实上不止如此。
- 一个迭代器 `begin` 与一个哨兵 `end` 就足够了！
- 例如你可以为 `const char*` 定义自己的哨兵：

- 你可以把它用于 `stdr::subrange` 来构造 range。
- 仍然允许用基于范围的 `for` 遍历！
- 你也可以在 range 版本的算法里使用它，但不能在普通算法里直接使用。
- 普通算法要求 `begin` 与 `end` 类型相同（也就是 `common_range` 概念）；你可以用 range adaptor `stdv::common` 把迭代器-哨兵对变成 common range。
- 所以，对不是 common 的双向 range，`reverse`/`slide` 仍会缓存 `begin`，因为你不能用 `end.base()` 拿到迭代器。

# 结语

- 还有一些细节未覆盖：
- 1. non-propagating cache：也就是 `chunk` / `lazy_split` / `join` / `join_with` 会为输入 range 保留一些东西，类似我们前面说的缓存状态（并会造成同样的问题！）。
- `join` / `join_with` 对“range 的引用”的 range 也会如此。
- 2. `std::unreachable_sentinel`：一个对 `==` 永远返回 `false` 的哨兵；用它当哨兵等价于 `while(true){ …; it++; }`。
- 3. `stdv::counted(it, n)`：range 也可以定义成“起始迭代器 + 计数”。
- 这类似带 `end` 的 `std::counted_iterator`。
- 4. C++23 起的 RACO 定制；因为需要更进阶知识，本课不讲。
- 你可以在学完整个课程后自学。
- 5. `stdv::as_rvalue` / `std::move_sentinel`；你会在移动语义里理解它们。
- 6. `.cbegin()` / `.cend()`：类似 `stdv::as_const`。

# 容器、ranges 与算法

Generator

# Generator

- 若你写过 Python，会发现 ranges 很像 Python，例如 `filter`、`enumerate`、`map` 等。
- 惰性求值也很熟悉……
- Python 里的 Generator 就是这样！
- 对生成器表达式，它就像惰性求值的流水线。
- 但生成器函数呢？
- 例如：函数执行到 `yield` 会暂停并“返回”一个数；当迭代器前进时，函数会恢复执行，直到下一个 `yield` 或真正的 `return`。

# Generator

- 自 C++23 起，协程（coroutine）也支持 generator。
- 协程彼此协作，并自行让出执行流。
- 相比之下，线程通常彼此竞争，并由操作系统打断以让其它线程运行。
- Generator 也一样：当你需要下一个值时，就把执行流交给 generator 函数；当 generator 完成其任务，它会交还执行权。
- 显然，协程的上下文会被保存，从而可以暂停与恢复。
- 在 Python 3.6 之前，协程是用 generator 实现的……
- Generator 也是 `input_range` 与 view；它提供 `begin()` 与 `end()` 用于遍历，对迭代器 `++` 会恢复函数。
- 注意：调用 `.begin()` 会启动协程！

# Generator

- 例如：

- 当你调用 `generator.begin()`，generator 函数会在 `co_yield begin` 处暂停。
- 当你解引用迭代器时，你会得到 `begin`（也就是 0）。

# Generator

- 一些备注：
- 注 1：一个 generator 只能用一次。
- 也就是你不能调用两次 `begin()`。
- 每个 generator 代表一个执行上下文，例如当 `begin` 是 3 时，你不能让执行“倒流”让 `begin` 变回 2。
- 若想再遍历一次，你需要新的 generator。
- 注 2：generator 有 `operator=`，但它实际上是交换两个 generator，也就是交换它们的执行上下文。
- 交换后迭代器不失效；它们仍指向原来的上下文。
- 注 3：`co_return` 可以省略。
- 注 4：保存上下文也需要内存，因此最后一个模板参数也可以提供分配器；这里先省略。
- 注 5：generator 也是 view，且只是 `input_range`。
- 协程我们会在以后深入讲！

# 容器、ranges 与算法

Function

# Function

- Function
- 成员函数指针（其实没那么重要……）
- 可调用对象参数
- 对 lambda 的强化

# 成员函数指针

- 我们讲过函数指针；但成员函数特殊，与普通函数不同。
- 那怎么取它们的指针呢？
- 首先，成员函数可以是静态或非静态。
- 静态成员函数就是带 `Class::` 的普通函数；其指针与普通函数指针一样。
- 然而，非静态成员函数总是绑定到某个具体对象，也就是有一个 `this` 指针作为参数。这才是我们想讲的。
- 所以成员函数指针意味着“一个不完整的函数”，要用它还得把对象绑定上去，才能变完整。

# 成员函数指针

- 等等……你可能会想“好麻烦！我直接用 `obj.xx` 不行吗？”
- 我们稍后再说；先看语法。
- 例如：

- 定义成员函数指针的类型是 `Ret (Class::*)(params)`。
- 与普通函数不同，`&Person::AddAge` 里的 `&` 是必需的（也就是不会 decay）。
- 用 `.*` 绑定对象，或用 `->*` 绑定对象指针。
- `(p.*ptr)` 要加括号，因为 `operator()` 优先级更高，不加括号会编译失败。

# 成员函数指针

- 类似地，你也可以定义数据成员指针，用法相同。
- 记住类型本身很麻烦，所以可以用 `auto ptr = &Person::AddAge`，或 `using FuncPtr = decltype(&Person::AddAge)`。

- 另外 `.*` 与 `->*` 很怪，因此自 C++17 起可以用 `<functional>` 里的 `std::invoke`：
- 这个函数会按对应运算符去调用可调用对象。
- 成员函数指针 + 对象：用 `.*`。
- 成员函数指针 + 对象指针：用 `->*`。
- 其它情况：尝试 `()`。

# 成员函数指针

- 自 C++23 起提供 `std::invoke_r<Result>`，会把调用结果转换成 `Result` 类型。
- 最后，它为什么有时有用？
- 指针只传达类型，而不是某个具体成员！
- 以前我写过一个简单的 SIMD 程序来混合两张图片。

`RGBdata` 是 `vector<RGB>`，这是混合 r 分量。

# 成员指针

- 若我还希望对 g、b 做混合，就得把代码粘贴两次。
- 若发现 bug？糟糕，再粘贴一次！
- 用成员指针可以写成：
- 整个过程就像：

- 干净又漂亮……
- 然而成员指针实际仍很少用……
- 结语：它们的大小是实现定义的。

# Function

- Function
- 成员函数指针
- 可调用对象参数
- 对 lambda 的强化

# 把函数当作参数

- 有时我们需要把函数当参数传递。
- 你立刻会想到函数指针。
- 这在 C 里很常用。
- 但有两个问题：
- 有时参数类型约束不严格，例如 `int` 能转换成 `double`，于是 `func(double)` 也可接受。
- 在 C++ 里，你通常只需要“某种可调用物”，也就是允许 functor。
- 这两者都无法单靠函数指针解决！
- C++ 里一般有两种做法：
- 用模板参数；`<algorithm>` 用这种方式接受可调用对象，例如 `sort` 里用于比较的谓词。
- 用 `<functional>` 里定义的 `std::function`。

# `std::function`

- `std::function<RetType(Args…)>` 几乎可以容纳所有“返回类型可转换为 `RetType` 且参数为 `Args`”的可调用对象。
- 例如 `std::function<int(B, std::string, float)> f` 可由以下构造：
- 普通函数/静态成员函数，例如 `int func(B, std::string, float){…}`。
- Functor，例如 `struct A{ int operator()(B, std::string, float) { … }};`。
- 闭包（无论是否有捕获），例如 `[&](B, std::string, float)->int { … }`。
- 非静态成员函数，例如 `int B::MFunc(std::string, float)`。
- 你可以把成员函数理解成第一个参数是 `this` 的函数……
- 类似地，可以用 `(B*, …)`、`(B&, …)` 等接受成员函数。
- 若类型写成 `B`，原 `B` 对象不会被改变，也就是按值拷贝。
- `nullptr`，就像空函数指针（也是默认构造的行为）。

# `std::function`

- 成员函数甚至保留多态：若它接受 `Parent&` 但你传入子类对象，且成员函数是 `virtual`，则会调用子类版本！
- 拿到 `std::function` 后直接用 `operator()` 调用即可。
- 调用空函数会抛出 `std::bad_function_call`。
- 可用例如 `if(f)` 或 `if(f != nullptr)` 判断是否为空，就像普通函数指针，因为它支持 `operator bool` 以及与 `nullptr` 的 `==`/`!=`。
- 更强的是，你可以绑定部分参数得到新的 functor。
- 例如可以用 `std::bind(any_callable, params)` 得到 `std::function`。
- 例如：

# `std::function`

- 注意：若要绑定引用，需要 `std::ref()` 或 `std::cref()`。
- 否则被绑定参数会被拷贝，而函数的引用参数会改到这个拷贝上，而不是你本来想绑定的那个对象。
- 例如：

# `reference_wrapper`

- `std::(c)ref()` 事实上创建 `std::reference_wrapper<(const) T>`，可视为一种“具现化的引用”。
- 例如你不能在容器里存引用类型。
- 那可以用例如 `std::vector<std::reference_wrapper<T>>`！
- 它本质是指针包装，但不能为空，就像引用一样。
- 你可以把对象赋给它，用 `.get()` 或转换成 `T&` 来取得引用。
- 若存的是函数，可以直接 `(params)` 调用（也就是 `operator()`）。
- 与引用不同，它可以用 `operator=` 绑定到另一个对象，更像指针。
- 也用于在标准库的某些方法里显式表示“这里应当是引用”，例如 `std::bind_xx`。
- 通常这时要特别注意生命周期问题，也就是使用它时引用的变量是否已析构（悬空）。

# `std::function`

- 我第一次了解 `std::function` 时非常震惊。
- 它强大到看起来只要一个 `std::function` 就够了。
- 显然，自由有代价。
- 它有两个缺陷：
- 性能：相比直接函数调用，大约会带来 10%–20% 的性能损失。
- 构造/析构时可能需要 `new/delete` 自定义 functor。
- 在性能关键场景里，`new/delete` 很贵，可能比算术运算慢成百上千倍。
- 更不用说函数调用之外的额外开销……
- 因此库实现会用 SOO（小对象优化）：准备一块不必动态分配的小缓冲区。
- 当 functor 足够小，就不会触发 `new/delete`。
- 标准也规定：从普通函数指针构造时永远不应动态分配。

# `std::function`

- 注意：调用 `std::function` 时，要保证 lambda 按引用捕获的实体仍然有效。
- 例如异步回调里若按引用捕获局部变量并离开函数，局部变量已析构；回调就会看到悬空引用……
- 最后，由于 lambda 只是匿名结构体，当捕获太大时，它可能不是“小对象”。
- 这时可以 `auto lambda = xx;` 并把 `std::ref(lambda)` 传给 `std::function` 的构造函数。
- 同时要保证在当前作用域结束前完成调用，也就是 lambda 还没析构。
- 否则对 lambda 的引用会悬空！
- 这也是为什么不能把引用绑到临时 lambda 上（`std::ref([&xx](){…})`），因为构造结束后 lambda 立刻失效。
- 若你现在不理解也没关系；以后我们会更多讲生命周期……

C++26 增加 `std::function_ref`，基本等价于这里的 `std::function{std::ref{xx}}`。推荐作为参数使用，就像 `std::string_view`（字符串视图我们以后也会讲……）。

# `std::function`

- `std::function` 的第二个缺陷是它并不能真正容纳所有 functor……
- 当 functor 不可拷贝（例如只能移动，如 `std::unique_ptr`）！
- 这是因为 `std::function` 支持 `operator=`，需要拷贝 functor。
- 因此自 C++23 起引入 `std::move_only_function`。
- 用法与 `std::function` 类似，除了：
- 它不能被拷贝。
- 为空时不会抛异常；调用空的 `move_only_function` 是 UB。
- 例如：

# Move only function

- 下面都可以：

- 但当你想绑定 `unique_ptr` 或传入只能移动的 functor 时，就不那么好了……
- 自 C++23 起：

- 但它仍不能与 `std::bind_xxx` 很好配合。
- 更进一步，若 functor 只能构造、不能移动，你仍可使用它：
- 这像 `emplace`，只传参数。
- `move_only_function` 本身仍可移动……

# Move only Function

- 另外，`std::move_only_function` 会尊重成员函数的 cv 限定、引用限定，以及 `noexcept` 说明符。
- cv 限定你已经知道；引用限定我们以后会提。
- 例如：

- 一句话（其它限定与 `const` 类似）：
- `F<Ret(Args…) const>` 不会接受没有 `operator() const` 的 functor。
- `std::function` 不允许在这里加限定符。
- `F<Ret(Args…)>` 会接受任意 functor，但不允许对 `const F` 调用 `operator()`。
- `const std::function` 仍会“容忍”调用 `operator()`。
- 自 C++26 起增加 `std::copyable_function` 来替代 `std::function`，也会尊重这些限定。
- C++ 又一个“脏”例子……

# `<functional>`

- `<functional>` 里也有一些预定义模板 functor。
- 你在容器部分见过 `std::less<T>` 就是其中之一。
- 算术 functor：`plus`、`minus`、`multiplies`、`divides`、`modulus`、`negate`（也就是 `-x`）。
- 比较 functor：`equal_to`、`not_equal_to`、`greater`、`less`、`greater_equal`、`less_equal`、`compare_three_way`（自 C++20）。
- 逻辑 functor：`logical_and`、`logical_or`、`logical_not`。
- C++17 的 `not_fn` 接受 functor `F`；其 `operator(…)` 是 `!F(…)`。
- 位运算 functor：`bit_and`、`bit_or`、`bit_xor`、`bit_not`（也就是 `~x`）。
- `identity`（自 C++20）就是返回自身。

# `<functional>`

- 这些 `Functor<T>` 都有 `auto operator()(const T&, const T&) const`。
- 那么看看 `std::set<std::string> s` 会做什么！
- 也就是 `std::set<std::string, std::less<std::string>>`。
- 对 `s.find("test")`，会先构造临时 `std::string`。
- 然后它会把这个 key 与很多内部节点比较，也就是 `std::less<std::string>{}(TempStr, key)`。
- 然而这个构造似乎不必要……
- 总之，比较只会读字符串，为什么不直接用原始 C 字符串？
- 即便 `s.find` 接受任意类型作为模板参数，`std::less<T>` 仍会转换它。
- 而且每次比较都可能构造一次！
- 所以我们还需要模板 `operator()`，能直接接受任意类型并用它们的比较运算符。

# `<functional>`

- 就像这样：

- 考虑到 `void` 不能作为对象类型，以前 `std::less<void>` 没有意义。
- 因此自 C++14 起，它被特化成上面这样（`<functional>` 里的 functor 都类似特化）。
- 也可以简写成 `std::less<>`。
- 它被称为 transparent operator，因为它定义了 `is_transparent` 的类型别名（任意类型都行，例如 `using … = void`），从而让容器识别并使用模板版 `find`。

# Transparent operator

- 支持 transparent operator 的方法称为 heterogeneous methods（异构方法）。
- 所以我们只需要 `std::set<std::string, std::less<>>` 就能启用该特性以提升性能！
- 你可以传入 C 字符串、`std::string`、以后会讲的 `std::string_view`，或任何能与 `std::string` 比较的类型！

- 类似地，无序容器也能改进。
- 例如使用 `std::equal_to<>`。
- 那哈希函数呢？它能 transparent 吗？

# Transparent operator

- 但不同类型可能有不同哈希方式……
- 例如 `std::hash<double>(1.0)` 通常不同于 `std::hash<int>(1)`。
- 这种性质在比较里通常不存在：`1.0 == 1.0` 与 `1.0 == 1` 一样，所以 `std::equal_to<double>` 可以改成 `std::equal_to<>`。
- 整个哈希流程是算哈希值、映射到桶、再用 `equal_to` 找精确值。
- 这假设相等对象的哈希值相同！
- 因此使用 transparent hash 通常很危险，所以没有 `std::hash<>` 特化。
- 不过存在一些场景：可以把可能类型低成本转换成某种 view 类型；例如 C 字符串、`std::string`、`std::string_view` 都能低成本变成 `std::string_view`，用 `std::string_view` 的哈希函数是可行的。

# Transparent operator

- 你也可以定义自己的哈希 functor：
- 注意：transparent hash 通常应是非模板，这样你能保证所有 key 用同一种哈希方式。
- 它总是接受一个确定的类型！
- 然后可以用 `std::unordered_set<std::string, Hasher, std::equal_to<>>` 提升性能。
- 注意：只有哈希与相等比较都 transparent 时，`find()` 等才会使用该特性。
- 结语：少数成员函数仍未支持 transparent operator，尽管 C++23 有提案改进。但 C++23 委员会要讨论的东西太多，只能把这个提案留到 C++26 以把时间留给更重要的特性。
- 总之，C++26 会更完整支持。

# Function

- Function
- 成员函数指针
- 可调用对象参数
- 对 lambda 的强化

# Lambda 表达式

- lambda 还要补充几点。
- 首先，自 C++14 起 lambda 可以像 `auto func(…)` 那样从函数返回。
- 我们把无捕获的 lambda 叫 stateless lambda；否则叫 stateful lambda。
- 我们说过 lambda 是匿名结构体，所以 `decltype(…)` 会推导出唯一类型。
- 理论上你可以用这个类型构造新 functor。
- 例如 `auto l = [](int a, int b) { return a + b; }; using T = decltype(l); T l2{};`
- 然而事实上这要到 C++20 才合法……
- 匿名结构体也有拷贝构造与赋值运算符。
- 有状态 lambda 不可默认构造、不可拷贝、不可赋值。

# Lambda 表达式

- 这对例如容器的无状态比较 functor 很有用。
- 以前你不能用 `std::set<xx, LambdaType>`，因为 `LambdaType` 不能用来构造新 functor，也就是 `LambdaType{}(…)` 不可能。
- 你只能 `std::set<xx> s{ …, Compare}` 传入 functor。
- C++20 后这个限制消除了！
- 此外，C++20 支持 unevaluated lambda：可以直接 `using T = decltype([](int a, int b) { return a + b; });`，而不必先声明真实 lambda 再 `decltype`。
- `decltype`、`sizeof`、……并不真的需要求值表达式，例如 `sizeof(a += b)` 不会真的执行 `a += b`。
- 结语：你还可以把 lambda 与变量初始化结合，并立刻调用：

# 容器、ranges 与算法

Algorithms

# Algorithm

- 算法由以下组成：
- 迭代器对，表示算法作用的区间。
- 也是 `[begin, end)`。
- 谓词函数/变换函数：从迭代序列取若干参数并判断/操作。通常它们不应就地改元素本身，所以参数应是 `const&` 或按值拷贝。
- 函数接受底层值而不是迭代器。
- 另外 `operator()`（若需要还有 `operator<`、`operator>` 等）通常应是 `const`；不过不 `const` 也基本可以。
- 若类型能以某种方式转换成 functor 类型（例如通过 `operator S()` 用户定义转换函数），也可以；
- 有时我们称之为 surrogate（代理）call function。
- 大多返回满足算法要求的迭代器；若没有满足者，返回 `end`。

# Algorithm

- 它们从不改变序列大小，除了：
- 你使用例如 `back_inserter`，迭代器本身会改变大小。
- `std::erase` / `std::erase_if`；这其实不在 `<algorithm>`，而在容器头文件里。
- 所以你要保证目标区间足够大，例如给 `vector` `resize`。
- 算法的可调用对象是值类型而不是引用类型。
- 若只是普通函数，那就是函数指针的拷贝。
- 或是临时量（例如立即调用的 lambda），也不算负担。
- 原因我们放到移动语义里讲……
- 但若是普通 functor，例如 `auto func = […](){…}; std::some_algorithm(…, func)`。
- 就会发生拷贝，也就是成员/捕获会被拷贝。
- 所以若你真的想传入 functor 的引用，可以用 `std::ref(func)`。

还记得吗？

# Algorithms

- Algorithms•Permutation
- 还有一些与随机数相关的算法，
- Search会在以后讲。
- Comparison•Numeric algorithms
- Counting
- Parallel algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Search

- 搜索算法有两类：
- 线性搜索，复杂度为各段长度乘积的量级，记为 𝑂(Π𝑟𝑎𝑛𝑔𝑒_𝑙𝑒𝑛𝑔𝑡ℎ)。
- 查找单个值：`(begin, end, …)`
- `std::find`：`(value)`，返回第一个等于 `value` 的迭代器。
- `std::find_if(_not)`：`(Pred)`，返回第一个使 `Pred(ele)` 为真（或为假）的迭代器。
- 在一段里查找是否属于另一段：`(begin1, end1, begin2, end2[, Pred2])`
- `std::find_first_of`。
- 在 `[begin1, end1)` 中找到第一个也属于 `[begin2, end2)` 的元素。
- 对每个 `ele1 ∈ [begin1, end1)`：
- `std::find(begin2, end2, ele1)`。
- 若不存在（得到 `end` 迭代器），继续循环；否则返回 `ele1` 的迭代器。
- 复杂度为 𝑂(𝑁𝑀)。

# Search

- 在序列中查找子序列（模式匹配）：`(begin1, end1, begin2, end2[, Pred2])`。
- 可选的 `Pred2` 充当 `operator==`。
- `std::search`：在 `[begin1, end1)` 中查找 `[begin2, end2)` 的第一次出现。返回该次出现在 `[begin1, end1)` 中的起始迭代器。
- `std::find_end`：在 `[begin1, end1)` 中查找 `[begin2, end2)` 的最后一次出现。返回该次出现在 `[begin1, end1)` 中的起始迭代器。
- 这两个方法的复杂度都是 𝑂(𝑁𝑀)！
- 然而我们学过，模式匹配里的 KMP 是 𝑂(𝑁 + 𝑀)。
- 以后我们会讲如何做到……
- 其它：
- `std::adjacent_find(begin, end[, Pred2])`，返回满足 `*it == *std::next(it)` 的迭代器。
- `std::search_n(begin, end, count, value[, Pred2])`，返回使 `[it, std::next(it, N))` 全为 `value` 的迭代器。𝑂(𝑁)。
- `std::next` 并不意味着要求随机访问；只是表示一段区间。

# Search

- 示例：

# Search

- 自 C++17 起，模式匹配可以线性完成。
- 但算法不是 KMP，因为在真实应用里 KMP 往往并非最优。
- 真实场景例如搜索引擎，匹配概率通常很低，暴力算法常数小，甚至可能比 KMP 更快。
- 这也许就是以前长期不提供更优保证的原因……
- 线性算法是 BM（Boyer-Moore）。
- 对应重载是 `std::search(begin, end, Searcher)`，需要随机访问迭代器（常见情况！）。
- `Searcher` 应由模式 `[begin2, end2)` 构造，并提供 `operator()(begin, end)`，返回 `[begin, end)` 内第一次出现范围的迭代器对 `[it1_begin, it1_end)`（但 `std::search` 只返回 `it1_begin`，因此你也可以直接用 `searcher`）。
- 标准库在 `<functional>` 提供三种 searcher：`std::default_searcher`（暴力）、`std::boyer_moore_searcher`、`std::boyer_moore_horspool_searcher`。BM 算法较难，有兴趣可看清华数据结构（下）第十三章（串）DE 两节课。BMH 只用坏字符（BC）表；BM 用 BC+好后缀（GS）表，因此后者线性，前者预处理与空间更小但最坏未必线性。

# Search

- 第二类是二分查找，作用于已排序序列。
- 关联容器自有成员方法，一般不用这些算法。
- 只有随机访问容器才能保证 𝑂(log 𝑁) 的跳转复杂度；否则到达中点要一步步走，是 𝑂(𝑁)。
- 但比较次数复杂度始终是 𝑂(log 𝑁)。
- 它们都接受 `(begin, end, value[, Pred2])`，`Pred2` 充当 `operator<`。
- 若 `a < b && b < a`，则视为相等。
- 注意许多算法要求 `operator<` 严格弱序，也就是 `a < b` 与 `b < a` 不能同时为真。
- 否则可能效率变差甚至完全错误！
- `std::binary_search`：返回 `bool`，表示 `value` 是否存在于 `[begin, end)`。
- `std::lower_bound`：返回 `it`，使 `value` 是 `[it, end)` 的下界。
- `[begin, it)` 都小于 `value`。这与 `std::map` 的含义一致。

# Search

- `std::upper_bound`：返回 `it`，使 `value` 是 `[begin, it)` 的上界。
- `[it, end)` 都大于 `value`。
- `std::equal_range`：返回迭代器对 `(it1, it2)`，使 `value` 与 `[it1, it2)` 等价。
- 即 `[std::lower_bound, std::upper_bound)`。
- 这三者与 `std::map` 的对应关系相同。
- 注意：若 `value` 类型不同，谓词可以只接受两种不同的类型。
- 例如对象很大，但只用整数 `id` 作为键。
- 于是可以直接用键，而不构造巨大的空对象。
- 对 `std::lower_bound`，只用 `Pred2(Element, KeyType)`。
- 对 `std::upper_bound`，只用 `Pred2(KeyType, Element)`。
- 对另外两个，需要两个方向的重载都能编译。

# Search

- 示例：

- 特别地，可以使用不同类型：

# Search

- 然而对 `std::equal_range` 与 `std::binary_search`，由于需要双向比较（要知道 `==`，`ele1 < ele2` 与 `ele2 < ele1` 都应能编译），谓词两侧类型通常要可转换。
- 一般可定义 `operator<`。

# Search

- 结语：也可以把 `auto operator<=>(int)` 与 `auto operator==(int)` 定义为成员函数来自动推导这些运算符！

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Comparison

- `std::equal(begin1, end1, …[, Pred2])`：返回 `bool`。
- `(begin2)`：设 `std::distance(begin1, end1) == n`，则比较 `[begin1, end1)` 与 `[begin2, begin2 + n)`。
- `(begin2, end2)`：比较 `[begin1, end1)` 与 `[begin2, end2)`。
- 自 C++14 起；若 `[begin2, begin2 + n)` 并非都有效，第一种写法可能危险（例如比较两个 `vector`，第一个更长）……
- 同样，`Pred2(Type1, Type2)` 即可；不必类型完全相同。
- `std::lexicographical_compare(begin1, end1, begin2, end2[, Pred2])`：返回 `bool`；`Pred2` 充当 `operator<`。
- 比较直到 `ele1 < ele2 || ele2 < ele1`，返回该布尔值。
- 若循环结束则返回 `false`。
- `std::lexicographical_compare_three_way(begin1, end1, begin2, end2[, Pred2])`：返回三向比较结果；`Pred2` 充当 `operator<=>`。

# Comparison

- `std::mismatch(begin1, end1, …[, Pred2])`
- `(begin2)`、`(begin2, end2)` 自 C++14 起。
- 类似 `std::equal`，但返回迭代器对 `(it1, it2)` 表示第一处不匹配。

这些算法都是 𝑂(𝑁)。

与 `equal_range` 类似，这也需要双向比较。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Counting

- `std::all_of/any_of/none_of(begin, end, Pred)`：返回 `[begin, end)` 中是否全部/任一/无一使 `Pred` 为真。
- `std::count(begin, end, value)`：返回 `[begin, end)` 中等于 `value` 的元素个数。
- `std::count_if(begin, end, Pred)`：返回 `[begin, end)` 中使 `Pred` 为真的元素个数。
- 这些算法都是 𝑂(𝑁)。
- 示例：

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Generating and Transforming

- `std::fill(begin, end, value)`：把 `[begin, end)` 全部填成 `value`。
- `std::fill_n(begin, count, value)`：把 `[begin, next(begin, count))` 全部填成 `value`。
- `std::generate(begin, end, Gen)`：对 `[begin, end)` 的每个元素，`ele = Gen()`。
- 调用 `n` 次，因此与 `fill` 不同。
- `std::generate_n(begin, count, Gen)`：类似。
- `std::for_each(begin, end, Transform)`：对 `[begin, end)` 的每个元素调用 `Transform(ele)`。
- `std::for_each_n(begin, count, Transform)`：类似，自 C++17。
- 通常手写 `for` 就够，因此不常用（并行版本以后讲）。所有 `_n` 算法返回 `begin + n`。

`for_each` 是唯一把传入的 functor 用作返回值的算法。

# Generating and Transforming

- `std::transform`：有一元/二元两种。
- `(begin, end, dstBegin, Transform)`：一元，类似 `for_each`，但返回值是变换结果，写到从 `dstBegin` 开始的序列，原序列不变。
- 要保证目标区间足够大，例如给 `vector` 先 `resize`。
- `dstBegin` 不应落在 `(begin, end)` 中会与被覆盖结果冲突的位置，否则后面元素会错。
- 特别地，`dstBegin == begin` 表示就地覆盖。
- `(begin, end, begin2, dstBegin, Transform)`：二元，变换为 `Transform(it1, it2)`；结果写到 `dstBegin`。
- 仍要注意 `begin2` 与 `dstBegin` 的有效区间……
- 这些算法都是 𝑂(𝑁)。

# Generating and Transforming

- 示例：

- 注意 `std::for_each` 通过接受 `T&` 来变换，而 `std::transform` 通过返回值变换。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Modifying

- 这部分算法最重要，因为用 `for` 循环往往很难写得同样正确或高效。
- `std::remove(begin, end, value)` / `std::remove_if(begin, end, Pred)`：返回迭代器 `it`，使 `[begin, it)` 不含等于 `value` 或使 `Pred` 为真的元素。
- 注意：算法不会 `erase` 任何迭代器！
- 因此 `[it, end)` 里是“无效”元素，若要删除它们，需调用容器的 `erase`。
- 无效元素的值未指定。以后你会知道原因。
- 这叫 erase-remove 惯用法。
- 自 C++20 起，它们整合为成员函数 `std::erase` / `std::erase_if`。
- `std::unique(begin, end[, Pred2])`：返回 `it`，使 `[begin, it)` 没有相邻相等元素；`Pred2` 充当 `operator==`。
- 这与我们讲过的 `list.unique()` 相同，但 `list.unique` 会删除元素，而 `std::unique` 不会（类似 `remove`）。
- 因此 `std::unique` 常用于随机访问容器。

# Modifying

- 这两个算法都是 𝑂(𝑛)，用双指针技巧。
- 我们以 `unique` 为例，你可以自己思考 `remove`。

- 1. 准备 `before` 与 `after` 两个迭代器，都初始化为 `begin`。
- `before` 始终表示最后被确认应保留的值；`begin` 一定在结果里，因此初始满足。
- 2. 推进 `after`。
- 3. 若 `after` 与 `before` 不等，则推进 `before` 并把 `*after` 拷贝/移动到 `*before`。
- 4. 循环直到 `after` 到达 `end`。
- 也要考虑 `begin == end` 的边界情况。

# Modifying

- 先把 `after` 推进到位置 1。

before

01 1 4 5 3 3 3

after

`*after == *before`？否，推进 `before`，把 `*after` 赋给 `*before`，再推进 `after`。

# Modifying

before

01 1 4 5 3 3 3

after

`*after == *before`？是，只推进 `after`。

# Modifying

before

01 1 4 5 3 3 3

after

`*after == *before`？否，推进 `before`，把 `*after` 赋给 `*before`。

# Modifying

before

01 4 4 5 3 3 3

after

`*after == *before`？否，推进 `before`，把 `*after` 赋给 `*before`……

# Modifying

- 最终结果类似：

before

01 4 5 3 3 3 3

after
- 返回 `before`。
- `remove` 非常类似，你可以自己设计算法！
- 若真要删除被移除元素，需要手动 `vec.erase()`（即 erase-remove）。
- 整体仍是 𝑂(𝑛)（配合 `erase` 删除后缀，不会像反复单元素删除那样退化）。
- 自 C++20 起也可用 `std::erase(_if)` 一次完成 remove + erase。

# Modifying

- 你可能注意到：若序列已经 unique，会对每个元素做一次赋值……
- 在我们的例子里，若仔细看会出现 `1=1` 这种自赋值。
- 这只发生在起始已经 unique 的那段：被检查的值赋回自己。
- 一旦出现非 unique 位置，检查值与当前值位置不同，这种现象就消失。
- 因此在 MS-STL 里会有预循环跳过起始的 unique 段。
- `remove` 也有类似优化：跳过所有不会被移除的元素。

# Modifying

- `std::replace(begin, end, oldValue, newValue)` / `std::replace_if(begin, end, Pred, newValue)`：把所有等于 `oldValue` / 使 `Pred` 为真的值替换为 `newValue`。
- `std::swap(x, y)`：交换两个元素。
- `std::iter_swap(it1, it2)`：`std::swap(*it1, *it2)`。
- `std::swap_range(begin1, end1, begin2)`：逐个交换两个区间。
- `std::reverse(begin, end)`：反转区间。
- 实现上从两端向中间 `swap`。
- `std::rotate(begin, mid, end)`：把 `[begin, mid)` 左旋转到末尾。2 3 4 4 5
- 若要右旋，可用反向迭代器。
- 更直观的说法是“循环移位”：左移并把挤出的元素补到另一侧。
- 这里是 `rotate(v.begin(), v.begin() + 3, v.end())`，把 `{2,3,4}` 左旋到另一侧。

# Modifying

- `rotate` 也是 𝑂(𝑛)，典型实现有三种。
- 先想暴力：像冒泡那样逐个交换到位，复杂度 𝑂(𝑘(𝑛−𝑘))，其中 𝑘 为 `std::distance(begin, mid)`。
- 观察：要让元素一步到位，最简单是每次只走一步！
- 想 `arr[0]`：下一个占据它的是 `arr[dis]`；填 `arr[dis]` 需要 `arr[dis*2]`。
- 当 `dis*n` 超过长度就取模（为性能也可每次超界就 `-= n`）。
- 一旦 `(dis*n) % size` 回到 0，用 `arr[0]` 填回，这一轮换群（循环群）就位。
- 轮换群个数即 `mk % n` 的等价类个数，为 `gcd(n,k)`。
- 每个轮换群里每个元素只移动一次，`gcd` 求法整体 𝑂(𝑛)，故总复杂度 𝑂(𝑛)。

注意 `gcd(𝑛,𝑘)` 有更紧的 𝑂(log min(𝑛,𝑘)) 复杂度，这里不展开。

# Modifying

- 该算法唯一问题是缓存不友好，因为跳转多。
- 所以在 MS-STL 里会改成成组交换来优化。

# Modifying

- 当各组都换到一端后，结果类似：

- 复杂度 Θ(𝑛−𝑘)。
- 再把移位后的元素调整到正确位置，即对剩余部分继续循环移位。
- 现在要对 `[first, mid, last)` 再 `rotate`，于是重复前面的循环。′ ′
- 即令 𝑛←𝑘，𝑘←𝑛−𝑛%𝑘；循环直到 𝑛%𝑘==0。
- 总复杂度 Θ(𝑛−𝑘)+Θ(𝑘−(𝑛−𝑛%𝑘))+⋯，后一项会抵消前一项的 −𝑘，最终 Θ(𝑛)。
- 实用实现（无递归，通过重置迭代器循环）：

# Modifying

- 第三种实现靠三次 `reverse`。
- 事实上 `reverse` 是很多算法的基础。
- `[begin, mid-1]` `[mid, end-1]`。
- 先分别反转两段，得到 `[mid-1, begin]` `[end-1, mid]`。
- 再反转整体，得到 `[mid, end-1][begin, mid-1]`。
- 结束；三次反转完成旋转！
- 复杂度 Θ(𝑘)+Θ(𝑛−𝑘)+Θ(𝑛)。
- 也比第一种更缓存友好。
- 在 MSVC 中，对随机访问或双向迭代器用此法；因需要回退，前向迭代器不用它而用前述最后一种实现。

# Modifying

- `std::shift_left/right(begin, end, n)`：自 C++20；把 `[begin, end)` 左/右平移 `n`；
- 与 `rotate` 不同，不是“循环”；被挤掉的元素永久丢失（无效）。
- `shift_left` 返回结果区间的末尾，`[it, end)` 无效；`shift_right` 返回结果区间的起始，`[begin, it)` 无效。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Copying

- 你在流迭代器里已经见过 `std::copy`。
- `std::copy(begin1, end1, dstBegin);`
- 仍要注意目标长度。
- `std::copy_if(begin1, end1, dstBegin[, Pred]);`
- `std::copy_n(begin1, n, dstBegin);`
- 这三个都是向前拷贝，即从 `begin` 往 `end`。
- `std::copy_backward(begin1, end1, dstEnd)`：把 `[begin1, end1)` 拷贝到 `[prev(dstEnd, distance), dstEnd)`，但方向向后。
- 你可能记得 `vector` 插入可用 `copy_backward` 实现。向前与向后的差别只在两区间重叠时出现。
- 类似还有 `std::move(_backward)`，是移动而非拷贝。移动语义里再讲。

# Copying

- 最后，一些修改型算法有“拷贝版”，即结果写到新区间而不是原地。
- 原区间不变。
- 只是在末尾参数多一个输出迭代器……
- `std::remove_copy(begin1, end1, dstBegin, value)`。
- `std::remove_copy_if(begin1, end1, dstBegin, Pred)`。
- `std::unique_copy(begin1, end1, dstBegin[, Pred2])`。
- `std::reverse_copy(begin1, end1, dstBegin)`。
- `std::rotate_copy(begin1, mid1, end1, dstBegin)`。
- `std::replace_copy(begin1, end1, dstBegin, oldValue, newValue)`。
- `std::replace_copy_if(begin1, end1, dstBegin, Pred, newValue)`。
- 用法不赘述：就是多一个输出目的地。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Partition and Sort

- 划分是快排的一部分，所给算法也相近，因此放在一起讲。
- 对划分（partition）：
- 指存在迭代器 `it`（划分点），使得 `[begin, it)` 中元素都使 `Pred` 为真，而 `[it, end)` 都使 `Pred` 为假。
- `std::is_partitioned(begin, end, Pred)`：判断是否已是划分。
- `std::partition(begin, end, Pred)`：重排成划分；返回划分点 `it`。
- 类似还有 `std::partition_copy`，需要真/假两个输出迭代器。
- 这两个算法都是 𝑂(𝑁)。

# Partition and Sort

- `std::stable_partition(begin, end, Pred)`：类似 `std::partition`，但每段内部保持原相对顺序。
- 例如 `D C A B`，其中 `A,C` 为真、`B,D` 为假，则稳定划分得 `C A D B`，使 `C` 仍在 `A` 前，`D` 仍在 `B` 前。
- 没有 `std::stable_partition_copy`。
- 空间 𝑂(𝑁)，时间 𝑂(𝑁)。
- 若空间不足，时间为 𝑂(𝑁 log 𝑁)。
- `std::partition_point(begin, end, Pred)`：假设已是划分，找划分点（假段第一个元素）。
- 既然已划分，可用二分；因此 𝑂(log 𝑁)。

# Partition and Sort

- 注意：`partition` 与 `partition_copy` 的输出形式可以不同！

# Partition and Sort

- `stable_partition` 的实现思路：
- 内存足够时：开缓冲区；把假段移到缓冲区，真段紧凑（类似 `std::remove_if`，被移走的一段暂存缓冲区）；
- 再把缓冲区元素搬回。
- 内存不足时：把序列两半分别稳定划分。
- 得到 `[true,false],[true,false]`。
- 对中间 `[false,true]` 做 `rotate`，使整体划分完成。𝑛
- 𝑇(𝑛)=2𝑇(𝑛/2)+𝑂(𝑛)，总体 𝑂(𝑛 log 𝑛)。
- 原因在《算法设计与分析》里会学。

# Partition and Sort

- 排序：总有可选谓词 `Pred2`，充当 `operator<`，默认升序。
- `std::is_sorted(begin, end[, Pred2])`：是否已排序。
- `std::is_sorted_until(begin, end[, Pred2])`：返回 `it` 使 `[begin, it)` 有序。
- 二者只是检查升序，故 𝑂(𝑁)。
- `std::sort(begin, end[, Pred2])`：排序。
- 𝑂(𝑁 log 𝑁)
- `std::stable_sort(begin, end[, Pred2])`：稳定排序；相等元素保持原顺序。
- 空间 𝑂(𝑁)，时间 𝑂(𝑁 log 𝑁)；空间不足时时间 𝑂(𝑁 (log 𝑁)²) 量级（实现相关）。

# Partition and Sort

- 由于 `std::sort` 要求最坏也达到 𝑂(𝑁 log 𝑁)，单纯快排不够。
- 快排平均 𝑂(𝑁 log 𝑁)，最坏 𝑂(𝑁²)！
- C++ 标准库广泛使用内省排序（IntroSort）。
- 结合插入排序、堆排序与快排。
- 元素很少时用插入排序。
- 我们知道它常数极小，即便 𝑂(𝑁²) 也可能更快。
- 递归太深时换堆排序。
- 这是为了避免最坏情况。
- 堆排序最坏 𝑂(𝑁 log 𝑁)，但父子跳转随机访问多，常数大、缓存不友好。
- 其它情况用快排。

# Partition and Sort

- 简要：• 为减少落到堆排序的概率，枢轴仍很重要。
- 不能真随机，否则用户难以调试。
- 例如排序中有内存泄漏时，用户需要确定性行为来观察原因。
- GCC/Clang 从 `{begin, mid, end - 1}` 中选。
- MS-STL 里 `ISORT_MAX = 32`。• 取三者中值为枢轴。

# Partition and Sort

- 若总选 `begin`，升序序列会让快排退化到 𝑂(𝑁²)。
- 也就是每次划分只去掉一个元素。
- 三数取中可消除该情况。
- 但还有“管风琴”（pipe organ）序列仍可能最坏！
- 每次枢轴总落在最左或最右，使划分极差。
- 可以看动画……

Credit: IceBear@zhihu.

# Partition and Sort

Random sequence Pipe organ sequence

当递归深度过大，每个局部会改用堆排序。

# Partition and Sort

- MS-STL 的应对：
- 9 个候选枢轴：元素足够多时从 9 个里选，而不是 3 个，更难触发最坏情况。

# Partition and Sort

- MS-STL 也优化了 `partition`。
- 当大量元素相等时，基本 𝑂(𝑁) 就够。
- 因此会把等于枢轴的元素成段，再分别排左右。
- 全相等时恰好 𝑂(𝑁)。

0 1 1 4 5 1 3 1

0 1 1 1 15 34

# Partition and Sort

- 划分过程大意：
- 设 `it0=pivot`，双向移动使 `[PFirst, PLast)` 都等于 pivot（P 表示 pivot 等值段）。
- `GFirst = PLast`，`GLast = PFirst`。
- 增大 `GFirst`：当 `*GFirst > *PFirst` 时满足划分一侧。
- 相等则说明是与 pivot 相等的元素，需把 `*GFirst` 与 `*PLast` 交换并 `++PLast`。
- 小于则应出现在左侧，跳出循环。
- 减小 `GLast`：当 `*GLast < *PFirst` 时满足另一侧；子情况与 `GFirst` 对称。
- 最后交换 `GFirst` 与 `GLast`，二者是一对“都不满足”的元素，交换后满足！
- 边界：若 `GFirst` 或 `GLast` 触到端点，等值段会“平移”给另一侧腾出划分空间。

# Partition and Sort

- 边界处理：• 递归深度估计为 1.5 log 𝑁（直到 `_Ideal==0`）。

- 快排主过程类似：

看起来只排一侧，其实是双侧；你可以自己想为什么。

# Partition and Sort

- 特别地，Go 与 Rust 的非稳定排序实现了 PDQsort（Pattern-Defeating Quicksort）。
- 顾名思义，它试图打破导致快排最坏的规律。
- 当一侧规模过小视为“不平衡”，会对序列做“洗牌”。
- 洗牌不是随机，而是在若干距离上交换元素。• 这会破坏最坏模式，使最坏情况回到 𝑂(𝑁 log 𝑁)。
- 这里有 C++ 实现；在我机器上基准测试一般不低于或快于 MSVC 的 `std::sort`。
- 该实现也考虑了缓存行等，对比不完全公平；但即便关掉这些优化，通常也应更快。

- 稳定排序：内存够时常用归并排序；否则分治直到空间够。
- 元素很少时同样用插入排序。

# Partition and Sort

- `std::partial_sort(begin, mid, end)`：整理 `[begin, end)` 使 `[begin, mid)` 已是全局排序中对应位置的那段。
- 𝑂(𝑀 log 𝑁)，常用堆实现。
- 类似还有 `std::partial_sort_copy`。
- `std::nth_element(begin, mid, end)`：重排使 `*mid` 已是全局排序中该位置的值，且整体按它划分。
- 过程类似快排。
- 选枢轴。
- 按枢轴划分。
- 若枢轴在 `mid` 右侧，在 `[begin, pivot)` 继续；否则在 `[pivot, end)`。
- 序列足够短时改插入排序。
- 𝑂(𝑁)；Knuth 在《计算机程序设计艺术》中给出分析。

# Partition and Sort

- `std::merge(begin1, end1, begin2, end2, dstBegin[, Pred2])`：归并两个（已排序）区间到 `dstBegin`；同归并排序里的 merge。
- 与 `std::list::merge` 相同思想，但需要输出目的地。
- 𝑂(𝑀+𝑁)；虽不强求已排序，但通常只对有序区间有意义。
- `std::inplace_merge(begin, mid, end)`：把已排序的 `[begin, mid)` 与 `[mid, end)` 归并为有序 `[begin, end)`。
- 类似归并排序的 merge，空间 𝑂(𝑁)，时间 𝑂(𝑁)。
- 空间不足时 𝑂(𝑁 log 𝑁)。
- 实现类似稳定划分，这里不展开。

注意：`stdv` 没有 `concat`；事实上常用 range-v3。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Heap

- 我们在 `priority_queue` 里讲过堆。
- 需要随机访问迭代器。
- `a[i] <= a[(i – 1) / 2]`。
- 下滤/上滤（percolate down/up）。
- 接口与排序部分类似；可选 `Pred2` 充当 `operator<`。
- `std::is_heap(_until)(begin, end[, Pred2])`。
- `std::make_heap(begin, end[, Pred2])`：Floyd 建堆。
- 这两个都是 𝑂(𝑁)。
- `std::push_heap(begin, end[, Pred2])`：假设 `[begin, prev(end))` 已是堆；把最后元素（`prev(end)`）当作新插入，上滤。
- `[begin, end)` 成为新堆。

# Heap

- `std::pop_heap(begin, end[, Pred2])`：假设 `[begin, end)` 是堆；用最后元素覆盖堆顶并下滤。
- `[begin, prev(end))` 成为新堆。
- 二者都是 𝑂(log 𝑁)。
- `std::sort_heap(begin, end[, Pred2])`：假设 `[begin, end)` 是堆；排序它。
- 理论上“已是堆”比普通区间更有结构。
- 实践上这就是堆排序，𝑂(𝑁 log 𝑁)。
- 但堆排序通常比 `std::sort` 慢……
- 若不能接受 𝑂(log 𝑁) 的递归栈增长，可考虑堆排序，它是 𝑂(1) 辅助栈。
- 也有报告称 `std::sort_heap` 代码体积更小。

https://stackoverflow.com/questions/12786518/reason-for-existence-of-sort-heap

# Heap

- 注意：不能假设 `pop_heap` 一定交换堆顶与最后元素（你可以把最后一个位置视为弹出后的无效位）。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Set operations

- 集合操作用于已排序区间，包括 `set`。
- 可选 `Pred2` 充当 `operator<`。
- `std::includes(begin1, end1, begin2, end2[, Pred2])`：判断第二段是否是第一段的子集；
- 设 `it1` 在第一段，`it2` 在第二段。
- 若 `*it2 < *it1`，由于都排序，`*it2` 不可能出现在第一段，返回 `false`。
- 否则若 `!(*it1 < *it2)`，则 `++it2`。
- 表示 `*it1 == *it2`，继续找下一个。
- `++it1`，继续循环。

# Set operations

it1

1 1 4 5 8

!(*it1 < *it2) && 14 8 !(*it2 < *it1), it1++, it2++. it2

# Set operations

it1

1 1 4 5 8

*it1 < *it2, it1++ 14 8

it2

# Set operations

it1

1 1 4 5 8

!(*it1 < *it2) && 14 8 !(*it2 < *it1), it1++, it2++. it2

# Set operations

- 后面步骤略；若第二个序列是 2 呢？

it1

1 1 4 5 8

*it2 < *it1，失败！ 12 8

it2

# Set operations

A B

- `std::set_intersection(begin1, end1, begin2, end2, dstBegin[, Pred2])`：即 𝐴 ∩ 𝐵。
- `std::set_union(begin1, end1, begin2, end2, dstBegin[, Pred2])`：即 𝐴 ∪ 𝐵。
- `std::set_symmetric_difference(begin1, end1, begin2, end2, dstBegin[, Pred2])`：即 𝐴 − 𝐵（讲义记法；标准语义为对称差）。
- `std::set_difference(begin1, end1, begin2, end2, dstBegin[, Pred2])`：即 𝐴 − (𝐴 ∩ 𝐵)。
- 都是 𝑂(𝑁)，实现与 `std::includes` 类似；可自行推演。
- 对 multiset，交保留较少次数，并保留较多次数（仍符合数学 multiset 语义）。

# Set operations

- 这里用 ranges 版算法展示更直观。
- 后面章节你会完全理解。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# MinMax

- `Pred2` 充当 `operator<`。
- `std::min/max/minmax(a, b[, Pred2])`：返回较小/较大元素的（或一对）引用。
- 也可传入初始化列表，返回列表中最小/最大值，但返回的是值而非引用。
- 若等价，返回 `a` / `[a,b]` 中的约定一侧。
- Ranges 版可直接传任意 range，更方便……
- `std::min_element/max_element/minmax_element(begin, end[, Pred2])`：返回最小/最大元素的迭代器。
- `minmax_element` 的比较次数是 Θ(𝑛) 而不是你以为的 Θ(2𝑛)。细节在《算法设计与分析》里学，这里不展开（知道技巧后很简单）。
- `std::clamp(value, low, high)`：若 `value` 在 `[low, high]` 内则返回 `value`；小于 `low` 返回 `low`；否则返回 `high`。
- 深度学习中常用来限制梯度等。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Permutation

- 排列指两个序列多重集相同但顺序可不同。
- 例如 `{1,2,3}` 与 `{2,1,3}`。
- `std::is_permutation(begin1, end1, begin2[, Pred2])`：判断是否互为排列。`Pred2` 充当 `operator==`。
- 类似 `equal` 与 `mismatch`，自 C++14 起可提供 `end2`。
- 𝑂(𝑁)；若严格相等可更快结束。
- `std::prev/next_permutation(begin, end[, Pred2])`：变成字典序上一/下一个排列。
- 例如 `{1,2,3}` 的 6 个排列按字典序为：
- 即 `{1, 2, 3}, {1, 3, 2}, {2, 1, 3}, {2, 3, 1}, {3, 1, 2}, {3, 2, 1}`。
- `{1, 3, 2}` 的下一个排列是 `{2, 1, 3}`。
- 若已是第一个/最后一个排列，`prev/next` 返回 `false`。可用 `while(next_permutation)` 遍历所有排列（起点应先排序成最小字典序）。
- 𝑂(𝑁)。

# Permutation

- `std::is_permutation` 实现不复杂：先检查长度是否相同。
- 相同则跳过开头已相等的部分。
- 对剩余元素逐个：
- 若它不是所有等价元素中的第一个，就跳过。这里用 `std::find`，𝑂(𝑛)。
- 否则是第一个，统计两序列中等价元素个数，用 `std::count`，𝑂(𝑛)。
- 若计数不同，返回 `false`。
- 因此 `find` 用来避免重复计数。
- 总体 𝑂(𝑛)。
- 注意这对 forward 迭代器也适用；`next/prev_permutation` 只能用于双向迭代器。

# Permutation

- `std::next/prev_permutation` 的实现（以 next 为例）：
- 找最右的、比其右侧邻居小的元素，记 `it1`。
- 若找不到（整体严格降序），返回 `false`。
- 再在右侧找最右的不小于 `it1` 的元素，记 `it2`。
- `std::iter_swap(it1, it2)`，再 `std::reverse(std::next(it1), end)`。

1 4 3 22 4 3 1 iter_swap

it1 it2 reverse

2 1 3 4

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Numeric algorithms

- 都定义在 `<numeric>`。
- 顾名思义，常用于数值计算，因此单独成头。
- 都是 𝑂(𝑁)。
- 通常得到标量或输出序列。
- 最基础的：
- `std::iota(begin, end, beginVal)`：把 `[begin, end)` 填成 `{beginVal, ++beginVal, …}`。
- 任何支持 `operator++` 的类型都可用。
- `std::adjacent_difference(begin, end, dstBegin, Op = std::minus)`：输出 `{ val[0], val[1] – val[0], val[2] – val[1], …}`。
- `–` 可换成 `Op`。

注意返回类型与 `initVal` 相同，因此对 `[0.2, 0.3]`，`float a = std::accumulate(f1,

# Numeric algorithms

f2, 0)` 得不到 `0.5`，而是 `0 + int(0.2) + int(0.3) = 0`……应写成 `0.f`。
- `std::accumulate(begin, end, initVal, Op = std::plus)`：累加 `[begin, end)`，返回 `initVal + Σ val`。
- `+` 可换成 `Op`，例如 `std::multiplies` 得乘积。
- `std::partial_sum(begin, end, dstBegin, Op = std::plus)`：输出 `{ val[0], val[0]+val[1], val[0]+val[1]+val[2], …}`。
- 返回输出序列的尾迭代器。
- 前缀和在区间求和里很有用：`Sum[k2] – Sum[k1]` 得原数组 `[k1,k2)` 的和。
- `std::inner_product(begin1, end1, begin2, initVal, Op1 = std::plus, Op2 = std::multiplies)`：得到 `initVal + a[0]*b[0] + a[1]*b[1] + …`。
- 这三个算法保证按从头到尾顺序执行。
- 若考虑并行，许多运算可独立再汇总。
- 因此自 C++17 起有不保序版本。

# Numeric algorithms

- `std::reduce(begin, end, initVal, Op = std::plus)`：与 `accumulate` 同类思想。
- `std::inclusive_scan(begin, end, dstBegin, Op = std::plus[, initVal])`：类似 `partial_sum`，但可提供 `initVal`，序列变为 `{initVal+val[0], initVal+val[0]+val[1], …}`。
- `std::transform_reduce(begin1, end1, begin2, initVal, ReduceOp = std::plus, BiTransformOp = std::multiplies)`：类似 `inner_product`。
- 不保序，因此允许更多优化。
- 它们也都有并行版本，以后再讲。
- 还有其它无序版本：
- `std::transform_reduce(begin, end, initVal, ReduceOp, UnaryTransformOp)`：二元版是 `Init ReduceOp BiOp(x1,y1) ReduceOp BiOp …`；这一版是 `Init ReduceOp UnaryOp(x1) ReduceOp UnaryOp …`。
- 没有默认运算参数。

# Numeric algorithms

- `std::exclusive_scan(begin, end, dstBegin, initVal, Op = std::plus)`：类似前缀和，但不包含当前元素本身，即 `{initVal, initVal+val[0], …, initVal+val[0]+…+val[n-2]}`。
- 因为是 exclusive，`initVal` 必需，不能像 `inclusive_scan` 那样放在最后当可选参数。
- `std::transform_inclusive_scan(begin, end, dstBegin, Op, UnaryTransformOp[, initVal])`
- 等价于先对每个元素做 `UnaryTransformOp`，再 `inclusive_scan`。
- `std::transform_exclusive_scan(begin, end, dstBegin, initVal, Op, UnaryTransformOp)`
- 等价于先 `UnaryTransformOp`，再 `exclusive_scan`。
- `initVal` 不会被变换。

# Numeric algorithms

# Numeric algorithms

- `<numeric>` 里还有三个算法：
- `std::gcd(a, b)`：自 C++17，返回 `abs(a)` 与 `abs(b)` 的最大公约数，`a,b` 应为非 `bool` 整数。
- 特别地，`a==0 && b==0` 返回 `0`；若只有一个为 0，返回另一个。
- `std::lcm(a, b)`：自 C++17，返回最小公倍数。
- 特别地，`a==0 || b==0` 返回 `0`。
- 若 `abs(result)` 不能用 `a,b` 中较大类型的范围表示，则 UB。
- 例如 `gcd(INT_MIN, 3)` 可能 UB，因为 `abs(INT_MIN)` 不能用 `int` 表示。
- `lcm` 更容易溢出，要小心。
- `std::midpoint(a, b)`：自 C++20，返回 `a + (b - a) / 2`。
- 接受算术类型（`bool` 除外）与指针类型。

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Parallel algorithms

- 可见许多算法都能并行化！
- 单机上有两类并行：
- SIMD：单指令多数据，即向量化，例如 ICS 里听过的 AVX。
- 例如 AVX 可操作 256 位寄存器，装 4 个 `double`，并提供同时对这 4 个数运算的指令。
- SIMT：单指令多线程，更确切说是多线程：用多个线程分块解决问题。
- 例如 `std::fill` 填 1G 数组显然可并行。
- SIMD 下可一次写 32 字节而不是 8 字节，理论上把问题规模缩到约 1/4（相对单标量）。
- SIMT 下可用 4 线程各做约 1/4 工作。
- 总之理论上可能接近 16 倍加速（示例量级）！

注：SIMT 并不等同于一般多线程，它更严格（从名字也能看出）。GPU 上很常见，可自行延伸学习。

# Parallel algorithms

- 自 C++17 起加入并行算法。
- 要给并行提示，使用 `<execution>` 里的策略常量。
- `std::execution` 定义四种执行策略。
- `seq`：顺序策略，算法不得并行化。
- `par`：并行策略，可用多线程。
- 要求多线程同时执行时结果仍正确。
- `unseq`：非定序策略（自 C++20），即可使用 SIMD。
- 要求同一线程内可乱序/向量化执行时结果仍正确。
- `par_unseq`：并行且可向量化。

# Parallel algorithms

- 例如：

- 这例向量化不安全但并行可能安全：单线程不能对 `mutex` 连续 `lock` 多次，但多线程可用 `mutex` 避免数据竞争。
- 也可从数据依赖角度分析。
- 当然，若操作彼此独立（例如所有元素 `+=1`），可用 `par_unseq`。

# Parallel algorithms

- 数据依赖分四类：
- RAR：读后读，无依赖。可任意顺序读。可用 `par_unseq`。
- 例如 `std::adjacent_find`，不写。
- RAW：写后读，例如 `a[j] = a[j-1]+1`。难以分块并行；也难向量化。
- WAR：读后写，例如 `a[j-1] = a[j]+1`。并行仍难，因为 `a[499]` 依赖 `a[500]`。但可向量化：读 `a[1]…a[8]`，算完写回 `a[0]…a[7]`。有时可用 `unseq`，但加上例如 `B[j]=A[j]*2` 仍可能不行。
- WAW：写写冲突，例如 `a[j-1]=a[k]; a[j]=a[k]+1;` 若交错执行结果不确定。

Credit: https://d3f8ykwhia686p.cloudfront.net/1live/intel/CompilerAutovectorizationGuide.pdf

# Parallel algorithms

- 不过加锁后仍可能并行。
- 但要精心设计，临界区太大就退化成单线程串行。
- 也可用原子变量，以后再讲。
- 总之我个人最常用 `par_unseq`；真正性能常靠手写线程划分任务，因为并行算法不指定线程数，粒度太粗。
- 下面谈并行算法细节。
- 需要把执行策略作为首参。
- 只介绍会改变复杂度保证的算法；除非谓词/变换有依赖（例如生成 Fibonacci），否则很多算法都可标 `par_unseq`。

# Parallel algorithms

- 并行本身不改变渐近复杂度（只减常数）；但有些算法不能直接并行，于是换成别的实现，复杂度保证也变。
- 并行算法实现细节本课不展开；若对 HPC 有兴趣可自行学习。
- 对 `std::partition`、`std::nth_element`，并行版复杂度为 𝑂(𝑁 log 𝑁)（但 `==` 仍只需 𝑂(𝑁)）。
- 对 `std::merge`，顺序版恰好 𝑁−1 次比较，并行版只保证 𝑂(𝑁)。
- 对 `std::inplace_merge`，顺序版仅当内存不足时才是 𝑂(𝑁 log 𝑁)；并行版总是 𝑂(𝑁 log 𝑁)。
- 由于 𝑂(log 𝑁) 很小，常数优化仍可能加速（务必实测）。

# Parallel algorithms

- 也有一些算法不能并行化。
- 只处理少量标量的：`std::min/max/minmax/clamp`、`std::swap/iter_swap`。
- 随机算法，本讲不讲。
- 带 `searcher` 的 `std::search`；你可自己写并行 searcher。
- `push/pop/make/sort_heap`，显然不好并行。
- 排列相关算法。
- 二分查找（含 `partition_point`），以及 `copy/move_backward`（可用反向迭代器替代思路）。
- 保序数值算法：`iota`/`accumulate`/`inner_product`/`partial_sum`。
- 另外，所有并行算法至少需要 forward 迭代器（即便策略是 `seq`！）。

# Parallel algorithms

- 若内存不足以准备并行（例如建线程失败），并行算法抛 `std::bad_alloc`；
- 若可调用体（例如变换函数）抛异常，程序会终止（即便策略是 `seq`！）。
- 注意：前面说的 16× 只是理论值。
- 受体系结构限制，加速通常次线性，往往不到 16 倍。
- 例如建线程有开销。
- 因此数据很小时（例如 `size < 1024`），并行实现常退回单线程。
- 此外高优化等级下编译器可能自动向量化。
- 若变换函数极便宜，并行也未必划算。
- 若系统不支持并行/向量化，可能静默退回顺序版本。

总之：请实测（profile）！

# Parallel algorithms

- 并行排序示例：
- 第一个是 Debug，第二个是 Release。
- 我们生成随机数并计时，以后课程会讲。
- 若在我们讲到之前就自己想测，可套用类似框架。

# Parallel algorithms

- 最后提醒：
- libc++ 至今（多年后）仍不支持并行算法。
- libstdc++ 需要链接 TBB 才能启用并行算法，否则仍是顺序。
- 也就是要安装 TBB 并 `-I Your/TBB/Path -ltbb`。
- 用 CMake/XMake 会简单些……

# Algorithms

- Algorithms•Permutation
- Search• Numeric algorithms
- Comparison•Parallel algorithms
- Counting
- Range-version algorithms
- Generating and Transforming
- Modifying
- Copying
- Partition and Sort
- Heap
- Set operations
- MinMax

# Range-version algorithms

- 经典算法要传迭代器对……
- 但我们常操作整个容器，也就是 `begin` 与 `end`。
- 何不直接传 range？
- 于是引入 range 版算法（也称 constrained algorithms），更灵活。
- 定义在 `std::ranges`，例如 `stdr::sort(vec)` 即可！
- 还可把 projection 作为最后参数，即进入真正比较/操作前先投影元素。
- 这只改变“比较准则”，元素本身不变。
- 多 range 可对应多个 projection。
- 输出仍要迭代器，因为原版也不是只传一个 range。

# Range-version algorithms

- 例如：

- 第二个 `{}` 表示使用默认比较器，即 `stdr::less{}`。
- 比较 functor 也有 `stdr` 版本。

# Range-version algorithms

- Range 版还有这些好处：
- 使用 C++20 `concept`，迭代器不满足要求时错误信息更友好。
- range 不必是 common（`begin/end` 类型可不同）。
- 更安全：传临时容器会返回 `stdr::dangling`，使用它会编译失败。
- 指向临时容器底层的 view 也被禁止，例如 `std::vector{1, 2, 3} | std::take(2)`。
- 一些算法增强，例如 `stdr::min/max` 可接受整个 range 并返回 `T`，而不只是两个值或 `initializer_list`。
- 也新增了一些接口。
- 可用模板参数直接接住返回值。
- 还有一些与 ADL 相关的好处，但 ADL 非本课内容。

# Range-version algorithms

- 但也有缺点。
- 不能与并行算法策略一起用。
- 有些能力变弱，例如 `std::search` 不能带 `searcher`。
- 返回值与 `std::` 版不尽相同，迁移要改代码。
- 但它们会返回更完整的信息，也未必算缺点。
- `stdr::remove/unique` 返回被移除区间 `[newEnd, end)`。
- 对需要表示多段结果的算法，返回类型主要有四类：
- `stdr::in_in_result<I1,I2>`：有 `.in1`、`.in2`，表示两段的结束或结果迭代器；用于 `std::swap_ranges` 与 `std::mismatch`。
- `stdr::in_out_result<I,O>`：有 `.in`、`.out`，用于单输入单输出且返回 `{input_end, output_end}` 的算法。
- 一元 `stdr::transform` 也属此类。
- 特别地，`stdr::set_difference` 虽接受两个 range，但只返回第一个 range 的迭代器。

# Range-version algorithms

- `stdr::in_in_out_result<I1,I2,O>`：有 `.in1`、`.in2`、`.out`，广泛用于两输入一输出并返回 `{input1_end,input2_end,output_end}` 的算法。
- 用于 `stdr::set_union/intersection/symmetric_difference`、`stdr::merge` 与二元 `stdr::transform`。
- `stdr::in_out_out_result<I1,I2,O>`：有 `.in`、`.out1`、`.out2`，仅用于 `stdr::partition_copy`（真/假两段输出）。
- 还有一些特殊返回类型。
- `stdr::in_found_result<I>`：`.in` 与 `.found(bool)`；用于 `stdr::next/prev_permutation`。
- `stdr::min_max_result<I>`：`.min`、`.max`；用于 `stdr::minmax(_element)`，前者返回值，后者返回迭代器。
- 结构体也可结构化绑定。

# Set operations

- 现在你能理解前面集合操作示例在说什么了！
- 1. 它们可直接作为模板参数传入。
- 对 `std::` 版，它们是函数模板，不是普通函数对象！
- 传入形式更啰嗦……
- 可把它们想成带 `operator()` 的 functor。
- 2. 用 `.out` 接住输出位置。
- 3. 直接传 range，而不是迭代器对。

# Range-version algorithms

- 数值算法自 C++23 起只提供两个 range 版。
- `std::iota` 对应 `stdr::iota`，但后者返回 `stdr::out_value_result`。
- `std::accumulate` 对应 `<algorithm>` 里的 `stdr::fold_left(R, init, Op)`。
- 也可用 `stdr::fold_left_first(R, Op)`，以首元素为初值；若无首元素则返回 `std::optional<T>`。
- `std::optional` 在错误处理一讲会讲。
- 也可从右向左：`stdr::fold_right_last(R, Op)` 或 `stdr::fold_right(R, init, Op)`，但需要双向 range。
- 还有 `stdr::fold_left_(first_)with_iter`，返回 `stdr::in_value_result`（即 `{end, result}`）。
- 这些在 `std::` 里没有！
- 另有一些 C++23 起只在 `stdr::` 存在的接口：

# Range-version algorithms

- `stdr::contains(R, val[, Proj])`：判断 `val` 是否在 `R` 中。
- `stdr::contains_subrange(R1, R2[, Proj1[, Proj2]])`：判断 `R2` 是否作为子序列出现在 `R1`。𝑂(𝑁)，但不能指定 `searcher`。
- `stdr::starts_with/ends_with(R1, R2[, Proj1[, Proj2]])`：判断前缀/后缀。𝑂(𝑁)。
- `stdr::find_last((_if)_not)`：等价于用反向迭代器做 `find_first`。
- 迭代器相关函数也有 range 版：
- `advance/next/prev` 可把上界作为最后一参，迭代器不会超过它；不确定步长时更安全。
- 例如 `stdr::next(vec.begin(), 100, vec.end())` 在 `vec.size()<100` 时返回 `vec.end()`。
- 还有重载 `(it0, bound)`，等价于 `for(auto it=it0; it!=bound; ++it)`。
- `distance` 也可直接传 range。

# Summary

- Ranges• Algorithms
- Range adaptors 的含义
- Numeric algorithms 与迭代取值
- Ranges 的一些注意点•Parallel algorithms
- Generator•Ranges-version algorithms
- Function
- 成员指针。
- `std::function`。
- `std::reference_wrapper`。
- 更偏好 lambda，而不是 `bind`！
- Transparent operators。
- 对 lambda 的补充。

# Next lecture…

- 下一讲是生命周期与类型安全。
- 前者是新手最常踩的坑……
- 本课也一再强调：要特别关注被引用的变量，无论 lambda 捕获还是 `bind` 配 `std::ref`。
- 后者同样重要，甚至老手也可能忽略，从而产生大量 UB。
