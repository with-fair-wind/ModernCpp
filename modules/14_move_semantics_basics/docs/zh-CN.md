移动语义基础（Move Semantics Basics）

# 现代C++基础

# 现代 C++ 基础（Modern C++ Basics）

### 梁佳明，北京大学本科生

### • 第 1 部分

### • 引言

### • 移动构造与移动赋值

### • 关于 noexcept 与继承的讨论

### • 零法则 / 五法则

### • 被移动后的状态（moved-from states）

### • 与移动语义相关的算法与迭代器

# 值分类与移动语义（Value Category and Move Semantics）

### 引言

# 为何需要移动语义？

### • 让我们从一个例子开始揭开移动语义的面纱！

- 为简化起见，此处忽略 `std::string` 的 SSO，字符数据都分配在堆上。

### • 在幻灯片过程中，你可以观察

### 某些操作是否

### 实际上

### 并不必要。

# 为何需要移动语义？

### • 在函数 `CreateAndInsert()` 中，我们知道在 `push_back` 之前

### 内存布局应当类似：

### StackHeap

size = 0 strVec capacity = 3

s size = 4“data”

# 为何需要移动语义？

### • strVec.push_back(s);

### Heap

### Stack

size = 0 strVec capacity = 3 Copy Constructor! Copy!

s size = 4“data”

# 为何需要移动语义？

### • strVec.push_back(s);

### Heap

size = 4 “data”

### Stack

size = 1 strVec capacity = 3 Copy Constructor! Copy!

s size = 4“data”

# 为何需要移动语义？

### • strVec.push_back(s+s);

### • 我们省略 `operator+` 的中间过程Heap

size = 4 “data”

### Stack

size = 1 strVec capacity = 3

s size = 4“data”

s + s size = 8 “datadata”

# 为何需要移动语义？

### • strVec.push_back(s+s);

### • 执行 push_backHeap

size = 4 “data”

### Stack

size = 8 “datadata”

size = 2 strVec capacity = 3 Copy Constructor! Copy! s size = 4“data”

s + s size = 8 “datadata”

# 为何需要移动语义？

### • strVec.push_back(s+s);

### • 销毁临时量 s+sHeap

size = 4 “data”

### Stack

size = 8 “datadata”

size = 2 strVec capacity = 3

s size = 4“data”

s + s size = 8 “datadata”

Destruct! free!

# 为何需要移动语义？

### • strVec.push_back(s);

### Heap

size = 4 “data”

### Stack

size = 8 “datadata”

size = 3size = 4“data” strVec capacity = 3 Copy Constructor! Copy! s size = 4“data”

# 为何需要移动语义？

### • return strVec;

- NRVO 避免为返回再创建新的临时对象。

### • 我们将在下一讲介绍 NRVO。Heap

size = 4 “data”

### Stack

size = 8 “datadata” return value size = 3size = 4“data” strVec capacity = 3

s size = 4“data”

Destruct! free!

# 为何需要移动语义？

### • v = CreateAndInsert();

### Heap

size = 4 “data”

### Stack

size = 8 “datadata” return value size = 3size = 4“data” strVec capacity = 3

Copy Assignment! Copy Constructor! * 3 Copy * 3

v size = 0 capacity = 0

# 为何需要移动语义？

### • v = CreateAndInsert();

### Heap

size = 4 “data”

### Stack

size = 8 “datadata” Destruct! size = 3size = 4“data” strVec capacity = 3

Destruct * 3 Free * 3

v size = 3size = 4 “data” capacity = 3 size = 8 “datadata” size = 4 “data”

# 为何需要移动语义？

### • 总计：我们调用了拷贝构造函数 6 次、析构函数 5 次，

### 因而发生拷贝 × 6、释放 × 5。

### • 关键观察

- 有些资源被拷贝之后随即又被释放！
- 例如 `strVec.push_back(s+s);`
- 我们可以直接「移动」它们，从而既避免拷贝又避免释放！

# 为何需要移动语义？

### • 那么，试试带

### 移动语义的 C++11。

### • 我们首先让代码更多地利用

