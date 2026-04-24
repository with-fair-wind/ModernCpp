容器，ranges 与算法 Container, Ranges and Algorithm

## 现代C++基础

## Modern C++ Basics

梁嘉铭，北京大学本科生

# 总览

- 在接下来的两讲中，我们将介绍容器、算法（俗称“STL”，这是历史遗留名称）以及 ranges。
- 对于容器（包括迭代器），你们之前已经学过基础内容。
- 这里不会讲授与内存相关的高级技巧，例如 `std::pmr`、分配器（allocator）。我们假定内存由 `new` 分配。
- 对于 ranges，我们主要讲授其用法。
- 对于算法，就是 `<algorithm>`、`<numeric>` 以及它们的扩展。
- 其中大部分你们已经学过，因此我们会补充一些关于实现细节的讲解。
- 这有时有助于你判断什么高效、什么不高效。
- 容器与算法的实现均以微软（Microsoft）版本为准。
- 标准只规定复杂度，而不规定实现细节（以便库厂商在各自平台上优化）。
- 但各家实现大体相似，讲完本课后你也可以自行阅读其它版本，例如 libc++/libstdc++（即 gcc/clang）。

# 总览

- 本讲主要讨论容器。
- 首先我承认：这些内容大多来自 cppreference，以及我阅读的 MS-STL 中的容器代码。
- 我不会、也可能不会去读 Hou-STL，因为它属于 2000 年代风格；SGI-STL 也可能略显陈旧。
- 因此若有错误，欢迎指正！
- 关于容器，有一些通用原则：
- 容器通过模板设计，对一般情况高效，但并非对所有情况都最优。在实际工程中，你可能会使用更适合场景的其它库（若你只是学生，这种情况相对少见……）。
- 有些容器会提供 `<algorithm>` 里也有的方法，但它们通常更高效（否则何必多此一举？）。
- 容器不是线程安全的。
- 当你了解实现方式后，这一点会很容易理解。

### • 第一部分

### • 迭代器

### • 顺序容器

### • `span`、`mdspan`

- `bitset`（不在其中，但很适合放在这里讲）

- `string` 本讲不涉及，将在后续课程中讲解。

### • 容器适配器

### • 关联容器

# 在那之前……

- 在讲这些内容之前，我们先介绍 `<cstddef>` 中定义的两个特殊整型别名：
- `std::size_t`：`sizeof()` 的返回类型；这意味着对象大小不能超过 `std::size_t` 可表示的范围。
- 因此，数组的最大尺寸在 `std::size_t` 范围内，下标也在其范围内。
- 于是，在容器中，所有类似 `.size()` 的接口都会返回 `std::size_t`，所有类似数组 `operator[]` 的接口都会接受 `std::size_t`。
- 它是无符号类型；有符号版本 `ssize_t` 并非标准。
- 自 C++23 起，可以用字面量后缀 `z` 表示有符号的 `std::size_t`，用 `zu` 表示 `std::size_t`。
- `std::ptrdiff_t`：两个指针相减的返回类型。
- 它是有符号类型。

# 在那之前……

- 它们之所以存在，是因为在不同平台上可能不同。
- 例如在 x86 与 x64 上，`size_t` 通常分别为 32 位/64位。
- 需要 `ptrdiff_t`，是因为在一些旧平台上，数组需要用“段（segment）”表示，指针只能在段内对地址做运算。
- 由于它们更“跨平台”，有人建议只用它们，而不要用例如 `int`。
- 我对此略有保留……
- 除非你在写非常底层的基础库（例如 C++ 标准库本身），若你希望在目标平台上获得跨平台的位数保证，更推荐使用例如 `std::uintxx_t`。
- 但对容器中的许多方法，它们可能返回 `std::size_t`，因此你有责任仔细考虑整数的上限与类型转换。

# 容器，ranges 与算法

迭代器

# 迭代器

- 顾名思义，迭代器是对“如何遍历容器以及许多其它可迭代对象”的一种抽象。
- 迭代器共有 6 类：
- 输入/输出迭代器：对于输出，你只能做 `*it = val`、`it++`、`++it` 与 `it1 = it2`，提供只写访问；对于输入，你还可以使用 `==`、`!=` 与 `->`，提供只读访问。
- 这两类迭代器通常不用于容器本身，而用于例如算法对迭代器的要求等场景。

- 前向迭代器：与输入迭代器相同，并且还可以被拷贝或默认构造。
- 这是容器能支持的最弱迭代器（例如单向链表）。
- 双向迭代器：与前向迭代器相同，并且还可以做 `--it`、`it--`。
- 可以一步一步双向移动，例如双向链表、`map`。

# 迭代器

- 随机访问迭代器：与双向迭代器相同，并且还可以与一个整数做 `+`/`-`/ `+=`/`-=`/`[]` 比较，以及用 `</>/<=/>=` 比较（但为了通用性，循环里通常仍用 `!=`）。
- 例如 `deque`；在操作上它几乎等价于指针。
- 连续迭代器（contiguous iterator，自 C++17）：与随机访问迭代器相同，并额外保证迭代器所指向的内存是连续的。
- 例如 `vector`、`string`。
- 这些迭代器之间并没有继承层次，尽管要求确实是逐级变严。
- 自 C++20 起，它们成为 concept，因此当你的迭代器不满足某些要求时，错误信息会更可读。
- 它们会用于面向 ranges 的算法；我们稍后会讲到！

注意：C++20 的迭代器 concept 与 C++17 的要求在约束上并不完全相同；这里也不会覆盖所有细节要求。若你有兴趣可自行查阅。

# 迭代器

- 重要：迭代器与指针一样不安全。
- 它们可能失效，例如越界。
- 即便它们来自不同容器，也可能被混用！
- 其中一部分可以在较高的迭代器调试等级下被检查。

- 所有容器都可以通过以下方式获取迭代器：
- `.begin()`、`.end()`
- `.cbegin()`、`.cend()`：只读访问。
- 除了仅支持前向迭代器的容器（例如单向链表）之外：
- `.rbegin()`、`.rend()`、`.crbegin()`、`.crend()`：反向迭代器，即向后遍历。

# 迭代器

它们也有 ranges 版本，例如 `std::ranges::begin`；自 C++20 起请使用它们（原因稍后说明）！
- 你也可以使用全局函数获取迭代器：
- 例如 `std::begin(vec)`、`std::end(vec)`。
- 它们定义在任意容器头文件中。
- 注意：指针也是迭代器！
- 因此，对于数组类型（不是指针类型），例如 `int arr[5]`，你也可以使用 `std::begin()`、`std::end()` 等。
- 我们得到的只是两个指针，例如这里是 `arr` 与 `arr + 5`。
- `<iterator>` 中还定义了迭代器操作的通用方法。
- `std::advance(InputIt& it, n)`：相当于 `it += n`（对非随机访问迭代器，则递增 `n` 次）。`n` 可以为负，但迭代器应为双向迭代器。
- `std::next(InputIt it, n = 1)`：返回 `it + n`，不改变原迭代器。
- `std::prev(BidirIt it, n = 1)`：返回 `it - n`，不改变原迭代器。
- `std::distance(InputIt it1, InputIt it2)`：返回 `it2 – it1`（对非随机访问迭代器，则不断递增 `it1` 直到 `it1 == it2`）。

# 迭代器特征（iterator traits）

- 迭代器会提供一些类型信息：
- `value_type`：所引用元素的类型。
- `difference_type`：可用于表示元素间距的类型（通常是 `ptrdiff_t`）。
- `iterator_category`：例如 `input_iterator_tag`。自 C++20 起增加了 `continuous_iterator_tag`。
- 在 C++20 中，若可用，更推荐使用 `iterator_concept` 而不是 `category`，它对迭代器（尤其是 C++20 ranges 的迭代器）描述更精确。
- `pointer`：指向所引用元素的指针类型，仅在容器迭代器中可用。
- `reference`：所引用元素的引用类型，仅在容器迭代器中可用。
- 你可以使用 `std::iterator_traits<IteratorType>::xxx`（定义在 `<iterator>`）来获取它们（缺失的成员会是 `void`）。

# 迭代器特征（iterator traits）

- 自 C++20 起，你也可以直接这样获取：
- `std::iter_value_t<IteratorType>` / `std::iter_reference_t<IteratorType>` / `std::iter_const_reference_t<IteratorType>` / `std::iter_difference_t<IteratorType>`
- 指针与 category 不会直接以这些别名提供。

出处：Nicolai M. Josuttis 著《C++20 The Complete Guide》

# 流迭代器（stream iterator）

- 除了容器的迭代器之外，标准库还提供流迭代器与迭代器适配器。
- 对于流迭代器……
- 当你以简单且固定的模式从输入流读取/向输出流写入时，可以使用 `std::istream_iterator<T>` 与 `std::ostream_iterator<T>`（分别为输入与输出迭代器）。
- 它们由流初始化，例如 `std::cin`/`std::cout`。
- `istream_iterator` 的初始化会触发第一次读取：`*` 取得当前值，`++` 触发下一次读取。
- 例如：

# 流迭代器

- 不过，它们最常与 `std::(ranges::)copy` 一起使用。

- 默认构造的 `istream_iterator` 表示“结束”；这意味着会一直读直到输入流无法解析为 `int`（例如输入了非数字字符）或遇到流错误（例如文件流读到文件末尾）。
- 因此这很危险：你无法假设用户的输入，而且 `vector` 的迭代器也可能越界……
- 有人希望使用 `std::copy_n(std::istream_iterator<int>{std::cin}, vec.size(), vec.begin())`，但若输入流提前结束，解引用同样会失效……
- 并不存在 `copy_until`！

# 迭代器适配器

- 迭代器适配器有两类：
- 一类由迭代器构造，用于实现不同用途：
- 例如反向迭代器：底层类型仍是迭代器，但 `++` 实际上是 `--`。
- 你可以从一个迭代器构造，例如 `std::reverse_iterator r{ p.begin() }`。
- 你可以通过 `.base()` 取得底层迭代器；它实际返回的是指向“当前所引用元素之后”的那个位置的迭代器。
- 例如 `rbegin().base() == end()`
- 还有另一种适配器叫移动迭代器（move iterator），将在《移动语义》一讲中介绍。

# 迭代器适配器

- 另一类由容器构造，用途不止于“遍历”。
- `std::back_insert_iterator{container}`：`*it = val` 会调用 `push_back(val)` 插入。
- `std::front_insert_iterator{container}`：调用 `push_front(val)` 插入。
- `std::insert_iterator{container, pos}`：调用 `insert(pos, val)` 插入，其中 `pos` 应为容器内的迭代器。
- 它们都是输出迭代器，值由赋值提供。
- 例如：

# 迭代器适配器

- 注意：对 `vector`/`deque` 而言，直接插入/赋值一个范围通常比用 `inserter` 一个一个插入更好。
- 或者至少先 `reserve`（我们很快会学到）。
- 最后：`std::make_xxx` 或 `std::xxx`（例如 `std::back_inserter()`、`std::make_reverse_inserter()`）这类函数也存在；你也可以用它们获得对应的迭代器适配器。
- 在 C++17 之前，你必须为类模板的类型参数显式写出类型，因此若不用这些函数，就得写冗长的 `std::back_inserter_iterator<std::vector<int>>(vec)`。
- 自 C++17 起，CTAD（类模板实参推导）可以推导类型，因此这些工厂函数未必比对象初始化更短。

# 容器，ranges 与算法

顺序容器

# 容器

- 顺序容器
- `array`
- `vector`
- `bitset`
- `span`
- `deque`
- `list`
- `forward_list`

# `array`