### 移动语义：

代码里唯一的改动！

# 为何需要移动语义？

### • 从头开始：

### StackHeap

size = 0 strVec capacity = 3

s size = 4“data”

# 为何需要移动语义？

### • strVec.push_back(s);

### Heap

size = 4 “data”

### Stack

size = 1 strVec capacity = 3 Copy Constructor! Copy!

s size = 4“data”

# 为何需要移动语义？

### • strVec.push_back(s+s);

### • 我们省略 `operator+` 的中间过程Heap

size = 4 “data”

### Stack

size = 1 strVec capacity = 3

s size = 4“data”

s + s size = 8 “datadata”

# 为何需要移动语义？

### • strVec.push_back(s+s);

### • 执行 push_backHeap

size = 4 “data”

### Stack

size = 1 strVec capacity = 3 Move Constructor! Move! s size = 4“data”

s + s size = 8 “datadata”

# 为何需要移动语义？

### • strVec.push_back(s+s);

### • 执行 push_backHeap

size = 4 “data”

### Stacksize = 8“datadata”

.

size = 2 strVecNO COPY! capacity = 3

s size = 4“data”

s + s size = 0

# 为何需要移动语义？

### • strVec.push_back(s+s);

### • 销毁临时量 s+sHeap

size = 4 “data”

### Stacksize = 8“datadata”

size = 2 strVec capacity = 3

s size = 4“data”

s + s size = 0

Destruct! NO FREE!

# 为何需要移动语义？

### • strVec.push_back(std::move(s));

### Heap

size = 4 “data”

### Stack

size = 8 “datadata”

size = 2 strVec capacity = 3 Move Constructor! Move! s size = 4“data”

# 为何需要移动语义？

### • strVec.push_back(std::move(s));

### Heap

size = 4 “data”

### Stack

size = 8 “datadata”

. size = 3size = 4“data” strVec capacity = 3

NO COPY!

s size = 0

# 为何需要移动语义？

### • return strVec;

- NRVO 避免为返回再创建新的临时对象。

### Heap

size = 4 “data”

### Stack

size = 8 “datadata” return value size = 3size = 4“data” strVec capacity = 3

s size = 0

Destruct! NO FREE!

# 为何需要移动语义？

### • v = CreateAndInsert();

### Heap

size = 4 “data”

### Stack

size = 8 “datadata” return value size = 3size = 4“data” strVec capacity = 3

Move Assignment!

v size = 0.

# 为何需要移动语义？

### • v = CreateAndInsert();

### Heap

### Stack

size = 0 strVec capacity = 0

v size = 3.size = 4 “data” capacity = 3 size = 8 “datadata” size = 4 “data”

# 为何需要移动语义？

### • v = CreateAndInsert();

### Heap

### Stack

NO FREE! Destruct! size = 0 strVec capacity = 0

v size = 3.size = 4 “data” capacity = 3 size = 8 “datadata” size = 4 “data”

# 为何需要移动语义？

### • 总计：拷贝构造函数只调用 1 次，因而只有 1 次拷贝。

### • 此外，虽然析构函数调用了 3 次，但我们根本没有

### 释放任何堆内存！

### • 6 次拷贝 -> 1 次拷贝

### • 5 次析构 -> 3 次析构

### • 5 次释放 -> 无释放

### • 性能提升巨大！

# 值分类与移动语义（Value Category and Move Semantics）

### 移动构造与移动赋值

# 移动构造与移动赋值

### • 最简单的例子：

# 移动构造与移动赋值

### • 这个例子并不实用，只是展示如何定义它们；

- 移动语义应当把另一份资源「窃取」到自己这里。

### • 不妨定义一个名为 `MyIntVector` 的类。

# 移动构造与移动赋值

### • 移动构造：直接窃取资源！

- 使用 `std::exchange(x, y)`：把 `y` 赋给 `x`，并返回 `x` 原来的值。

### • 移动赋值：窃取资源并释放自身资源。

# 移动构造与移动赋值

### • 仍有一些值得讨论的问题。

- 何时会调用移动构造与移动赋值？
- 拷贝为何用 `const&`，移动为何不用 `const&&`？
- 自我移动是否合法？
- 继承体系下如何定义移动构造与移动赋值？
- 移动构造与移动赋值是否应抛出异常？

# 移动构造与移动赋值

### • 1. 何时会「移动」而不是拷贝？

- 仍以 `std::string` 为例；唯一差别在形参：`const std::string&` 还是 `std::string&&`？
- 后者只能绑定「rvalue」，我们称之为「rvalue reference」（右值引用）。
- rvalue 也可以被 `const&` 绑定，但优先级更低。
- 讲完值分类后会给出严格答案；
- 粗略地说，rvalue 包括：
- 临时量，例如函数的返回值；
- 以及被 `std::move` 标记的对象。
- 因此 `std::move` 只是把值暂时标成「rvalue」；真正的移动发生在移动构造与移动赋值里！
- 例如，单独的 `std::move(a)` 没有任何效果；`b = std::move(a)` 才会把 `a` 移给 `b`。

# 移动构造与移动赋值

### • 例如：

普通构造，既不是拷贝也不是移动。

拷贝，因为 `s` 既不是临时量，也没有被 `std::move` 标记。移动，因为 `s + s` 返回临时量。

移动，因为出现了 `std::move`。

移动，因为 `CreateAndInsert()` 返回临时量。

# 移动构造与移动赋值

### • 思考：在 `Func(std::string&& s)` 里，`s` 是 rvalue 吗？

- 不是，因为它既不是临时量，也没有被 `std::move` 标记。
- 它只是指代调用方传来的 rvalue，但在被调函数体内已经变成 lvalue。

- 因此有人会说「rvalue reference 未必是 rvalue」。
- 若要再把它当作 rvalue，请用 `std::move(s)`！

# 移动构造与移动赋值

### • 再看一个移动构造的例子。

很朴素；下一讲会改进。

# 移动构造与移动赋值

### • 为什么不移动 `salary`？

- 你也可以移，但对整型而言移动与拷贝完全一样。
- 对任何基本类型，移动与拷贝相同，且拷贝足够便宜。
- 但对朴素的 `std::string`，它用指针表示堆资源，拷贝它再把成员置 `nullptr` 就等于窃取。
- 比拷贝堆内存便宜得多！

### • 事实上，编译器可以自动生成这些成员！

- 隐式拷贝构造 -> 逐成员拷贝构造；
- 隐式拷贝赋值 -> 逐成员拷贝赋值；
- 隐式移动构造 -> 逐成员移动构造；
- 隐式移动赋值 -> 逐成员移动赋值；

# 移动构造与移动赋值

### • 与 `salary` 类似，并非每个类都需要移动操作。

- 例如：

- 没有什么可窃取。
- 这类类通常可以使用默认生成的特殊成员函数。
- 练习：`std::array` 是否有有意义的移动操作？
- 有也没有。
- 没有：`std::array<T, N>` 在栈上是 `T[N]`，无法被「整块窃取」。
- 有：若 `T` 有有意义的移动（如 `std::array<int>`、`std::array<std::unique_ptr<int>>`），则会逐元素移动。
- 对比：`std::vector` 只需交换指针（很便宜），而 `std::array` 需要逐成员移动（比拷贝便宜，但不如 vector）。

# const&&

### • 2. 为何是 `std::string&&` 而不是 `const std::string&&`？

- 想想为何拷贝用 `const std::string&` 而不是 `std::string&`。
- 拷贝过程中我们不想修改实参里的任何东西！
- 再看移动构造做什么：
- 它会「窃取」实参的成员，修改不可避免。
- 因此不打算保持 const！

### • 记住：`const&&` 几乎总是没有用处。

- 下一讲会再讨论。

# 自我移动（Self-move）

### • 3. 自我移动是否合法？

- 即 `a = std::move(a);`
- 我们已经知道它只是调用移动赋值；
- 若移动赋值里没有 `if (&another == this) return *this;` 会怎样？

糟糕！我们会释放自己的资源。