- 我们学过 C 风格数组，例如 `int a[5];`
- 然而，把它传给函数时会退化为 `int*`，尺寸信息会丢失。
- 也就是说，数组形参的第一维没有意义：`void func(int a[5])` 与 `void func(int a[])` 或 `void func(int* a)` 实质相同。
- 因此，在函数内外 `sizeof(a)` 会不同……
- 返回类型也不能是 `int[5]`……
- 对 `int a[5], b[5]`，`a = b` 非法。
- 边界从不检查，因此可能访问无效内存……
- 总之，我们需要更安全的数组！
- `std::array<T, size>` 就是为此而生。
- 它与 `T[size]` 相同，只是它始终保存尺寸信息，可以从另一个数组拷贝，并且能做更多事情，例如边界检查。

# `array`

- 因此，它通常分配在栈上！
- 呃，但如果你 `new std::array`，那它仍然在堆上。
- 构造函数：与 C 风格数组同样的初始化方式（有时需要额外加一层括号）。
- 例如：`struct S {int i; int j;};`，`std::array<S,2> arr{{ {1,2}, {3,4} }}`。
- 成员访问：
- `operator[]`/`at()`：按下标访问；`at()` 会检查边界，即若下标大于等于 `size`，会抛出 `std::out_of_range`。
- `front()`/`back()`：取得首/尾元素。
- 连续迭代器，如前所述。
- 若你想取得数组内容的原始指针，可使用 `.data()`。

# `array`

- 你还可以使用一些额外方法：
- `.swap()`：与 `std::swap(arr1, arr2)` 相同。
- `operator=`、`operator<=>`。
- 这些方法要求两个数组尺寸相同！

- `.fill(val)`：把所有元素填为 `val`。
- `std::to_array(C 风格数组)`：自 C++20，从 C 风格数组得到 `std::array`。
- 尺寸相关操作：
- `.size()`：取得尺寸（返回 `size_t`）。
- `.empty()`：返回 `bool`，表示 `size == 0` 是否成立。
- `.max_size()`：取得本系统上可能的最大尺寸（通常没什么用）。
- 很简单，因此我们很快带过……

# 基础

- 顺序容器
- `array`
- `vector`
- `deque`
- `list`
- `forward_list`

`push_back(5)`

# `vector`：`1 2 3 4` → `1 2 3 4 5`

- 准确地说，`vector` 是可以改变大小的动态数组。
- 它支持随机访问，并占用连续空间。
- 在尾部插入与删除元素（即 `push`/`pop` back）时，复杂度为摊还 𝑂(1)。
- 若不在尾部，则为 𝑂(𝑛)。
- 这个容器非常重要，我们会花很多时间讲它。
- 它是所有容器中最常用的。
- 尽管有时它的理论复杂度可能高于其它容器（例如 `list`），但由于缓存友好，实践中仍常常是更好的选择。
- 我们在 ICS 课上学过：缓存友好的程序可能比糟糕的程序快成百上千倍。
- 若你无法判断哪个更好，请用性能分析器（profiler）！

# `vector`

- 首先复习一下“摊还（amortized）”是什么意思。
- 有时我们讨论最坏复杂度。
- 有时我们可能假设情况分布（最常见的是均匀分布），从而得到平均/期望复杂度。
- 例如快速排序：最坏 𝑂(𝑛²)，平均 𝑂(𝑛log𝑛)。
- 对数据结构，也常用摊还复杂度。总复杂度 / 操作次数
- 也就是说，若连续做一系列操作，摊还复杂度就是：总复杂度除以操作次数。
- 因此“尾部插入摊还 𝑂(1)”意味着：若你从尾部连续 `push` 很多元素，其中某些单次操作可能很慢（对 `vector` 通常是 𝑂(𝑛)），但它“很少发生”，从整体看平均每次操作只有 𝑂(1) 时间。

# `vector`

- 因此，最朴素的 `vector` 版本是：*
- `push_back` 时：分配连续空间，尺寸加一，把所有元素拷贝过去并把新元素放到新空间，最后释放旧空间。
- 你不能只分配一块新空间，因为那样无法保证数组性质——“连续空间”。
- `pop_back` 时：把空间缩小一，把剩余元素拷贝到新空间，最后释放旧空间。
- 显然，每次 `push` 或 `pop` 都是 𝑂(𝑛)……
- 那么，若我们在分配时“预留”比当前需要更多的空间，使得 `push` 只需在尾部构造新对象呢？
- 这就是 𝑂(1)，我们只需控制重分配很少发生，从而使拷贝的摊还代价为 𝑂(1)。
- 元素个数叫 `size`；总空间叫 `capacity`。

实践中通常是“移动”而不是拷贝，但这不影响这里的理论复杂度讨论。我们会在《移动语义》中讲“移动”。

# `vector` 的重分配策略

- 最简单的策略是线性增长空间。
- 例如 `0->4->8->12->16…`
- 每 𝑘 次操作触发一次重分配并拷贝 𝑛 = 𝑘𝑚 个元素。∑𝑖=1..𝑘 𝑖
- 因此摊还复杂度为 Θ(∑𝑖/𝑘𝑚) = Θ(𝑚) = Θ(𝑛/𝑘)。
- 考虑到 𝑘 是常数，这仍是 𝑂(𝑛)。
- 这意味着“线性增长”并不“罕见”！
- 那么指数增长呢？
- 例如 `1->2->4->8->16->32…`
- 每 2ᵏ 次操作触发一次重分配并拷贝 𝑛 = 2ᵏ 个元素。∑𝑖=1..𝑘 2ⁱ
- 因此摊还复杂度为 Θ(2ᵏ/2ᵏ) = Θ(1)。
- 等差数列求和 vs 等比数列求和。

这里假设“重分配发生在最后一步”（最坏情况）。你也可以分析更一般的情形，或在清华数据结构课上观看相关内容。

# `vector` 的重分配策略

- `vector` 也支持插入一个范围。
- 因此可能一次插入多个元素。
- 考虑到某些插入会导致增长超过 2 倍。
- 你可以计算满足需求的最小 `capacity`。
- 在 MS 实现中，它会直接分配所需空间，这样更便宜。
- 下面我用图示展示上述过程……

# `vector` 的重分配策略

- 情形 1：`push_back(3)`
- 已满，因此需要重分配：`capacity = 2` → `1 2` → `size = 2`

# `vector` 的重分配策略

- 指数式重分配……

`1 2` `capacity = 4` `size = 2`

# `vector` 的重分配策略

- 拷贝所有元素，`push_back(3)`

`1 2` `capacity = 4` `size = 3` `1 2 3`

# `vector` 的重分配策略

- 释放原空间……

`capacity = 4` `size = 3` `1 2 3`

# `vector` 的重分配策略

- 情形 2：`insert(vec.end(), {4,5,6,7,8,9})`
- 最终尺寸为 `3+6=9 > 4*2=8`
- 对普通指数增长……

`1 2 3` `capacity = 16` `size = 3`

# `vector` 的重分配策略

- 情形 2：`insert(vec.end(), {4,5,6,7,8,9})`
- 最终尺寸为 `3+6=9 > 4*2=8`
- 对普通指数增长（拷贝、插入、释放）……`capacity = 16` `size = 9`

`1 2 3 4 5 6 7 8 9` `3`

# `vector` 的重分配策略

- 情形 2：`insert(vec.end(), {4,5,6,7,8,9})`
- 最终尺寸为 `3+6=9 > 4*2=8`
- 对 MS 实现（为简化，这里把所有步骤合并）……

`capacity = 9` `size = 9`

`1 2 3 4 5 6 7 8 9`

# `vector` 的重分配策略

- 最后，为什么指数是 2？
- 你可以证明：对任意大于 1 的指数，摊还复杂度都是 𝑂(1)。
- 这是空间与时间的权衡。
- 若指数太低，重分配会更频繁，从而使 𝑂(1) 的常数因子变大。
- 若指数太高，空间会迅速膨胀，可能浪费很多内存。
- 这只是实践中的一种选择（例如 gcc）。
- 在 MS 中，它是 1.5。
- 这考虑的不仅是简单权衡。
- Facebook Folly 文档：尽管其它编译器把增长因子降到 1.5，gcc 仍坚持维持因子 2。这使得 `std::vector` 对缓存与内存分配器都不够友好。
- 下面解释这意味着什么！

# `vector` 的重分配策略

- 我们假设起始地址为 `addr`，并且只有 `vector` 需要分配内存……

`addr`

# `vector` 的重分配策略

- 对 ×2 策略……

`addr` `addr+4`

# `vector` 的重分配策略

- 对 ×2 策略……

`addr` `addr+4`

# `vector` 的重分配策略

- 下一次重分配尺寸为 16，无法复用 `addr`–`addr + 4`……

`addr` `addr+12`

……

合计 16
- 你会发现重分配永远不会利用已释放的空间！2ᵏ⁻¹ + 2ᵏ⁺¹
- `1 + 2 + ⋯ + 2ᵏ < 2ᵏ⁺¹`。

# `vector` 的重分配策略

- 对 ×1.5 策略……

`addr` `addr+4`

# `vector` 的重分配策略

- 对 ×1.5 策略……

`addr` `addr+4`

# `vector` 的重分配策略

- 对 ×1.5 策略……

`addr` `addr+4`

`addr+10`

# `vector` 的重分配策略

- 对 ×1.5 策略……

`addr` `addr+10`

……

合计 9

# `vector` 的重分配策略

- 对 ×1.5 策略……

`addr` `addr+10`

……

`addr+19` 合计 9

…

合计 13

# `vector` 的重分配策略

- 对 ×1.5 策略……

`addr` `addr+19`

……

合计 13

# `vector` 的重分配策略

- 下一次分配为 `13 * 1.5 = 19`，因此可以复用 `addr`–`addr+19`！

`addr` `addr+19`

……

合计 13

2ᵏ⁻¹ + 2ᵏ⁺¹
- `1.5ᵏ + 1.5ᵏ⁻¹ + ⋯ + 1.5 = 2 * (1.5ᵏ - 1.5ᵏ⁻¹) > 1.5ᵏ` 可能成立。
- 在实践中对内存管理与缓存更友好。

# `vector`

- 总结：
- `vector` 就是动态数组。
- 它占用连续空间，并可用 `[]` 随机访问。
- 它的成员大致包括：指向内容的指针、`size` 与 `capacity`。
- 在实现中，它们通常对应 `first` 指针、`last` 指针与 `end` 指针。

- 当 `vector` 满时，基本会按指数方式重分配，从而使 `push_back` 为 𝑂(1)。

# `vector`

- 显然，`pop_back` 是 𝑂(1)。`size`
- 你可能会以为当 `size` 相对 `capacity` 太小时 `vector` 会收缩！
- 分析与追加类似，可以证明自动收缩也可做到摊还 𝑂(1)。
- 然而实践中，为了效率，`vector` 通常不会自动收缩，但提供手动收缩的手段。
- 此外，自动收缩会破坏关于迭代器失效的规定，我们稍后会讲。

- 对于插入，你可能学过的实现是：
- 从最后一个元素开始向后移动（避免覆盖）。
- 再插入到空出的位置。

# 插入

- `insert(vec.begin() + 1, {4, 5})`

`capacity = 6` `size = 4` `1 2 3 4`

# 插入

- 向后移动……

`capacity = 6` `size = 6` `1 2 3 4` `2 3 4`

# 插入

- 插入

`capacity = 6` `size = 6` `1 4 5` `2 3 4`

# 插入

- 若从 `begin` 向前移动……

覆盖！

`1 2 3 4` `1 2 3` `2`

错误拷贝……

`1 2 3 2` `3 2` `1 2 3 2` `3`

# `vector`

- 删除类似，但从删除区间末尾向前移动到删除点，最后析构尾部若干元素。
- 你可以自己画一画。
- 若需要重分配，可以把“从旧到新拷贝”与“插入中的内部移动”合并。

- 然而 MS-STL 的插入实现并不采用这种方式（原因以后讲）；它是：
- 若需要则重分配（与普通插入相同）。
- 把所有元素逐个 `push_back`。
- 再旋转（rotate）到插入位置。

但删除仍与我们描述的一致。

# 插入

- `insert(vec.begin() + 1, {4, 5})`

`capacity = 6` `size = 4` `1 2 3 4`

# 插入

- 逐个 `push_back` 所有元素……

`capacity = 6` `size = 6` `1 2 3 4` `4 5`

# 插入

- 旋转（这是 𝑂(𝑛)，下一讲会讲……）

`capacity = 6` `size = 6` `1 2 3 4` `4 5`

# `vector`

- 下面看 `vector` 提供的方法（未说明则返回 `void`）：
- 构造函数：
- 默认构造。
- 拷贝构造与移动构造。
- `(size_t count, const T& elem = T{})`：构造包含 `count` 个 `elem` 副本的 `vector`。
- `(InputIt first, InputIt last)`：把 `[first, last)` 的元素拷贝进 `vector`。
- `(std::initializer_list<T>)`：把初始化列表中的所有元素拷贝进 `vector`。
- 所有构造函数都可以带可选的分配器参数。

# 初始化列表

- 那么 `std::initializer_list` 是什么？
- 在列表初始化中，我们可能用 `{ 1, 2 }` 向构造函数/函数传参。
- 然而，如何统一 `vector` 与 C 数组的初始化？
- 总之，我们需要把“一串元素”传给构造函数！它如何表示？
- 通过 `std::initializer_list`。
- 因此，当类接受 `std::initializer_list<T>` 时，元素类型为 `T` 或可转换为 `T` 的 `{ … }` 会被当作 `std::initializer_list<T>`，而不是多个独立参数！
- 于是你可以用 `std::vector<int> v{1,2,3,4}` 初始化 `vector`（统一初始化）。
- 因此当你真正想用 `(size_t count, const T& elem = T{})` 时，若 `T` 例如是 `int`，你不能用 `v{10, 1}` 构造“10 个元素都为 1”的 `vector`。
- 你必须用 `v(10, 1)`。
- 最后，`std::initializer_list<T>` 大致可看作底层有 `const T[N]`，并提供 `begin`、`end` 与 `size`。

- 成员访问（与 `array` 相同）：
- `operator[]`/`at()`：按下标访问；`at()` 会检查边界，即若下标大于等于 `size`，抛出 `std::out_of_range`。
- `front()`/`back()`：取得首尾元素。
- 连续迭代器，如前所述。
- 若你想取得 `vector` 内容的原始指针，使用 `.data()`。
- 容量相关操作（即调整内存）：
- `.capacity()`：取得容量（返回 `size_t`）。
- `.reserve(n)`：若 `n` 大于当前容量则扩展内存使 `capacity` 至少为 `n`（否则什么都不做）；但 `size` 不变。
- 你可以先 `reserve` 来避免反复重分配（尤其多次 `push_back`）！
- 这在某些并行程序中极其重要，因为涉及迭代器失效；我们稍后会讲。
- `.shrink_to_fit`：请求把容量收缩到 `capacity == size`。
- 这是你手动收缩的常用方式；请求可能被接受也可能不被接受。
- 对主流实现（libc++/libstdc++/MS STL），只要你的类型可以无异常地拷贝或移动，并且空间足够分配新 `vector`，基本都会真的收缩。这与异常保证有关，后续课程会讲！

# `vector`

- 尺寸相关操作（即操作元素，可能间接触及容量）
- `.size()`：取得尺寸，返回 `size_t`。
- `.empty()`：返回 `bool`，表示 `size == 0` 是否成立。
- `.max_size()`：取得本系统上可能的最大尺寸（通常没什么用）。
- `.resize(n, obj=Object{})`：使 `size = n`；
- 若原 `size` 已是 `n`，什么都不做。
- 若大于 `n`，`[n, end)` 的元素会被移除。
- 若小于 `n`，会插入新元素，值均为 `obj`。
- `.clear()`：移除所有元素；之后 `size` 为 0。
- 但容量通常不会改变！若你想同时释放内存，需要显式使用容量相关接口。

# `vector`

- `.push_back(obj)`：在尾部插入元素。
- `.emplace_back(params)`：在尾部就地构造元素。
- 自 C++17 起，它返回所插入元素的引用（之前返回 `void`）。
- `.pop_back()`：从尾部移除元素。
- `.insert(const_iterator pos, xxx)`：在 `pos` 处插入，`vec[pos – begin]` 成为第一个被插入元素。`xxx` 与构造函数参数类似：
- `(value)`：插入单个元素。
- `(size_t count, value)`：插入 `count` 个 `value` 的副本。
- `(InputIt first, InputIt last)`：插入 `[first, last)` 的内容。
- `(std::initializer_list<T>)`：插入初始化列表内容。
- `.emplace(const_iterator pos, params)`：在 `pos` 处就地构造元素。

# `vector`

- `.erase(const_iterator pos)` / `.erase(const_iterator first, const_iterator last)`：删除单个元素 / 删除 `[first, last)`。`first`、`last` 必须是本 `vector` 的迭代器。
- `insert`/`erase` 会返回指向插入/删除之后下一个有效位置的迭代器，因此你可以写 `it = vec.erase(…)` 继续遍历。原因稍后说明。
- 与另一个 `vector` 交互：
- `.assign`：也与构造函数类似
- `(vec)`：等价于 `operator=`，赋值为另一个 `vector`
- `(count, const T& value)`
- `(InputIt first, InputIt last)`
- `(std::initializer_list<T>)`。
- `.swap(vec)`：与另一个 `vector` 交换，等价于 `std::swap(vec1, vec2)`。
- 自 C++23 起增加 ranges 相关方法。
- `.assign_range(Range)`：把任意 range 拷贝进 `vector`。
- `.insert_range(const_iterator pos, Range)`
- `.append_range(Range)`：从尾部追加 range。

# 迭代器失效

- 显然，迭代器被设计成“指向元素的指针”的包装。
- 所有操作本质上都是对指针操作，例如 `+/-` 就是移动指针。
- 但指针不安全！
- 迭代器可能不安全，因为它可能无法正确表示所遍历对象的状态。原因包括：
- 重分配：原指针悬空；解引用迭代器会访问未知内存。
- 插入与删除：原指针可能指向“并非你本意”的元素。
- 例如 `1 2 3 4`，`it` 指向 `3`；删除 `2` 后，它实际指向 `4`，而这在原语境中等价于原来的 `it + 1`！
- 这称为迭代器失效（iterator invalidation）。

# 迭代器失效

- 对 `vector`：
- 若 `capacity` 改变，所有迭代器失效。
- 若 `capacity` 不变但某些元素被移动，则“变更点之后”的迭代器失效。
- 也就是说，插入/删除会使插入/删除点之后的迭代器失效。
- 因此 `insert`/`emplace` 会返回指向插入元素的新迭代器，`erase` 会返回最后一个被删元素之后的迭代器。
- 你可以用它们继续遍历 `vector`。
- 我们说 `vector` 线程不安全，因为：
- 只有两个线程都在读 `vector` 时才安全。
- 若一个线程写、另一个读，可能读到不一致内容（例如 `vector<pair>` 可能读到旧的 `first` 与新的 `second`……）。
- 当容器内部结构改变（例如插入时 `vector` 重分配），另一线程可能访问无效内存（即意外的迭代器失效）……

# `vector`

- 但若能保证线程只写不同元素，基本还可以。
- 特别地，`vector<bool>` 若两个位在同一字节里仍然危险（下一页你会理解）……

- 最后：
- `vector` 支持比较，如我们在 `operator<=>` 一讲所述。
- 若你想删除 `vector` 中所有等于 `XXX` 的元素，反复使用 `erase` 代价很高（每次删除往往为 𝑂(𝑛)）……
- 下一讲会教你们 𝑂(𝑛) 的方法。
- 自 C++20 起可以直接用 `std::erase(vec, val)` / `std::erase_if(vec, func)`；它们返回被删除元素个数。

# `vector<bool>`

- `vector<bool>` 是 `vector` 的一个奇怪特化……
- 布尔值只需 1 位表示，因此标准规定 `vector<bool>` 被压缩为“位的动态数组”。
- 然而，CPU 能直接操作的最小单位是字节，这里不能对 `operator[]` 返回 `bool&`！
- 返回的是一个代表该位的代理类（proxy）。
- 对 `const` 成员函数，仍返回 `bool`。
- 你可以通过这个代理读写该位，就像普通引用一样。
- 这有时令人困惑，例如：
- 对 `vector<int>`，`auto a = vec[1]; a = 1;` 不会改变 `vector`，因为 `auto` 不会推导成引用。
- 但对 `vector<bool>`，`auto` 是代理对象，它持有该位的引用，因此会改变 `vector`！
- 基于范围的 `for` 可能使用 `auto`，因此要特别注意！

# `vector<bool>`

- 此外，由于返回的代理是值类型而不是引用，返回对象是临时的！
- 于是遍历 `vector<bool>` 时不能用 `auto&`，尽管对其它类型可以……
- 总结：若要修改 `vector<bool>` 的元素，用 `auto` 而不是 `auto&`；若不想修改，用 `const auto&` 或 `bool`。
- 特化还带来更多方法……
- 代理支持 `operator~` 与 `flip()`，会翻转所指位；
- `vector<bool>` 支持 `flip()`，会翻转向量中所有位。
- `vector<bool>` 支持 `std::hash`，将在无序 `map`（哈希表）一讲中介绍。
- 最后
- 由于其对泛型代码与新手不友好等性质，许多人不鼓励使用 `vector<bool>`。此外，按位操作也比按字节慢，在现代计算机上“省内存”往往并不必要。
- 其迭代器也不被视为连续迭代器。
- 因此若你要使用/处理这种类型，务必谨慎！

# 容器

- 顺序容器
- `array`
- `vector`
- `bitset`
- `span`
- `deque`
- `list`
- `forward_list`

# `bitset`

- `bitset` 其实不是容器；我们在这里讲它，只是因为它也像 `vector<bool>` 那样有很多位……
- 然而其尺寸在编译期确定，即你必须写 `bitset<size>`。
- 从 `vector<bool>` 到 `bitset`，类似于（但不等同于）从 `vector` 到 `array`！
- 区别：
- `bitset` 不提供迭代器。
- `bitset` 提供更多方法，是更合适的位操作方式。
- 你可以使用 `&`、`|`、`^`、`~`、`<<`、`>>`，像操作二进制串一样。
- 你可以用 `set()`、`set(pos, val = true)`、`reset()`、`reset(pos)`、`flip()`、`flip(pos)` 把所有位设为 1/0/翻转，或把 `pos` 位设为指定值/0/翻转。
- `pos` 是下标（`size_t`），因为 `bitset` 不支持迭代器。
- 你可以用 `all()`、`any()`、`none()`、`count()` 检查是否全为 1/是否存在 1/是否全为 0/统计为 1 的位数。
- 也可以用 `>>`/`<<` 做输入输出。

# `bitset`