- 拷贝构造若不检查自我赋值也会有类似问题。
- 总之，自我移动是否合法仍取决于你的实现。

# 继承中的移动语义范式

### • 4. 继承下如何定义移动构造与移动赋值？

- 这与继承中的拷贝构造、拷贝赋值非常相似！
- 基类：

# 继承中的移动语义范式

### • 派生类：

- *每行太长，风格未必最佳。
- `Person{anotherStudent}` 涉及从 `Student&` 到 `Person&` 的隐式转换。
- `Person{std::move(anotherStudent)}`：`Student&&` 到 `Person&&`。
- `Person::operator=` 用于调用基类版本（否则 `=` 会直接调用 `Student::operator=`）。

# 移动语义中的 noexcept

### • 5. 移动构造与移动赋值是否应抛出异常？

- 在《错误处理》中我们说过：若确定不会抛异常，应使用 `noexcept`。
- 在先前的代码片段里，你可能会注意到绿色波浪线。

- 但当你实现其它不抛异常的函数（例如 `x * x`）时，往往不会出现这条警告。
- 这说明 `noexcept` 对移动构造/赋值如此重要，以至于需要单独警告！

# 移动语义中的 noexcept

### • 再用一个例子开头——`std::vector` 的重新分配。

- 回顾：当 `size == capacity` 时，`push_back()` 会触发指数级扩容，使其既动态又具有摊还 𝑂(1) 的复杂度。
- 整个过程是：
- 1. 计算新容量并分配新空间；显然
- 2. 让原空间里的元素「搬到」新空间；移动！
- 3. 把新元素 `push` 到新空间；
- 4. 释放原空间。
- 注意 2 与 3 可以互换，因为我们知道新元素应放在哪里。
- 还记得 `push_back` 的异常安全保证吗？
- 强异常保证！
- 意思是：若 `push_back` 中抛异常，状态会回滚，vector 仍是原来的空间与原元素。

# 移动语义中的 noexcept

### • 那就试试看…

输出：COPY Roach COPY Ghost /* 视平台而定，可能出现与收缩相关的输出… */ Test resizing… MOVE Soap COPY Roach COPY Ghost

# 移动语义中的 noexcept

### • 前两次拷贝来自 `std::initializer_list`输出：

COPY Roach
- 我们说过 `initializer_list` 里的每个元素都可视为 const；因此初始化就是逐元素拷贝。COPY Ghost Test resizing…

### • 重新分配时，只有新元素被移动，而原元素全部被拷贝！MOVE Soap

### COPY Roach

COPY Ghost
- 为什么？C++ 难道傻到不开这个优化？

### • 结合异常安全保证，有想法吗？

- 若移动可能抛异常，你如何恢复原始状态？
- 再移回去可能再次抛异常！
- 因此，为确保一定能回滚成功，`push_back` 不会「窃取」原资源；抛异常时只需析构并释放新内存。

# 移动语义中的 noexcept

### • 若用 `noexcept` 保证移动不会失败呢？

输出：COPY Roach COPY Ghost Test resizing… MOVE Soap

### • 多数情况下，移动操作是：MOVE Roach

MOVE Ghost
- 若是移动赋值，会销毁原资源；
- 类似析构函数。
- 「窃取」被移走对象的资源。
- 我们说过标准库假定析构函数为 `noexcept`；「窃取」通常不创建新对象，因此也通常没问题。

### • 结论：移动构造/赋值几乎总应是 `noexcept`。

# 移动语义中的 noexcept

### • 可以用 `std::is_nothrow_move_constructible_v<Type>` 来

### 检测某类型是否 noexcept 可移动构造。

- 赋值：`std::is_nothrow_move_assignable_v<Type>`。

### • 头脑风暴：这段代码是什么意思？

《错误处理》中的作业。

### • 通常你不必这样检测 `noexcept`，仅供参考。

# 移动语义中的 noexcept

### • 好消息：若所有数据成员在默认构造时都不抛异常，

### 则默认构造（无论是隐式声明还是

### `=default`）默认就是 `noexcept`。