- 此外，`bitset` 可通过 `.to_string(zero = '0', one = '1')` / `.to_ullong()` 转为 `std::string` / `unsigned long long`。
- 前者在 `string` 分配失败时可能抛出 `std::bad_alloc`。
- 后者若值无法用 `unsigned long long` 表示，可能抛出 `std::overflow_error`。
- `bitset` 也可由 `string` / `unsigned long long` 构造，即 `(str, start_pos = 0, len = std::string::npos, zero = '0', one = '1')`。
- 不必死记；需要时查 cppreference。
- 相似点：
- 你可以用 `operator[]` 访问位，同样返回代理类（`const` 成员函数仍返回 `bool`）。
- `bitset` 没有 `at(pos)`，但有 `bool test(pos)`，会做边界检查。
- 可以比较两个 `bitset`（仅同尺寸，且只有 `==` 与 `!=`）。
- 可用 `.size()` 取得尺寸。
- 可用 `std::hash` 做哈希。

# 容器

- 顺序容器
- `array`
- `vector`
- `bitset`
- `span`
- `deque`
- `list`
- `forward_list`

# `span`

- 自 C++17 起，越来越多“视图（view）式”的东西被提供。
- View 意味着它并不真正拥有数据；它观察数据。
- 因此其构造与拷贝比传统容器便宜得多。
- `span` 是连续内存的视图（例如 `vector`、`array`、`string`、C 风格数组、初始化列表等）。
- 以前你可能写 `void func(int* ptr, int size)`。
- 即便有 `vector`，你也可以传引用；但若你只想操作例如“子 `vector`”呢？
- 你得拷贝到新容器，代价很高……
- `span` 就是为这种场景；你可以写 `void func(std::span<int> s);`。

# `span`

- 你几乎可以像操作数组一样操作 `span`。
- 可以使用随机访问迭代器。
- 可以使用 `front()`/`back()`/`operator[]`/`data()`。
- 可以使用 `size()`/`empty()`。
- 也可以使用 `size_bytes()` 取得字节数。
- 你可以很廉价地创建新的子 `span`：
- `.first(N)` / `.last(N)`：取前 `N` 个/后 `N` 个元素构成子 `span`。
- `.subspan(beginPos(, size))`：从 `beginPos` 开始、长度为 `size` 的子 `span`（默认直到最后一个）。
- 记住：`span` 只是一个指针加尺寸！所有“拷贝式”操作都很便宜。你也可以用 `std::as_bytes` 与 `std::as_writable_bytes` 把 `span` 转成字节 `span`。

# `span`

注意：如果你从 `vector` 创建 `span`，而例如 `vector` 发生了重新分配（即 `capacity` 改变），则原来的 `span` 将悬空！

始终记住：它与指针一样不安全。

- 你也可以用 `[begin, end)` 迭代器对或 `(begin, size)` 对创建 `span`。
- 注意：`span` 永远不会（C++26 增加了 `.at()`）检查访问位置是否合法！
- 例如你可以对 `operator[]` 使用越界下标。
- 你必须小心管理！
- `span` 实际是 `std::span<T, extent>`，但 `extent` 默认为 `std::dynamic_extent`。
- 对固定 `extent` 更危险，因为你可以赋给一个“实际上没有 `extent` 个元素”的范围（但需要显式构造）。
- 只有 C 风格数组（即 `T[extent]`）与 `std::array<T, extent>` 可以隐式构造它。
- 你也需要 `.first<N>()`/`.last<N>()`、`.subspan<offs, N>()` 来创建固定 `extent` 的子 `span`（对固定/动态 `span`，`.subspan<offs>()` 会创建固定/动态子 `span`）。
- 可用静态成员 `extent` 取得 `extent`。

对非动态 `extent`，对象只存指针，因为 `extent` 就是尺寸。

# `span`

- 注意：自 C++17/20 起，`std::data()`/`empty()`/`size()`/`ssize()` 等可用于取得原始指针等，就像 `std::begin` 提取迭代器一样。
- 但在 C++20 中，你应更偏好 `std::ranges::data()`/`begin()`/……，它更安全且有其它优势；ranges 下一讲会讲。

- 最后：若你希望 `span` 只读，应使用 `std::span<const T>`。
- 这会使指针成为 `const T*`，因此只读。
- 但对容器，你需要写成 `const std::vector<int>`。
- 这是因为 `span` 是观察者，而容器是拥有者！

# `mdspan`（选讲）

- 你可能会觉得用容器声明多维数组很麻烦……
- `vector<vector<int>>` 实际上类似：
- 内存四处散落，对缓存不友好。
- 我们希望多维数组像下面这样连续：

# `mdspan`（选讲）

- 讲到多维下标 `operator[]` 时，我们实现过一个很简单的多维数组：用一维数组做缓冲区，并重载 `operator[]` 让它像多维数组一样访问。
- 自 C++23 起也提供了多维 `span`。
- 它仍是非拥有的视图，提供多维 `operator[]`！
- 对拥有内存的版本，`mdarray` 可能在 C++26 提供……
- 要覆盖全部内容相当复杂，且当前库支持不足，因此这里只做介绍。
- 本部分是可选的；若你不能完全理解也完全正常！

# `mdspan`（选讲）

- `mdspan` 有三个组成部分：
- Extent：我们也需要多维尺寸；因此写成 `std::extent<IndexType, sizes…>`。
- 例如 `std::extent<std::size_t, 3, 2, 4>`。
- 类似地，若你希望某些维度是动态的，仍可使用 `std::dynamic_extent`，例如 `std::extent<std::size_t, std::dynamic_extent, 2, std::dynamic_extent>`。
- 显然最常用的是所有维度都动态，因此可简写为 `std::dextent<IndexType, dimensionNum>`。
- Layout：默认是 `std::layout_right`。
- 你可能知道 Fortran 与 C/C++ 的数组布局不同。
- 在 C/C++ 中，最后一维（最右边）是连续的。
- 行主序（row major），即一行一行存储。
- 在 Fortran 中，第一维（最左边）是连续的。
- 列主序（column major），即一列一列存储。

# `mdspan`（选讲）

- 因此 layout 用来规定你如何“看待”内存。
- 总之，所谓多维只是把映射写出来：\((i_1,\dots,i_n) \to index\)，再用 `index` 访问内存。
- 你也可以用 `std::layout_left` 使用 Fortran 顺序，或用 `std::layout_stride` 使用特殊步长。
- 有时你需要更精细的控制，例如滑动窗口（此时映射可能不是单射，即两个多维下标可能映射到同一索引）、分块数组、三角矩阵等。
- 这时你可以自定义自己的 layout 策略！
- 你需要定义模板结构体 `LayoutPolicy::mapping<Extent>`，在其中规定映射的许多性质。
- Uniqueness：是否单射（injective）。
- Exhaustion：是否满射（surjective）。
- Stride：某一维是否有固定步长。
- 方法是 `is_always_xxx`（例如 `is_always_unique`、`is_always_exhaustive`、`is_always_strided`）与 `is_xxx`。

`[0, 0]` `[0, 1]` `[1, 0]` `[1, 1]` Tile0 Tile1 `[0, 2]` `[0, 3]` `[1, 2]` `[1, 3]`

# `mdspan`（选讲）

- 例如对分块（tiled）数组：
- 注意：分块数组有多种访问方式，这里只是其中一种。
- 它总是单射，因此 `is_always_unique` 与 `is_unique` 应返回 `true`；
- 它可能不满射，因此 `is_always_exhaustive` 应返回 `false`；但当分块恰好切分数组时一定满射，因此 `is_exhaustive` 应返回 `(extents[0] % tile[0] == 0) && ...`；
- 它可能不是 strided：若分块在同一行，则步长固定；若分块换行，则步长不同。
- 你可以通过观察 `[0,0]`、`[1,0]`、… 之间的步长（行方向）以及 `[0,0]`、`[0,1]`、… 之间的步长（列方向）等来判断 stride。
- 因此 `is_always_strided` 应返回 `false`，并且由于分块 stride 基本无法固定，`is_strided` 也应返回 `false`。
- 这些性质用于优化，因此即便某些角落情形能返回 `true`，你也可以返回 `false`。
- 此外，你需要在 `mapping` 中加入其它函数与类型。暂时没有官方示例，你可以参考这里的示例。

# `mdspan`（选讲）

- 最后一个组成部分是 accessor，默认是 `std::default_accessor`。
- 也就是说，当你得到索引后，如何访问内存？
- 默认就是返回 `mem[i]`；但例如你可能想加锁……
- 因此你需要定义 `access(pointer, index)` 返回元素引用，以及 `offset(pointer, index)` 返回指向元素的指针。
- 这很少用，默认的就够。
- 你可能觉得太复杂！
- 但总之，最常用的就是 `std::mdspan<T, std::dextent<IndexType, DimNum>>`。

# 容器

- 顺序容器
- `array`
- `vector`
- `bitset`
- `span`
- `deque`
- `list`
- `forward_list`

# `deque`

- 双端队列（Double-Ended Queue）
- `deque` 最核心的要求是：
- 在前端或后端插入与删除元素均为 𝑂(1)。
- 随机访问。
- 其它性质与 `vector` 类似，例如中间插入/删除为 𝑂(𝑛)。
- 方法上，除了额外提供 `push_front`、`emplace_front`、`pop_front`（以及自 C++23 起的 `prepend_range`）之外，其余（包括构造函数）都与普通 `vector` 相同，因此不再重复。
- 重要的是它如何实现。

# `deque`

- 在介绍 `deque` 的实现之前，我们需要一种新数据结构：循环队列（circular queue）。
- 队列是 FIFO。
- 但实践中空间通常有限，不能无限 `push`……
- 循环队列分配固定大小缓冲区，记录 `head` 与 `tail`。
- `enqueue` 时 `tail` 前移。
- `dequeue` 时 `head` 前移。
- 若 `tail == head` 即队列满，则覆盖 `head` 处元素，并让 `tail` 与 `head` 都前移。
- 这被广泛使用，例如硬件里的预取预测：若预测太多，最旧的会被丢弃。

# 循环队列

`head` `tail` `head` `tail` `head` `tail` `enqueue(3)` `dequeue` `1 2` `1 2` `3` `2 3`

`head` `head` `tail` `head` `(tail)` `(tail)` `enqueue(6)` `dequeue` `5` `2` `3` `4` `5` `6` `3` `4` `5` `6` `4`

# `deque`

- 那么，我们能用循环队列模拟 `deque` 吗？
- 我们已经说过如何从尾部 `enqueue`；你也很容易知道如何从头部 `enqueue`。
- 因此显然插入与删除是 𝑂(1)！
- 然而 `deque` 在满时不应丢弃元素。
- 因此我们可以用动态循环队列：满时扩容并用新空间构成新的循环队列。
- 与 `vector` 类似，需要指数式扩容空间。
- 当你连续 `enqueue` 时，摊还复杂度为 𝑂(1)；`dequeue` 显然 𝑂(1)。
- 你也可以随机访问，例如 `deque[i]` 就是 `vec[(head + i) % size]`。

# `deque`

- 但 `deque` 期望的是真正的 𝑂(1)，而不是摊还 𝑂(1)……
- 这看起来几乎不可能！
- `deque` 实现的做法是：“对昂贵对象拷贝而言，复杂度近似 𝑂(1)”。
- 若只用动态循环队列，扩容时需要拷贝所有元素；那无法满足要求。
- 解决办法是降低拷贝成本……但这怎么可能？
- 典型实现是使用动态循环队列（称为 `map`），其元素是指针。
- 每个指针指向一个块（block），块里存放许多对象。
- 块大小固定，例如在 libc++ 中是 `max(16*sizeof(obj), 4096)`；在 libstdc++ 中是 `8*sizeof(obj)`。
- 你可以把它想成“整体上”的一个大循环队列！

# `deque`

`tail` `head` `null` `map` `ptr`

`blocks` `1 2 3 4 5 6 7 8 9`

整体上作为循环队列

`tail` `head` `null` `map` `ptr`

# `deque`

全局虚拟节点，尚未分配。
- `deque` 需要记录/知道的是：全局偏移为 6
- `map` 及其尺寸。
- 块大小。
- 第一个元素的全局偏移 `off`。`1 2`
- 元素个数。
- 我们可以用 `off / block_size` 知道 `head` 的位置。
- 扩容时只需拷贝所有指针！
- 指针个数是 𝑛 / 𝑘，拷贝它们非常便宜……
- 若对象拷贝昂贵，这一成本可近似看作 𝑂(1)。
- 即便在摊还复杂度语境下，也是 𝑂(1) + 𝑂(指针拷贝成本)/𝑘 而不是 𝑂(1) + 𝑂(对象拷贝成本)，仍然更便宜。
- 下面展示不同情形……

# `deque`

- 原始：

`tail` `head` `null` `Map size is 4` `map` `ptr`

`1 2 3 4 5 6 7 8 9`

全局偏移为 6，总大小为 9

# `deque`

- 情形 1：`push_front`

`tail` `head` `null` `Map size is 4` `map` `ptr`

`0 1 2 3 4 5 6 7 8 9`

全局偏移为 5，总大小为 10

# `deque`

- 情形 2：`push_back`

`tail` `head` `null` `Map size is 4` `map` `ptr`

`0 1 2 3 4 5 6 7 8 9 10`

全局偏移为 5，总大小为 11

# `deque`

- 情形 3：`push_back`，对 `map` 做 `enqueue_back`。`tail` `head`

`map` `Map size is 4`

`0 1 2 3 4 5 6 7 8 9 10`

全局偏移为 5，总大小为 12

# `deque`

- 情形 4：`pop_back`，也从 `map` 的后端 `dequeue`。

`tail` `head` `null` `Map size is 4` `map` `ptr`

`0 1 2 3 4 5 6 7 8 9 10`

全局偏移为 5，总大小为 11

全局偏移为 3

# `deque`

`-2`
- 情形 5：`push_front`，对 `map` 做 `enqueue_front`。`tail` `head`

`map` `Map size is 4`

`-1 0 1 2 3 4 5 6 7 8 9 10`

总大小为 13

# `deque`

- 普通的 `pop_front`/`pop_back`（即不对 `map` 做 `dequeue`）很简单，你可以自己想。
- 你也可以在 `map` 出队时模拟 `pop_front`。
- 注意：并不强制释放块的内存（例如 MS 实现中）。
- 我们只移动 `tail` 与 `head`。
- 这有点像“懒加载”：资源在需要时才分配，但未必会释放。
- 当你发现 `map` 的某个元素不是 `nullptr`，可以直接使用它指向的块。
- 我们的策略保证有效数据不会被覆盖。
- 你也可以像 `vector` 一样使用 `shrink_to_fit` 来释放那些未使用的块。
- 它也可能收缩 `map`，就像 `vector` 一样。

# `deque`

全局偏移为 3

- 若现在需要 `push_back` 呢？`-2`
- `tail == head`，`map` 满了！`tail` `head`

`map` `Map size is 4`

`-1 0 1 2 3 4 5 6 7 8 9 10`

总大小为 13

# `deque` 中的 `map` 重分配

- 当 `map` 满时需要重分配……
- 我们假设新增加的块数为 `count`。
- 现在只需让循环队列在新 `vector` 中仍然连续。
- 步骤：
- 首先，把 `vec[head, vecEnd)` 的所有元素拷贝到 `newVec[head, currEnd)`；
- 然后，若 `head <= count`，把 `[0, head)` 拷贝到 `[currEnd, …)`。
- 否则，尽可能拷贝到 `currEnd` 之后，剩余部分安排到 `newVec` 开头。
- 最后，把其余位置设为 `nullptr`。
- 这有点抽象，我用图示说明……

# `deque` 中的 `map` 重分配

- 分配新空间……

Old Map

`head` `(tail)` `count` `count`

New Map

# `deque` 中的 `map` 重分配

- 从旧 `head` 拷贝到新 `head`……

Old Map

`head` `(tail)`

New Map

New head

# `deque` 中的 `map` 重分配

- 情形 1：`head <= count`

Old Map

`head` `(tail)`

New Map

New head New tail

# `deque` 中的 `map` 重分配

- 情形 2：`head > count`

Old Map

`head` `(tail)`

New Map Too small to fill in all…

New head

# `deque` 中的 `map` 重分配

- 情形 2：`head > count`

Old Map

`head`

New Map

New New head tail

# `deque` 中的 `map` 重分配

- 我们看代码！
- 找新尺寸……

# `deque` 中的 `map` 重分配

- 把旧的拷贝到新的……

# `deque` 中的 `map` 重分配

- 销毁旧 `map` 并把成员改为新 `map`！

# `deque`

- 插入与删除都是 𝑂(𝑛)：
- 实现也与 `vector` 类似。
- 插入通过 `push` 与旋转（rotate）。
- 删除通过移动与 `pop`。
- 特别地，由于 `deque` 两端都能 `push`，会选择更近的一端做 `push`/`pop`。因此严格说复杂度是 𝑂(更近一侧距离)。
- 迭代器就是带偏移的 `deque` 指针。
- `*` 是 `deque->map_[offset / block_size][offset % block_size]`。
- `+`/`-`/`++`/`--` 只是操作偏移（到达整体末尾时要绕回 0）。也可能检查是否越过 `tail`/`head` 来判断迭代器有效性。

# `deque` 的迭代器失效

- 从 `vector` 视角看，插入只会让插入点“之后”的元素失效。
- 但那是因为 `vector` 总是用 `push_back` 插入；`deque` 可能用 `push_front` 来降低复杂度。你不能假设失效一定发生在插入点之前或之后。
- 此外，`map` 可能扩容，因此即便只做 `push_back`/`push_front`，原偏移也不保证仍正确。
- 例如扩容前 `tail < head`，扩容后 `tail` 被拷贝到 `head` 之后，从而 `tail > head`。
- 因此插入后所有迭代器都视为失效。
- 这也包括 `size` 增长的 `resize`；也包括 `shrink_to_fit` 与 `clear`，因为它们可能改变 `map` 尺寸（例如 MS 中 `clear` 会同时丢弃元素与 `map`）……
- 仅从前后端删除只会使被删元素失效；否则所有迭代器也失效。
- 这也包括 `size` 缩小的 `resize`。

# 最后说明

- 从前后端操作（包括例如 `insert(end)`）时，元素引用不失效，因为块本身不变。唯一变化的是 `map`。
- 当然，被删除元素的引用会失效；这是必要且显然的。
- `vector` 无法保持引用，因为缓冲区本身已经改变。

# 容器

- 顺序容器
- `array`
- `vector`
- `bitset`
- `span`
- `deque`
- `list`
- `forward_list`

# `list`

- 双向链表
- 尽管 `list` 是重要数据结构，我个人几乎从不使用它。
- `forward_list` 更少见。我以前从未用过。
- `list` 具有这些性质：
- 插入与删除 𝑂(1)。
- 拼接（`splice`）𝑂(1)。
- 无随机访问。
- 实现与我们学过的类似，因此这里主要讲 API。

# `list`

- 我们知道双向链表由节点组成。
- 每个节点包含 `T data`、指向前一节点的 `prev` 与指向下一节点的 `next`。
- 特别地，第一个节点的 `prev` 是 `nullptr`，最后一个节点的 `next` 是 `nullptr`。
- 若你写过双向链表，会发现边界情况非常烦人……
- 因此在 MS 实现中，`list` 用循环链表实现。
- 也就是说，我们引入哨兵节点（sentinel），它是第一个节点的 `prev` 与最后一个节点的 `next`。
- 这会统一边界情况并极大降低代码难度，因为它不是 `nullptr`，而是可操作的虚拟节点。
- 因此总体上 `list` 保存哨兵节点（或其指针）与 `size`（为了让 `size()` 为 𝑂(1)，尽管你也可以 𝑂(𝑛) 去数）。其它节点动态分配并链接起来。

`head`

# `list`：`sentinel`

- 例如：`tail`

哨兵版本（这里只有 `int`，因此用哨兵的 `val` 存 `size`）

https://leetcode.cn/problems/design-linked-list/description/

无哨兵版本

# `list`

- 迭代器只是节点的包装。
- `--`/`++` 走向 `prev`/`next`。
- `end()` 是哨兵节点。
- 因此很容易理解 `list` 的迭代器失效！
- 只有删除，并且只使被删节点失效。
- 但它仍线程不安全，例如同时删除两个相邻节点。

# `list`

- 成员访问：
- `front()`/`back()`：取得首尾元素。
- 双向迭代器，如前所述。
- 尺寸相关：
- `.size()`：取得尺寸，返回 `size_t`。
- `.empty()`：返回 `bool`，表示 `size == 0` 是否成立。
- `.max_size()`：取得本系统上可能的最大尺寸（通常没什么用）。
- `.resize(n, obj=Object{})`：使 `size = n`；
- 若原 `size` 已是 `n`，什么都不做。
- 若大于 `n`，`[n, end)` 的元素会被移除。
- 若小于 `n`，会在尾部插入新元素，值均为 `obj`。
- `.clear()`：移除所有元素；之后 `size` 为 0。

# `list`

- 这里与 `deque` 相同，以下都是 𝑂(1)：
- `.push_back(obj)`：尾部插入。
- `.emplace_back(params)`：尾部就地构造。
- 自 C++17 起返回插入元素引用（之前返回 `void`）。
- `.pop_back()`：尾部删除。
- `.push_front(obj)`
- `.emplace_front(params)`
- `.pop_front()`。
- 自 C++23 起，𝑂(𝑙𝑒𝑛(𝑟𝑎𝑛𝑔𝑒))
- `.assign_range(Range)`
- `.append_range(Range)`
- `.prepend_range(Range)`
- `.insert_range(const_iterator pos, Range)`

# `list`

- 与 `vector` 相同：
- `.insert(const_iterator pos, xxx)`：`xxx` 与构造函数参数类似：
- `(value)`：插入单个元素。
- `(size_t count, value)`：插入 `count` 个副本。
- `(InputIt first, InputIt last)`：插入 `[first, last)`。
- `(std::initializer_list<T>)`：插入初始化列表。
- `.erase(const_iterator pos)` / `.erase(const_iterator first, const_iterator last)`：删除单个 / 删除 `[first, last)`。`first`、`last` 必须是本容器的迭代器。
- 插入返回指向第一个插入元素的迭代器；删除返回指向被删元素下一个元素的迭代器。

# `list`

- 构造函数：
- 默认构造。
- 拷贝构造与移动构造。
- `(size_t count, const T& elem = T{})`：构造包含 `count` 个 `elem` 副本的 `list`。
- `(InputIt first, InputIt last)`：把 `[first, last)` 拷贝进 `list`。
- `(std::initializer_list<T>)`：把初始化列表拷贝进 `list`。
- 所有构造函数都可以带可选分配器参数。
- 与另一个 `list` 交互：
- `.assign`：也与构造函数类似
- `(count, const T& value)`
- `(InputIt first, InputIt last)`
- `(std::initializer_list<T>)`。
- `.swap(list)`：与另一个 `list` 交换，等价于 `std::swap(list1, list2)`。
- `operator<=>`。

# `list`