- 类似地，若所有数据成员都是 `nothrow_move_constructible`，则默认移动构造为 `noexcept`。

### • 于是，移动构造/赋值最终可能形如：

# 何时使用 noexcept

### • 现在总结何时使用 `noexcept`。

- 对移动构造/赋值，`noexcept` 或至少条件 `noexcept` 几乎是必须的。
- 理由：移动是很基础的优化；因缺少异常保证而退回复制代价很高。
- 析构函数应是 `noexcept`，但可省略（默认即是）。
- 释放函数，包括 `operator delete`，应是 `noexcept`。
- 交换操作，如 `.swap()` 或友元 `swap()`。
- 理由：库中有些函数会利用它来优化。
- 那些显然不会抛异常的方法。
- 特别是哈希器（如 `std::hash` 的特化）、`operator<=>` / `operator==` 更好写成 `noexcept`。
- 据我所知 libstdc++ 在这种情况下对无序容器有很大性能提升。
- 《C++ 核心指南》[[C.85/86/89]] 也提到这一点。

# 拷贝并交换惯用法（Copy-and-swap idiom）

### • 等等，还有一件事…

- 与拷贝赋值类似，移动赋值的实现里也能看到明显的重复。

- 也可以利用拷贝并交换惯用法！

同样，`another` 可以 `MyIntVector&` 形式指代，因为它是 lvalue（再次强调：右值引用形参在函数体内是 lvalue）。

- 但与拷贝不同，移动赋值里的拷贝并交换通常不提升异常安全，只是简化代码。
- 因为「窃取」通常不抛异常，而拷贝可能抛。
- 只有少数情形拷贝并交换几乎必须，例如 `std::shared_ptr`。《内存管理》里会讨论。
- 同样，拷贝并交换比手写操作略贵一点。

- 也有人建议移动赋值可实现为逐成员 `swap`。
- 等价于推迟释放原资源。
- 为什么？想一想。
- 但也有人反对，因为用户往往期望立即释放资源而不是交换，例如 Scott Meyer。
- 作业里会再讨论。

# 值分类与移动语义（Value Category and Move Semantics）

### 零法则 / 五法则

# 零法则 / 五法则

### • 我们说过，某些情况下移动/拷贝构造、移动/拷贝赋值可以

### 由编译器自动生成。

- 实际规则是什么？
- 表格含义：用户对「行」的声明会迫使「列」成为 `table[row][col]`
- 注意：`=default` 与 `=delete` 也算声明。

致谢：Howard Hinnant，表格摘自 https://howardhinnant.github.io/bloomberg_2016.pdf

# 隐式生成规则*

### • 移动构造/赋值列与我们已学内容一致：

- 声明了拷贝构造/赋值、析构函数/另一组移动相关函数会禁用隐式移动声明。

### • 一旦用户显式声明普通构造，默认构造被禁用。

### • 一旦用户声明移动构造/赋值，默认拷贝被

### 禁用。

- 为何不禁用析构/另一组拷贝？
- C++ 视其为设计缺陷并标为弃用，因此最好显式 `=default` 拷贝。
- 于是只定义析构会惊人地让移动操作实际变成拷贝。

# 零法则 / 五法则

### • 规则太多记不住。

- 维护者也很难快速判断类是否可拷贝/可移动。

### • 五法则：显式声明拷贝构造与拷贝赋值、移动

### 构造与移动赋值、析构函数。

- 必要时可用 `=default` / `=delete`，但要显式写出！
- 特例：若它们全是 `=default` 呢？
- 也就是说，只做逐成员拷贝/移动/析构？

### • 零法则：若可能，就不要显式声明其中任何一个。

- 除减少代码行数外，零法则还有额外好处。
- 可平凡拷贝（trivially copyable）！

顺便一提，C++98 里五法则是三法则，在现代 C++ 中不够用。

在《生命周期》一讲中。

# 零法则（Rule of Zero）

### • 回顾：

若在本可默认生成时你却手写定义，有时会失去成为 trivially copyable 的机会。

# 可平凡拷贝（Trivially Copyable）