- 现在终于介绍一些 `list` 独有 API……
- `.remove(val)` / `.remove_if(func)`：删除所有值为 `val` 或使 `func` 返回 `true` 的元素。
- `.unique()` / `.unique(func)`：删除相邻且相等（由 `==`/`func` 判断）的元素；例如 `0 1 1 2` 可变为 `0 1 2`（重复的一侧会被移除）。

- 自 C++20 起它们返回被删除元素个数。
- `.reverse()`：反转整个链表。
- `<algorithm>` 里也有这些方法，但它们不会从链表中删除节点，效率更低。
- 原因下一讲再说……

# `list`

- `.sort` / `.sort(cmp)`：稳定排序。
- MS 实现是归并排序，空间复杂度 𝑂(1)，时间复杂度 𝑂(𝑛 log𝑛)。
- `<algorithm>` 的 `sort` 需要随机访问迭代器，且通常不是归并排序，因为对 `vector` 而言归并排序空间复杂度不好（𝑂(𝑛)）。

# `list`

- 有两种方法可以把节点从另一个 `list` 移过来：
- 也就是说，另一个 `list` 不再拥有这些节点。
- `.insert(pos, it1, it2)` 不会改变节点所有权；它只是拷贝！
- `.merge(list2)` / `.merge(list2, cmp)`：与归并排序中的 `merge` 步骤相同，通常用于已排序链表。
- 两个有序链表会合并成一个有序链表。
- `.splice(pos, list2, …)`：
- `()`：把整个 `list2` 插入到 `pos`。
- `(it2)`：把 `it2` 插入到 `pos`（并从 `list2` 移除）。`it2` 必须来自 `list2`。
- `(first, last)`：把 `[first, last)` 插入到 `pos`（并从 `list2` 移除）。`first`、`last` 必须来自 `list2`。
- 你可能会注意到：仅从链表中摘掉一些节点并移到另一链表，其实不需要 `list` 本身！
- 那为什么后两种还要提供 `list2`？
- 因为 `list2` 要维护 `size`！

# `list`

# 容器

- 顺序容器
- `array`
- `vector`
- `bitset`
- `span`
- `deque`
- `list`
- `forward_list`

# `forward_list`

- 单向链表。
- `forward_list` 的目的是节省空间，因此它不额外记录 `size`，也不提供 `.size()`。
- 若你确实需要，可以用 `std::(ranges::)distance(l.begin(), l.end())`，但记住它是 𝑂(𝑛)。
- 实现方式与我们学过的一样：节点包含 `T data` 与指向下一个节点的 `next`。
- 它没有哨兵；最后一个节点的 `next` 是 `nullptr`。
- API 几乎与 `list` 相同。
- 但由于 `forward_list` 没有 `prev`，无法回到前一个节点（前向迭代器），因此许多 API 改成 `xxx_after`！
- 迭代器失效规则相同。

# `forward_list`

- 区别：
- `forward_list` 没有 `.back()` / `.pop_back()` / `.push_back()` / `.append_range()`。
- `.insert_after()` / `.erase_after()` / `.emplace_after()` / `.insert_range_after()` / `.splice_after()`：位置参数接受“插入位置之前”的迭代器，这样你可以在它之后插入。
- 特别地，`.splice_after()` 中来自另一个链表的迭代器区间也是 `(first, last)` 而不是 `[first, last)`！
- 但 `.insert_after()` 仍是 `[first, last)`，因为它不需要往回走，且可来自任意容器。
- 你无法提供“插入位置本身”而不是“其前一个”，因为无法向后走……
- 因此 `forward_list` 提供 `.before_begin()` / `.cbefore_begin()`，以便在头部插入。
- 最后，由于 `forward_list` 不单独记录 `size`，`splice_after` 里的 `list2` 事实上是冗余的……
- 它可能在调试模式下用于检查节点是否属于 `list2`。

# 容器，ranges 与算法

容器适配器

# 容器适配器

- 容器适配器是对已有容器的包装；它把接口变换成另一种数据结构的接口。
- 它们通常不提供迭代器。
- 因此模板参数总是 `<T, Container, …>`。
- 自 C++23 起提供 `flat_set`/`flat_map`/`flat_multiset`/`flat_multimap`，但它们与关联容器很相似，因此我们在下一节之后再讲。

# 容器

- 容器适配器
- `stack`
- `queue`
- `priority_queue`
- `flat_set`/`flat_map`/`flat_multiset`/`flat_multimap`

# `stack`

- `stack` 是 LIFO 数据结构。
- 就这些！
- 所提供的容器需要具备 `push_back`、`emplace_back`、`pop_back`，因此 `vector`、`deque`、`list` 都可以。
- 默认是 `deque`，但我个人认为 `vector` 更好。
- 如前所述，`vector` 的缓存效率更好；但当数据非常大时 `deque` 可能更占优，因为其重分配效率更好（总之，若很重要就用 profiler）。
- 因此使用 `vector` 时你可能希望 `reserve` 容量，但用 `stack` 时不太容易做到。
- 含义会在《移动语义》中解释……
- 构造函数方面，除了默认构造与拷贝构造外，只能从一个容器构造。
- 自 C++23 起，可以从 `[first, last)` 构造。
- 你也可以在最后可选地传入分配器。

# `stack`

- `stack` 提供的 API：
- `.pop()`：从后端弹出，返回 `void`；
- `.push(val)` / `.emplace(params)`：压入后端。
- 自 C++17 起，`emplace` 返回插入元素引用。
- `.push_range(Range)`：自 C++23。
- `.top()`：后端元素。
- 以上是栈的基本功能；其它还有：
- `.empty()` / `.size()`
- `.swap(s2)`
- `operator=`
- `operator<=>`
- 这些也应由底层容器提供。
- 就这些，很简单，对吧？

# 容器

- 容器适配器
- `stack`
- `queue`
- `priority_queue`
- `flat_set`/`flat_map`/`flat_multiset`/`flat_multimap`

# `queue`

- `queue` 是 FIFO 数据结构。
- 也就这些。
- 所提供的容器需要具备 `push_back`、`emplace_back`、`pop_front`，因此 `deque`、`list` 都可以。
- 默认是 `deque`。
- 若你想用 `list`，显然 `forward_list` 更好；但它不提供 `size()` 与 `push_back()`，因此你可能需要自己写一个小包装。
- 注意：若你在 `queue` 中不使用 `size()`，可以选择不提供 `size()`；这受益于选择性实例化（会在《模板》中讲）。
- 所有成员函数与 `stack` 相同，除了：
- 你应使用 `.front()` 查看下一个将被弹出的对象（最旧数据），而不是 `.top()`。
- 也提供 `.back()` 查看最新插入元素。

# 容器

- 容器适配器
- `stack`
- `queue`
- `priority_queue`
- `flat_set`/`flat_map`/`flat_multiset`/`flat_multimap`

# `priority_queue`

- 它定义在 `<queue>` 里！
- 它其实是最大堆（max heap）；也就是说，在插入新元素/弹出当前最大元素之后，仍能在 𝑂(log 𝑛) 内提供当前最大元素。
- 建堆只需 𝑂(𝑛)，比排序更便宜。
- 它只需要容器支持随机访问，因此默认 `vector` 与 `deque` 都可以。
- 由于需要比较，你可以提供模板参数作为比较方法。
- 例如若你想要最小堆，可以用 `std::priority_queue<T, std::vector<T>, std::greater<T>>`（是的，最小堆需要 `greater`！）。
- `priority_queue` 的构造函数有个怪事：比较器是第一个参数，因此若你想传入已 `reserve` 的 `vector`，就得写成例如 `std::priority_queue q{ std::less<int>{}, vec }`。
- CTAD 会帮你填全模板参数。

# `priority_queue`

- 然后，自 C++11 起可以提供输入迭代器对（而不是像 `stack`/`queue` 那样等到 C++23）。
- 即 `(first, last, cmp = CMP{}, container = Container{})`。
- 我们快速复习堆相关算法。
- 它其实是二叉树，并满足子节点不大于父节点（在数组里即 `arr[i] <= arr[(i-1)/2]`）。
- 渗透（percolation）是核心算法。
- 当新元素插入数组末尾时，需要向上渗透以维持堆性质。
- 也就是与父节点交换，直到不再大于父节点；显然 𝑂(log 𝑛)。
- 当最大元素从数组顶端移除时，需要向下渗透。
- 也就是用最后一个元素填到顶端，再与较大的子节点交换，直到大于两个子节点。
- 建堆用 Floyd 算法：从最后一个非叶节点到根依次向下渗透。
- 越高的节点渗透复杂度越高，但节点数越少。𝑛
- 因此总复杂度为 ∑𝑂(log 𝑛 - log 𝑖) = 𝑂(∑log …)；由 Stirling 近似可知为 𝑂(𝑛)。

# 有问题吗？

- 你可以在这里稍作休息 ☺。

# 容器，ranges 与算法

关联容器

# 关联容器

- 它们叫“关联容器”，是因为把键（key）与值（value）关联起来。
- 值也可以省略，从而只检查键是否存在。
- 分为有序与无序两类。
- 有序容器需要“小于”比较函数。
- 遍历有序容器也会按“升序”得到元素。
- 类似地，你也可以指定比较函数。
- 它是 BBST（平衡二叉搜索树），查找、插入、删除均为 𝑂(log 𝑛)。
- 通常是红黑树，尽管例如 AVL 树也能满足要求。
- 红黑树在拓扑删除时通常比 AVL 树更少重排。
- 无序容器需要哈希函数与“相等”比较函数。
- 它是（开放寻址式）哈希表，查找、插入、删除期望为 𝑂(1)，最坏为 𝑂(𝑛)（若所有键哈希到同一位置）。
- 它们都是基于节点的容器，即每个元素单独存放在节点中（类似链表）。
- 因此可以提取节点并插入，以降低在容器间移动数据的复杂度。

# 容器

- 有序容器
- `map`
- `set`
- `multimap`
- `multiset`
- 无序容器
- `unordered_map`
- `unordered_set`
- `unordered_multimap`
- `unordered_multiset`

# `map`

- 这里不复习红黑树，也不深入分析 MS 实现，因为它略复杂且你们之前学过理论。
- `map` 是把键映射到值的容器。
- 键唯一；一个键不能映射到多个值。
- `std::map<Key, Value, CMPForKey = std::less<Key>>`。
- 你应实现 `operator<`（或 `<=>`）或提供比较函数。
- `CMPForKey` 应能接受 `const Key`；
- 若是成员函数，应为 `const`。
- 例如你可以定义 `auto operator<=>(const Key&) const`。

# `map`

- 成员访问：
- `operator[]` / `at()`：按键访问；`at()` 会检查键是否存在；若不存在抛出 `std::out_of_range`。
- 双向迭代器，如前所述。
- 注意：`++`/`--` 最坏可能是 𝑂(log 𝑁)，因为你可能从左子树最右节点走到右子树。
- 你可以想象一棵 BBST：`++`/`--` 就是中序遍历意义下的下一个/上一个节点。
- 因此 `.begin()` 是最左节点，`.rbegin()` 是最右节点。
- 每个节点有颜色、父节点、左子、右子；为降低复杂度，空节点也会实例化，并用 `isnil` 成员判断是否为虚拟节点。
- 为防止低效，实现会保存虚拟“头（header）”节点：其父为根，左子为最左节点，右子为最右节点，且 `isnil=true`。
- 它也充当根的父节点与 `end()`，从而像链表哨兵一样统一代码。
- 红黑树成员主要就是 `header` 与树节点数（即 `.size()`）。
- `++` 就是找比自身更大的下一个元素。
- 因此你可能沿父链向上，直到不再是父的右孩子。
- 然而从 `begin` 迭代到 `end` 是 𝑂(𝑁)，因此 `++`/`--` 平均仍是 𝑂(1)。

# `map`

`𝑂(log 𝑁)` 路径 否，返回父节点。

是父的右孩子吗？

在这里 `++`

# `map`

- 注 1：`operator[]` 在键不存在时会插入默认构造的值，因此：
- 若键不存在且你不需要默认值，`insert_xxx`（稍后讲）更高效（构造+赋值 vs 仅构造）。
- 它可能插入新值，因此不是 `const` 成员函数，不能在 `const map` 上使用。
- 若值类型无法默认构造（类没有默认构造函数），也不能用它。
- 注 2：键值对存在红黑树里，因此迭代器也指向 `pair`。
- 注 3：自 C++17 起可用结构化绑定（structured binding）简化遍历。

# 结构化绑定

- 如你所见，结构化绑定就是 `auto& […]{xx}`。
- `{xx}` 可以是 `(xx)` 或 `=xx`。
- `auto&` 可以换成任何带 `auto` 的东西。
- 例如 `auto` 表示按值拷贝；`const auto&` 也合法；等等。
- 我们已经看到 `xx` 可以是 `pair`；它也可以是：
- 所有数据成员均为 public 的对象，会绑定到这些成员上。
- C 风格数组或 `std::array`，会绑定到 `arr[i]`。
- 类 tuple 的东西，会绑定到每个元素。
- `pair` 就是一种类 tuple；那什么是 `tuple`？
- 它是 `pair` 的推广；可以有不止两个成员。
- 例如 `std::tuple<int, float, double> t{1, 2.0f, 3.0};`

# `tuple`

`std::tuple<int, float, double> t{1, 2.0f, 3.0};`
- 它只能用编译期可确定的索引访问（不像 Python 的 tuple）！
- 因此你可以用 `std::get<0>(tuple)` 取得 `int`。
- 自 C++14 起，当类型在 tuple 中只出现一次时，也可以用基于类型的索引，例如 `std::get<int>(tuple)`。
- 你可以使用 `operator=`、`swap` 与 `operator<=>`，就像 `pair`。
- 你也可以用 `std::tuple_size<YourTupleType>::value` 取得尺寸，用 `std::tuple_element<index, YourTupleType>::type` 取得第 `index` 个元素类型，再用 `std::tuple_size<YourTupleType>::value` 取得 tuple 尺寸。
- tuple 的类型可以用例如 `decltype(t)` 得到，以后会进一步讨论！
- 你可以用 `std::tuple_cat` 连接两个 tuple 得到新 tuple。
- 注意：tuple 在日常编程中不鼓励使用，因为元素没有名字，维护时容易晦涩。它常用于元编程，会在《模板》中讲。

# 结构化绑定

- 注 1：`pair` 与 `std::array` 也某种程度上是类 tuple，可以使用一些 tuple 方法，例如 `std::get`。
- 我们以后会给出“类 tuple”的精确定义。
- 注 2：结构化绑定是声明，因此不能绑定到已有变量。
- 反过来，对 `tuple` 与 `pair`，你可以用 `std::tie(name, score) = pair` 赋值。
- 我个人喜欢用 `_` 作为变量名表示“除了构造它之外永远不用”。
- 例如 `auto& [_, score] : scoreTable`。
- 自 C++26 起，`_` 被保留为“丢弃值”；你可以在函数作用域内重复声明 `_` 而不报重定义错误（但重声明后不能再使用它；它是丢弃的）。
- 对 `std::tie`，你可以用 `std::ignore`，例如 `std::tie(std::ignore, score) = pair`。

# 结构化绑定

- 注 3：结构化绑定等价于使用匿名结构体，并对其成员起别名。
- 例如 `b` 的类型是 `const int` 而不是 `&`，因为这是 `const auto&` 的匿名对象，且 `anonymous.b` 不是引用（这里是 `std::tuple<int,float>&` 而不是 `std::tuple<int&,float&>`）。
- 但 `anonymous.b` 仍引用原始数据，因为匿名对象本身是引用。
- 注 4：结构化绑定通常比新手/粗心程序员写的代码更高效。
- 例如你可以这样写：
- 然而 `map` 存的是 `std::pair<const std::string, int>`，若用 `std::pair<std::string, int>` 迭代会导致不必要拷贝。
- 当然 `const auto& p` 也能消除这个问题。

提示：只有在你能保证成员顺序稳定时，才应使用结构化绑定。否则若你在定义里交换了两个同类型成员，用户会得到完全错误的结果！

# `tuple`

- 最后：当你无法默认 `operator<=>` 但仍想做某种字典序比较时，可以利用 `tuple` 的 `operator<=>`！
- 例如：

# `map`

- 尺寸相关：
- `.size()`：返回 `size_t`。
- `.empty()`：返回 `bool`。
- `.max_size()`：通常没什么用。
- `.clear()`：清空；`size` 变为 0。
- 查找：
- `.find(key)`：返回指向键值对的迭代器；若不存在返回 `end()`。
- 记住 `if (auto it = map.find(key); it != map.end()) { … }`！
- `.contains(key)`：自 C++20，返回 `bool` 表示键是否存在。
- `.count(key)`：返回键对应元素个数；在 `map` 中只能是 0 或 1。

# `map`

- `.lower_bound(key)`：找到 `it` 使得 `prev(it)->key < key <= it->key`。
- 顾名思义，用 `key` 作为下界，使 `[it, end)` 中元素都 `>= key`。
- 若 `key` 最大则返回 `end()`。
- `.upper_bound(key)`：找到 `it` 使得 `prev(it)->key <= key < it->key`。
- 顾名思义，用 `key` 作为上界，使 `[begin, it)` 中元素都 `<= key`。
- 若 `key` 最小则返回 `end()`。
- `.equal_range(key)`：找到与 `key` 相同的区间。
- 它等价于 `[lower_bound(key), upper_bound(key))`，但更高效。
- 特别地，若 `key` 存在，返回 `[it, it+1)`；否则返回 `[it, it)`。
- `operator=`、`operator<=>`、`swap`、`std::erase_if` 都可用。
- `operator<=>` 会从头到尾逐对比较。

# `map`

- 插入
- 由于 `map` 规定键唯一，插入可能失败。返回 `pair<iterator, bool>`；
- 若成功，迭代器指向插入元素且 `bool` 为 `true`；
- 若失败，迭代器指向同键已有元素且 `bool` 为 `false`。
- 但不同方法对失败的处理不同：
- 保持不变：
- `.insert({key, value})`
- `.emplace(params)`：与 `insert` 相同，只是参数用于构造 `pair`。
- 覆盖（C++17）：
- `.insert_or_assign(key, value)`：返回 `pair<iterator, bool>`；
- 区别：会覆盖；分别提供 `key` 与 `value`，而不是提供整个 `std::pair`。
- 保持不变且甚至不构造待插入值（C++17）：
- `.try_emplace(key, params)`：与 `emplace` 类似，只是参数用于构造 `value`，并且在失败时 `emplace` 仍可能被禁止构造 `pair` 的某些部分。

构造 `Texture` 很昂贵时，`try_emplace` 很合适！

# `map`

- 删除：
- `.erase(…)`
- `(key)`
- 返回被删元素个数，在 `map` 中为 0 或 1。
- `(iterator pos)`：删除 `pos`，要求 `pos` 来自该 `map`。
- `(iterator first, iterator last)`：从 `map` 删除 `[first, last)`。
- 这两种返回最后一个被删迭代器之后的迭代器。
- 你也可以为插入提供提示迭代器 `hint`。
- `hint` 应位于插入元素之后才能获得效率。
- 若在它之前，效率可能受损，因此要小心使用。
- 插入方法相同，除了：
- 第一个参数应是 `hint`。
- `emplace()` 换成 `emplace_hint()`。
- 只返回迭代器，没有 `bool`。

`key_comp()` 取得比较函数。这意味着 `someKey >= pLoc->first`，而 `lower_bound` 意味着 `someKey <= pLoc->first`，因此 `someKey == pLoc->first`，表示该键已存在。

# `map`

- `hint` 常用于下面的惯用法：

{

}
- 对 `else` 分支：键不存在时，`lower_bound()` 会返回“刚好大于 `key`”的位置（即未来插入位置的迭代器），有利于后续插入。
- 注意：它们不返回 `bool`，以便与例如 `vector::insert` 统一。
- 是的，你看到该方法参数与 `vector` 一样！
- 插入迭代器会用到它，因此你也可以把元素插入到 `map` 中。

# `map`

- 注意：`map` 的键是 `const` 以保持有序；若要改键，需要删除原元素再插入新元素。
- 因此你不能直接用 `std::copy(.., .., scoreTable2.begin())`，因为它会赋值整个 `pair` 并违反键的 `const`。

除提取、插入与判断是否为空外，你也可以通过节点句柄（`node_type`）的 `.key()` / `.value()` 取得键或值的引用（从而在重新插入前分别修改键或值）。若句柄最终被销毁且其中节点仍未被插入到 `map`，析构函数会自动释放该节点。若你还想了解其它接口，可自行查阅 cppreference 等资料。

- 最后，由于 `map` 是基于节点的容器，也可以像链表的 `splice` 一样提取节点并插入到另一个 `map`。
- 自 C++17 起：
- `extract(key)` / `extract(iterator pos)`：从 `map` 提取节点。
- 返回 `node_type` 对象，你可以直接写 `auto`。
- 实际是 `std::map<Key, Value, …>::node_type`，名字太长……
- 你可以把它想成指针；若键不存在，则 `ret.empty()` 或 `operator bool` 为 `false`，就像返回 `nullptr`。空节点在插入时不会做任何事情。
- `.insert(node_type&&)`：把节点插入 `map`。
- 我们以后会解释 `&&` 是什么意思；现在只需知道你需要传 `std::move(node)` 或直接把 `xx.extract(yy)` 传给该参数。
- 返回 `insert_return_type`，结构体包含 `{ iterator position, bool inserted, node_type node }`；
- 前两项与普通插入相同；若成功则 `node` 为空，否则保留原节点。
- `std::move(node)` 之后，变量 `node` 失效，你应从这里再取回信息。
- 你也可以把 `hint` 作为第一个参数，返回类型仍只是 `iterator`。
- `.merge(另一个 map/multimap)`：合并另一个 `map` 到自身，即仅当键不存在时才从另一个容器移动过来（已存在键不会被移动）。

# `map`

- 自 C++23 起也可以使用 `insert_range`。
- 构造函数方面，除了默认/拷贝/移动构造外：
- `(cmp)`：指定比较函数，利用 CTAD。
- `(first, last, cmp = Compare())`：用迭代器对构造，迭代器指向键值对。
- `(initializer_list<pair>, cmp = Compare())`。
- 迭代器失效：由于 `map` 基于节点，迭代器就是指向节点的指针，因此只有删除会使被删元素的迭代器失效。
- 这与链表相同。

# 容器

- 有序容器
- `map`
- `set`
- `multimap`
- `multiset`
- 无序容器
- `unordered_map`
- `unordered_set`
- `unordered_multimap`
- `unordered_multiset`

# `set`

- `set` 就是去掉 `value` 的 `map`；也就是说，你只能以 𝑂(log 𝑁) 插入/删除/检查元素是否存在。
- 键仍唯一；这与数学中集合定义一致。
- 因此与 `map` 的唯一区别是没有 `operator[]` 与 `.at()`；迭代器只指向键而不是键值对。
- 就这些！

# 容器

- 有序容器
- `map`
- `set`
- `multimap`
- `multiset`
- 无序容器
- `unordered_map`
- `unordered_set`
- `unordered_multimap`
- `unordered_multiset`

# `multimap`