A 是 trivially-copyable，而 B 不是！

### • 例如：

# 仅可移动类（Move-only class）

### • 特别地，`=delete` 拷贝构造与拷贝赋值而保留移动

### 版本，则类不可拷贝但可移动。

- 标准库里很多例子：`std::unique_ptr`、`std::thread` / `std::jthread`、`std::fstream` / `stringstream` / …、`std::future` / `std::promise` / …
- 通常表示独占某种资源。
- 它们有一些共同特征：
- 1. 有 API 可检查是否实际占有资源，例如 `std::thread::joinable()`、转换为 `bool` 等。
- 2. 有 API 可释放所占资源，例如 `std::thread::join()`、`std::unique_ptr::reset()`、`std::fstream::close()` 等。
- 3. 基于范围的 for 不能用 `auto` / `const auto`，因为涉及拷贝。
- 例如：

编译错误

# 仅可移动类（Move-only class）

- 4. 不能用 `std::initializer_list<MoveOnlyClass>` 初始化 `Container<MoveOnlyClass>`。
- 原因：`std::initializer_list` 提供 const 视图，容器只能逐元素拷贝，而仅可移动类禁止拷贝。编译
- 示例：错误
- 解决方案：

- 稍后我们会用「移动迭代器」等方式给出替代方案。

# 值分类与移动语义（Value Category and Move Semantics）

### 被移动后的状态（moved-from states）

# 被移动后的状态（moved-from states）

### • 通常，类会维护一些不变式（invariant）。

- 例如对 `MyString`，你可能假设它有有效指针并以空字符结尾。
- 但移动操作可能破坏不变式。
- 例如你窃取了 `ptr` 并赋为 `nullptr`。
- 依赖这些不变式的成员函数也会失效。

### • 因此要特别留意被移动后的对象。

- 一般而言，类至少应保证：
- 可安全析构。
- 可安全赋值。
- 且赋值后回到正常状态。

### • 看一些例子。

# 典型不变式及其破坏

### • 案例 1：某些数据成员有特定格式。

### • 例如 `Card` 类，其成员是带 `"-of-"` 的字符串，如

### `"queen-of-hearts"`。

### • 还有一个名为

### `Print` 的函数用于打印点数与花色。

### • 在该不变式假设下，

### 可以写成：

- 于是它有默认移动构造与移动赋值，即逐成员移动。

# 典型不变式及其破坏

### • 因此当 `Card` 被移动后，`cardFullName_` 为空。

- 若此时调用 `Print` 会怎样？
- `substr` 会返回 `std::string::npos`，于是 `pos + 4` 为 3，导致越界访问。

### • 案例 2：数据成员之间的约束。

- 例如，若 `MyIntVector` 的移动构造/赋值不改变被移出对象的 `size`。
- 那么 `.size()` 等操作就失去意义！

# 被移动后的状态（moved-from states）

### • 由于被移动后的状态完全由类的

### 实现决定，有几种应对思路。

- 1. 让被移动后的状态仍满足现有不变式。
- 对 `MyIntVector`，`size_` 应置为 0。
- 对 `Card`，让 `cardFullName_` 为 `"-of-"`，但这会让被移对象仍占有部分资源。
- 但这样 `Card` 的被移状态并不完全自然，因为仍占有资源。
- 在移动操作里 `new` 往往会被很多人反对。

# 被移动后的状态（moved-from states）

- 2. 扩大不变式范围，使所有成员函数都考虑被移动后的状态。
- 对 `MyIntVector`，在每个成员函数里把 `nullptr` 且 `size == 0` 视为合法。
- 对 `Card`，在每个成员函数里考虑空状态。
- 例如在 `Print` 里检查 `if (pos == std::string::npos)`。
- 3. 在文档中明确写出被移对象允许/禁止哪些操作。
- 可用 `assert` 检查。

### • 对用户而言，在对象被赋予新状态之前，

### 要谨慎操作被移动后的对象。

# 值分类与移动语义（Value Category and Move Semantics）

### 与移动语义相关的算法与迭代器