- `multimap` 取消键唯一性，即一个键可映射到多个值。
- 因此也不能使用 `operator[]` 与 `at()`。
- 这些等价值按插入顺序排列在同一键下。
- `operator<=>` 逐对检查相等，因此即便两个 `multimap` 存的东西相同，只是相等键内部顺序不同，`==` 也可能为 `false`。
- 例如：

# `multimap`

- 此外，插入永远不会失败，因此没有 `insert_or_assign`/`try_emplace`，`insert`/`emplace` 只返回 `iterator`。
- 仍可使用 `hint`。
- 对 `find()`，只返回某个同键元素的随机一个。
- 对 `count()`，返回元素个数（不只是 0/1）。
- 复杂度为 𝑂(log 𝑁 + 𝑀)，其中 `M = count()`。
- 对 `equal_range()`，返回同键迭代器对。
- 这是 `multimap` 中查找元素的常用方法；`it1 == it2` 表示键不存在。
- 最后，`multimap` 与 `map` 的节点可以互换，例如可以把从 `map` 提取的节点插入 `multimap`，反之亦然。
- `multimap` 合并进 `map` 时只会保留相等范围中的第一个元素以维持唯一性。

# 容器

- 有序容器
- `map`
- `set`
- `multimap`
- `multiset`
- 无序容器
- `unordered_map`
- `unordered_set`
- `unordered_multimap`
- `unordered_multiset`

# `multiset`

- 除了只有键没有值之外，与 `multimap` 相同。
- 就这些！
- 你也可以交换 `multiset` 与 `set` 的节点。
- 事实上 `map` 几乎就像 `set<pair>`，`multimap` 几乎就像 `multiset<pair>`（以第一个元素作为比较标准）。

# 容器

- 有序容器
- `map`
- `set`
- `multimap`
- `multiset`
- 无序容器
- `unordered_map`
- `unordered_set`
- `unordered_multimap`
- `unordered_multiset`

# `unordered_map`

- `std::unordered_map<Key, Value, Hash = std::hash<Key>, Equal = std::equal_to<Key>>`
- 许多类型有 `std::hash<Type>`，例如 `std::string`、`float` 等，因此可以直接作为键。
- 与 `map` 类似，`Hash` 与 `Equal` 应类似 `(const Key&) const`。
- 不同键的哈希值可能相同，因此需要 `Equal` 判断到底要哪个键。
- 在开放寻址哈希表中，同哈希值的元素存放在一个“桶（bucket）”里。
- 桶通常实现为单向链表，以便 𝑂(1) 随机删除。
- C++ 只要求前向迭代器，因此这是可行的。
- 桶由数组组成，因此最终下标是 `hash 值 % bucket 数量`（若桶数为 2 的幂可优化为 `&`）

# `unordered_map`

- 一种可能的实现：

Bucket array empty

# `unordered_map`

- 在 MS 实现中略有不同。
- 它用双向链表而不是单向链表。
- 因此可把它看成“双向链表数组”；从 `list` 实现角度看，每个链表需要分配哨兵。
- 然而哈希表通常很稀疏，每个链表都很小；每个链表一个哨兵浪费很多内存。
- 因此 MS 让所有链表“共享”同一个哨兵。
- 更准确地说，所有节点被链接成一个巨大的链表。
- 因此 `begin` 迭代器就是 `sentinel->next`，`end` 是哨兵。
- `operator++` 就是链表迭代器的 `operator++`。
- 插入、删除都在整条链表上操作。
- 有点抽象，我用动画演示！

- 每个桶记录 `begin()` 与 `end()`，其中 `[begin, end]` 包含该桶所有元素。
- 注意：闭区间！

Double linked list

sentinel

Bucket array empty

# `unordered_map`

- 更具体地，假设桶数组大小为 `s0`，则在 MS 实现中会分配大小为 `2 * s0` 的指针数组。

in fact sentin sentin empty begin0 end0 begin2 empty end2 begin3 end3 elel

- 当桶为空时，其 `begin` 与 `end` 都指向哨兵指针。
- `begin` 与 `end` 在实现里分别叫 “bucket high” 与 “bucket low”。

# `unordered_map`

- 当插入元素太多，每个桶里元素也会太多！
- 这会提高按键查找的复杂度。`size / bucket num`
- 称为负载因子（load factor）。
- 因此当负载因子过高时，需要扩大桶数组尺寸，使元素再次分散。
- 增长策略不由标准规定，例如在 ms 中可看 `<xhash>` 里的 `_Desired_grow_bucket_count`。
- 这称为 rehash。
- 尽管元素哈希值不变，取模后的下标会变，因此整条链表需要重排。
- C++ 规定 rehash 会使所有迭代器失效，尽管用链表实现时迭代器本可不失效（但你不能跨平台依赖这一点）。
- 由于基于节点，引用始终有效。
- MS 文档：

# `unordered_map`

- 作为哈希表，它提供许多相关方法：
- `.bucket_count()`：桶数组大小。
- `.max_bucket_count()`：类似 `vector` 的 `max_size()`，通常不太有用。
- `.load_factor()`：`size() / bucket_count()`
- `.max_load_factor()`：当负载因子超过该阈值会触发 rehash。可用 `.max_load_factor(float xx)` 设置。
- 在 MS 中默认是 1，即平均每个桶超过 1 个元素就会 rehash。
- `.rehash(n)`：使 `bucket_count() = max(n, ceil(size()/max_load_factor()))` 并 rehash；特别地，在调整 `max_load_factor()` 后可用 `rehash(0)` 立即 rehash 以满足最低要求。
- `.reserve(n)`：预留桶以至少容纳 `n` 个元素，即在 `size() > n` 之前不应发生 rehash。等价于 `rehash(ceil(n/max_load_factor()))`。
- 这与 `vector::reserve(n)` 类似：在 `size() > n` 之前不应发生扩容。

# `unordered_map`

- C++ 也提供直接访问桶的接口。
- `.bucket(key)`：取得键所在桶索引。
- `.begin` / `.cbegin` / `.end` / `.cend(index)`：取得某索引桶的迭代器范围。
- `.bucket_size(index)`：某桶大小。
- 就是 `std::distance(begin(index), end(index))`，复杂度 𝑂(桶大小)。
- 最后，可用 `hash_function()` 与 `key_eq()` 观察函数对象。

# `unordered_map`

- 作为“`map`”，其方法几乎与 `std::map` 相同，除了：
- 它没有 `lower_bound` 与 `upper_bound`，因为无序。
- 也可以用 `hint`，但要求不同。
- 标准不规定 `hint` 如何影响查找；在 MS 实现中，仅当插入键与 `hint` 的键相同时才有用。
- 显然基本只对 `unordered_multimap` 有用。

`_Hint->_Myval`：取得 `hint` 的键值对。`_Traits::Kfn`：取得键。

`_Traitsobj::operator(a, b)`：

`_Mypair._Myval2._Get_first()` 会取得 `_Keyeq`，即相等函数。

# `unordered_map`

- 你也可以提取节点并插入。
- 比较方面，两个 `unordered_map` 只能比较 `==`/`!=`。
- 特别地，对 `unordered_multimap` 的 `==`/`!=` 只要求每个键上的值集合相同，而不像 `multimap` 那样强制插入顺序。
- 也就是说，只要互为排列（permutation）就算相等，顺序不重要。
- 最坏复杂度因此是 𝑂(𝑁)，平均也是 𝑂(𝑁)，因为判断两个区间是否为排列最坏是 𝑂(𝑁)。

# `unordered_map`

- 当你用自己的类作为键时，需要自定义哈希。
- 你当然可以定义带 `operator()` 的类并返回 `size_t`。
- 另一种常用方式是特化 `std::hash`；模板特化会在《模板》中讲，这里先略窥一斑。

- 我们用 `xor` 组合成员哈希值。
- 这只是常见做法；设计好的哈希函数以降低冲突很难，这里不展开。

# 容器

- 有序容器
- `map`
- `set`
- `multimap`
- `multiset`
- 无序容器
- `unordered_map`
- `unordered_set`
- 无序容器之间的关系与有序容器相同，因此不再重复。
- `unordered_multimap`
- `unordered_multiset`

# 容器

- 容器适配器
- `stack`
- `queue`
- `priority_queue`
- `flat_set`/`flat_map`/`flat_multiset`/`flat_multimap`

# 扁平容器（flat containers）

- `map`/`unordered_map`/… 的唯一缺陷是：它们对缓存极不友好！
- 这是基于节点容器的共性问题，包括链表。
- 当 `sizeof` 数据其实很小，而节点有很多指针时也会浪费大量内存。
- 这被很多人批评（例如 Google），因此他们会写自己的版本。
- 即便理论复杂度可能更高，真实效率仍可能更高，因为局部性好。
- 扁平容器（flat containers）为此而生。
- 功能与 `set`/`map` 相同；
- 但它其实是有序“`vector`”！
- 没有冗余数据，显然更缓存友好。
- 对 `flat map`，就是两个 `vector`。

# 扁平容器（flat containers）

- 因此完整定义是 `std::flat_map<Key, Value, Compare = std::less<Key>, ContainerForKey = std::vector<Key>, ContainerForValue = std::vector<Value>>`
- 你也可以选择 `deque` 作为容器。
- 显然复杂度是：
- 查找 𝑂(log 𝑁)，常数非常小（只是简单二分，远小于红黑树）。
- 插入/删除 𝑂(𝑛)。
- 尽管找插入位置是 𝑂(log 𝑁)，但仍要移动元素，甚至可能扩容，这些都是 𝑂(𝑛)。
- 对 `iterator++`，常数 𝑂(1)。
- 迭代器也是随机访问迭代器！

# 扁平容器（flat containers）

- 但你要更注意迭代器失效。
- 对 `map`/`set`，只有被删元素迭代器失效。
- 对无序容器，尽管 rehash 后迭代器失效，至少引用仍有效。
- 但若用 `vector`，插入/删除本身就会使更多甚至所有迭代器/引用失效。
- 扩容也会导致迭代器与引用失效！
- 此外，你不能存放不可拷贝/不可移动的对象。
- 仍有扩容问题！
- `vector` 的异常保证也比 `map` 更松，会在《错误处理》中讲。
- 我们以后可能再补充细节。

# 扁平容器（flat containers）

- 最后，由于键与值分别存在两个容器里，迭代器不指向完整 `pair`；它只保存索引，并分别提供 `first` 与 `second`。
- 因此解引用会得到 `pair` 的代理对象。

- 呃……我还想讲异构容器（heterogeneous container），但这讲信息已经太多，下一讲再讲。

# 小结

- `span`、`mdspan`（只需掌握最简单情形下的用法即可）
- 迭代器：知道最基本用法
- 迭代器类别、流迭代器、迭代器适配器（基于迭代器/基于容器）
- 容器适配器：`stack`、`queue`、`priority_queue`
- `flat_map`/`flat_set`/`flat_multimap`/`flat_multiset`
- 容器：包括设计目的、部分实现
- 关联容器：`map`/`set`/`multimap`/`multiset` 及其 API
- 结构化绑定、`tuple`
- 迭代器失效与引用失效
- `unordered_map`/`unordered_set`/`unordered_multimap`/`unordered_multiset`：功能与复杂度
- 顺序容器：`array`、`vector`、`deque`、`list`、`forward_list`
- `vector<bool>`、`bitset`

# 下一讲……

- 我们会先讲 C++20 的 ranges。
- 然后更深入理解函数对象！
- 接下来会讲异构容器，它比较容易理解。
- 最后介绍算法。
- 这部分略枯燥，但我们会快速把握其整体设计。
- 我们也会介绍一些算法的实现方式。