# 与移动语义相关的算法

### • 有几类算法会利用移动语义。

### • 1. 直接表示移动的算法。

- `std::move(inputBegin, inputEnd, outputBegin);`
- 设 `outputEnd = outputBegin + std::distance(inputBegin, inputEnd)`，该 API 将 `[inputBegin, inputEnd)` 的元素逐个移到 `[outputBegin, outputEnd)`，并返回 `outputEnd`。
- 相当于用移动操作做 `std::copy`。
- 等价于：

- 与单参 `std::move(arg)` 同名，属于重载。

# 与移动语义相关的算法

- `std::move_backward(inputBegin, inputEnd, outputEnd);`
- 类似地，用移动实现 `std::copy_backward`。
- 等价于：

- 例如：回忆 `std::vector::insert` 的一种可能实现？
- 必要时重新分配。
- 然后把插入位置之后的元素后移…
- 需要自尾向头的移动以免覆盖！
- 可直接使用 `std::move_backward`。
- `stdr::move` / `stdr::move_backward(inputRange, outputIt);`

# 与移动语义相关的算法

### • 2. 用于「删除」元素的算法。

- `<algorithm>` 里的函数并不能真正「删除」元素，只是返回未删除元素所在的区间。
- 被「删除」的元素落在其余位置，状态未指定，因此可利用移动语义。
- `std::remove(begin, end, value)`：移除所有等于 `value` 的元素；返回迭代器 `mid`，使 `[begin, mid)` 顺序包含所有未被移除的元素。
- 容器配合 `std::erase`。
- `std::remove_if(begin, end, Pred)`：类似，但条件不是 `elem == value` 而是 `Pred(elem)`。
- 实现：双指针；前指针表示保留区，后指针扫描；满足条件时 `*before = std::move(*after)`。

# 与移动语义相关的算法

- `std::unique(begin, end)`：使相邻元素互异，返回 `mid`，`[begin, mid)` 为去重后的序列。
- 例如 `1 1 2 3 3 3 1 => 1 2 3 1`
- 同样可用双指针解决。
- C++23 的 `std::shift_left` / `shift_right`。

### • 3. 许多其它算法会隐式

### 使用移动。

- 例如 `std::rotate` 环形移位；`std::reverse` 通过交换，可能使用移动语义。

# 与移动语义相关的迭代器

### • 我们说过用 `std::initializer_list` 初始化

### 容器会拷贝而不是移动。

- 例如输出：COPY Roach COPY Ghost

### • 通常初始规模不大，拷贝可接受。

- 但若拷贝很贵，或元素仅可移动呢？

### • 除了手工逐个 `push`，还可以用移动

### 迭代器！

# 与移动语义相关的迭代器

### • 示例：

输出：MOVE Roach MOVE Ghost

### • 这显然比 `persons.resize(2);`

### `stdr::move(initPersons, persons.begin());` 更简单、更便宜。

### • 注意：C++17 里我们用 CTAD 省略模板实参；

### C++17 之前可写成 `std::make_move_iterator(…)` 例如：

# 与移动语义相关的迭代器

### • 也可在单次遍历算法上使用移动迭代器，例如

### `std::for_each`：

- 单次遍历是为了保证被移走的对象不再被使用，除非你明确知道其被移后的状态。

# 小结

### • 引言 • 零法则 / 五法则

- 性能提升 • 可平凡拷贝。
- 对「rvalue」的非正式理解 • 仅可移动类
- `std::move`。

### • 被移动后的状态

### • 移动构造 / 移动赋值

### • 与移动相关的算法与迭代器

- 对「rvalue

### semantics

reference」的非正式理解。
- 无用的 `const&&`
- 自我移动
- 与异常相关。
- `noexcept`、拷贝并交换惯用法。
- 与继承相关。

# 下一讲…

### • 我们将正式介绍值分类（value category）。

- 从而正确理解 rvalue reference 是什么。
- 以及 `decltype`！
- 接着会做一些练习：按形参选择分析开销。
- 也会涉及一些进阶主题，例如返回值优化（return value optimization）。
