值分类与移动语义（Value Category and Move Semantics）

## 现代C++基础

## 现代 C++ 基础篇

梁佳明，北京大学本科生

### • 第 2 部分

### • 值类别（value category）

### • decltype

### • 引用限定符（reference qualifier）

### • Deducing this（显式对象形参成员函数）

### • 复制消除（copy elision）

### • 返回值优化（return value optimization）

### • 分析移动语义下的性能

# 值分类与移动语义

值类别（value category）

# 值类别（value category）

- 值类别（value category）是对**表达式**的分类。
- 历史沿革：
- K&R C 中的经典分类：
- lvalue（左值）：可出现在 `=` 左侧的表达式。
- rvalue（右值）：只能出现在 `=` 右侧的表达式。
- 在 C++ 中又加入了 `const`。
- 缺陷：`const` 并不能出现在 `=` 左侧，那为什么不用「能否取地址」来区分？

# 值类别（value category）

- ANSI-C（C89）与 C++98 中的分类：
- lvalue：定位器值（locator value），其地址可用 `&` 取得。
- rvalue：只读值（read-only value）。

# 值类别（value category）

- 自 C++11 起引入了移动语义（move semantics）。
- 我们需要一种类别，既能描述 `std::move(lvalue)`，又能描述临时量（经典意义上的 rvalue）！
- 因为二者都表示「其资源可被窃取」的值。
- 于是把这类统称为 rvalue（右值），而「经典 rvalue」称为 prvalue（pure rvalue，纯右值）。
- 属于 rvalue 但不是 prvalue 的称为 xvalue（eXpiring value，将亡值）。
- 注意：xvalue 在某些性质上类似 lvalue（例如由 lvalue 经 `std::move` 得到），因此也可称它们为 glvalue（generalized lvalue，泛左值）。

# 值类别（value category）

- 更具体地说：
- prvalue 包括：

`int a = 1;`

与那些通常返回值类型而非引用类型的重载运算符一样（例如 `(a++) = 2;` 这类写法里，运算符往往返回值类型）。

也就是成员函数；它们的值类别往往不太重要。

逗号表达式的类别与结果，始终与最后一个子表达式相同。

转换会创建新对象。

与字面量类似；NTTP（非类型模板形参）将在下一讲涉及。

# 值类别（value category）

- 对于 `? a : b`：若 `a` 与 `b` 类型相同且值类别相同，则整个表达式的类别与它们一致；否则会创建新的临时量，因此为 prvalue。

- 总之，在大多数情况下，prvalue 就是临时量！
- 字面量，包括枚举项；
- 返回值为值类型的函数调用结果（因此返回的是临时量）；
- 会创建临时量的运算符与转换；
- 只有少数令人意外的情况，但都不重要。
- 例如：成员函数与 `this`。

# 值类别（value category）

- xvalue 包括：

右值的数据成员，例如 `std::move(a).b`、`A{}.b`。

当 `b` 与 `c` 都是 xvalue 时。

后文在复制消除（copy elision）中再讲。

后文在返回值优化（return value optimization）中再讲。

# 值类别（value category）

- 总之，xvalue 是：
- 右值的数据成员；
- 产生右值引用（rvalue reference）的表达式，例如函数调用与转换。
- 以及若干特殊情形，包括 `?:`、`[]` 与 `,`。
- `std::move` 产生的就是 xvalue！
- 是的，它是一个对原对象产生右值引用的函数；
- 但它如何实现？
- 提示：
- 是的，`std::move(x)` 与 `static_cast<Type&&>(x)` 完全一样！
- 只是后者的简写。
- 注意：对 `const` 对象会生成 `const Type&&`（因而无法被「窃取」），因为丢掉 `const` 是危险的。

# 值类别（value category）

- lvalue 包括：

具名变量。

与那些通常返回引用类型的重载运算符一样。

左值的数据成员，以及静态数据成员。（`p->m` 等价于 `(*p).m`，而 `*p` 总是 lvalue。）

函数的右值引用（但不重要）。

# 值类别（value category）

- 总之，lvalue 大体上就是「活得久」的数据。
- 具名变量；
- 左值的数据成员，以及任意类别对象的静态数据成员；
- 返回左值引用类型的函数调用结果；
- 在效果上等价于对原对象产生左值引用的运算符与转换；
- 特别地，字符串字面量。
- 可以非正式地理解为：它们存放在程序的只读段中，因此「活得久」（而其他字面量往往只是临时量）。
- 以及一些不太重要的情形，例如函数的右值引用。

# 值类别（value category）

- 等等，还有一点……
- cppreference 省略了一些内容……

- 原因：对静态成员与引用成员而言，对象事实上并不「拥有」它们的资源，因此若把它们规约为 xvalue，可能带来意外行为。
- 这会让 `std::move(a.b)` 与 `std::move(a).b` 产生差别。
- 对 `struct A { string b; }`，`c = std::move(a.b)` 与 `c = std::move(a).b` 等价。
- 二者都是 xvalue，因此会调用 `string` 的移动赋值。

## • 对 `struct A { string& b; }`，`c = std::move(a.b)` 与 `c = std::move(a).b` 不等价。

- 前者明确是「移走 `a.b`」，后者则是「从已移动的 `a` 上取得 `b`」。是否「拥有」资源就很重要！

在编写泛型代码时可能需要考虑这一点。

# 值类别（value category）

- 现在可以正式区分不同的引用。
- 左值引用（`Type&`）：只能绑定到（非 `const`）lvalue。
- `const` 左值引用（`const Type&`）：为与 C++98 一致，可绑定到任意值类别，但只读。
- 右值引用（`Type&&`）：只能绑定到（非 `const`）rvalue，即 xvalue 与 prvalue；因此其资源可能被窃取。
- `const` 右值引用（`const Type&&`）：基本无用。

- 作为形参时，重载决议规则为：
- 非 `const` lvalue 会先尝试匹配 `&`，其次 `const&`。
- `const` lvalue 只会尝试匹配 `const&`。
- rvalue 会先尝试匹配 `&&`（再 `const&&`），其次 `const&`。

# 值类别（value category）

- 练习：Alice 听说 `const` 有利于优化，于是写了这样一个函数：

- 解释上面所有的 `const`，并尝试找出性能陷阱。

# 值类别（value category）

`ele` 是只读的。

该方法不能修改任何数据成员（`mutable` 除外）；临时的 `const A` 也能调用该函数。是只读的。

- 问题：只读临时量完全没用。
- 你仍可用非 `const` 变量去接收它。
- 但它产生的是 `const` rvalue，无法绑定到 `A&&`！
- 只能绑定到 `const A&`，因此会调用拷贝构造函数而非移动构造函数！
- 结论：返回 `const` 值类型几乎总是无益。（返回引用类型可能有用，例如 `operator[]`。）

# decltype

- 那有没有办法判断表达式的值类别？
- 有，`decltype`！
- declared type 的缩写。
- 读音：`/ˈdaɪkl/` 或 `/ˈdiːkwəl/`

- 这是一个从变量名（含成员访问）或表达式推导类型的关键字。
- 两类规则不同！

注意：某些编译器实现有误时，可能把某些表达式分错类，例如 msvc。

# decltype

- 对变量名与成员访问推导类型：与声明类型一致。
- 例如 `a`、`a.b`、`ptr->b`
- 例如：

可用 `std::remove_reference_t<decltype(str)>` 始终得到值类型。

# decltype

- 对表达式推导类型：
- `decltype(prvalue)` → 值类型。
- `decltype(lvalue)` → 左值引用。
- `decltype(xvalue)` → 右值引用。
- 借此可判断表达式属于哪一类值类别。

- 例如 `T1 == int`，`T2 == int&`，`T3 == int&&`

# decltype

- 若再套一层括号，变量名就变成表达式。
- 而变量名作为表达式时是 lvalue，因此总会得到左值引用。
- 例如：

# decltype(auto)

- 有时你需要 `decltype(Statement) var = Statement;`
- 但不能用 `auto`，因为它只会推导 decay 后的类型。
- 但这么写太长……
- C++ 提供了 `decltype(auto)`！
- 可直接写 `decltype(auto) var = Statement`。
- 与 `auto` 类似，也可用于函数返回类型，例如 `decltype(auto) Func() { return 1; }`。
- 练习：对 `int a = 1;`
- `decltype(auto) b = a;`
- `decltype(auto) d = (a);`
- `decltype(auto) e = std::move(a);`
- `decltype(auto) c = 1;`

# 值分类与移动语义

引用限定符（reference qualifier）

# 引用限定符（reference qualifier）

- 似乎对基本类型非法的一些操作，在带运算符重载的类上却变成合法。
- 为什么？

编译错误

编译居然可以通过？！

# 引用限定符（reference qualifier）

*
- 重载运算符本质上是函数调用，因此等价于 `operator+(b, 1).operator+=(1)`。
- `b.operator+` 产生 `Integer` 的 rvalue。
- 而 `Integer` 的 rvalue 当然可以进行函数调用……
- 若要让它非法，就需要禁止 rvalue 调用它。
- 这正是引用限定符（reference qualifier）要做的事！

*不过自 C++17 起，求值顺序与内建运算符一致，这在第 1 讲已提过。

# 引用限定符（reference qualifier）

- `&` 只绑定 lvalue，`&&` 只绑定 rvalue。
- 也可与 cv 限定符组合，因此 `&` 表示绑定非 `const` lvalue，而 `const&` 表示捕获所有值（等价于 `Integer&` 与 `const Integer&`）。
- 与 cv 限定符不同：一旦使用引用限定符，不带引用限定符的重载就是非法的。
- 例如：

# 引用限定符（reference qualifier）

- 许多令人惊讶的实用技巧，本质上来自对值类别的限制。提示：基于范围的 `for` 循环的本质是……
- 案例 1（C++23 之前）：下面这段代码有 bug 吗？

万能引用（universal reference）；此处可暂时把它看成对初始化器的 `const&`（仅限此处与当前语境）。

# 引用限定符（reference qualifier）

- 于是我们的程序类似：

- `RecruitNewPerson` 返回临时的 `Person`……
- 而 `GetName` 返回其成员的引用！
- 第一条语句结束后，`Person` 临时量会被销毁，引用因此悬空！
- 于是我们的 `for` 循环在已释放的内存上迭代……
- 等等，你可能记得我们在《生命周期》一讲里教过：

顺带一提，`&&` 也能延长生存期。

# 引用限定符（reference qualifier）

- 是的，但它延长的是 `const std::string&` 所引用的对象，而不是 `Person` 临时量本身。
- 因此不会延长 `Person` 的生存期……

- 方案 1：让 `GetName` 返回 `std::string`。
- 这样可以通过引用延长生存期。
- 但对 lvalue 调用 `GetName` 可能低效，因为函数调用总会创建新的 `std::string`。
- 即 `const auto& str = person.GetName()` 会不必要地创建 `std::string`。

# 引用限定符（reference qualifier）

- 方案 2：自 C++20 起使用基于范围的 `for` 的 init-statement。

- 但这需要使用者自己小心；能否从源头避免悬空？
- 方案 3：使用引用限定符！
- 对 lvalue 返回引用；
- 对 rvalue 返回值类型！
- 使用 `std::move` 是因为 rvalue 大体表示「值可被窃取」，因此移走成员既合理又高效。

# 引用限定符（reference qualifier）

- 注 1：除了防 bug，也可用来提升性能。
- 例如：

- 这等价于 `std::move(person.name_)`，但通过 Getter 暴露出来。
- 注 2：自 C++23 起，范围初始化式（range-initializer）中由表达式产生的大多数临时量的生存期会自动延长。
- 因为这个 bug 太常见……
- 除非刻意为之，这里的生存期问题基本不再是问题。

# 引用限定符（reference qualifier）

- 案例 2：`std::optional` / `expected` 的优化。
- 例如：

注意 `opt` 里的 `Object` 已被移走。它仍有 `.has_value()`，但值处于 moved-from 状态。

- 因此若使用 lvalue，链上的第一个 `or_else` 会拷贝；需要 `std::move(xx).or_else()` 才能走移动。

# Deducing this

- 自 C++23 起，也可使用显式对象成员函数（非正式名称 Deducing this）。
- 若第一个形参用 `this` 修饰，且 decay 后的类型就是类本身，则第一个形参就是对象本身。
- 即此处 `this == &self`。
- 哇，这有点像 Python！
- 你还能做一些全新的东西……

# Deducing this

- 可以让显式对象按值传递！
- 例如，若某对象很小，我们说过最好用值类型而非引用类型（例如减少别名）。
- 因此若不需要修改原对象，可以写成：

- 汇编层面的变化。致谢：C++ Team Blog《C++23's Deducing this: what it is, why it is, how to use it》

# Deducing this

- 注 1：所有成员都应通过第一个形参访问；`name_`、`this` 与 `this->name_` 都非法。
- 注 2：它完全取代原函数；
- 不能在函数声明符末尾再加任何限定符；
- 不能定义同用途的非显式对象成员函数。

- 注 3：可用这种方式定义递归 lambda。
- 问题：这里的 `this auto` 是什么意思？

若 lambda 很大，也可用 `auto&` 或等价写法。

# 值分类与移动语义

复制消除（copy elision）

# 值分类与移动语义

- 复制消除（copy elision）
- prvalue 的复制消除
- 返回值优化（return value optimization）

# 复制消除（copy elision）

- 观察：`auto a = std::string{"Hello, world"};`
- 即 `std::string a = std::string{"Hello, world"};`
- 这里按理该调用哪些函数？

- 逻辑过程：`"Hello, world"` 用来构造 `std::string` 临时量，再调用移动构造函数构造真正的变量。
- 事实：`"Hello, world"` 并不会调用移动构造函数。
- 这种合理优化称为「复制消除（copy elimination）」，因为不需要任何拷贝（与移动）。
- 若移动构造函数有副作用（例如输出），这确实会带来副作用层面的差异，因此编译器有责任检查。
- 但自 C++17 起，标准强制要求此类消除，即使有副作用也必须发生。

# 复制消除（copy elision）

- 我们知道 prvalue 通常是短命的临时量，要么被丢弃，要么用于生成对象。
- 因此对 prvalue 会发生复制消除：折叠临时量的中间构造步骤。
- 也就是说，尽量推迟「用 prvalue 构造真实对象」的时机。
- 当对象最终被构造出来时，就说该 prvalue 被实体化（materialized）了。

- 例如 `auto a = std::string{"Hello, world"};`
- `std::string{"Hello, world"}` 是 prvalue，因此其构造会尽量推迟，直到确定必须产生结果对象（即 `a`）。

# 复制消除（copy elision）

- 那么「尽量推迟」的定义是什么？
- 或者说，除了结果对象之外，prvalue 何时必须实体化？

- 绑定到某些引用（例如 `T&&`、`const T&`）；
- 例如 `void Func(const A&); Func(A{});`
- 对类对象，访问其非静态数据成员 / 调用非静态成员函数；
- 例如 `A{}.a;`
- 【很少用】作为数组，被下标访问或转换为指针；
- 用于 `std::initializer_list`；
- 例如 `std::vector{ A{}, A{} };`
- 或最终被丢弃（因为逻辑上本应创建新对象）。

- 若结果对象不是 lvalue，则实体化为 xvalue。

# 复制消除（copy elision）

- 例 1：

# 复制消除（copy elision）

- 例 2：构造函数与析构函数各调用几次？
- 注意：没有中间步骤强迫返回的 prvalue `create()` 实体化。
- 最终没有真正产生结果对象，因此被丢弃的 prvalue 会用于实体化为 xvalue。
- 构造与析构各只调用一次。
- 注意：这在 msvc 19.29（VS2019 最终版）上无法通过编译；该 bug 在 msvc 19.30（VS2022）中已修复。
- 例 3：`Test{}.obj` 的值类别：
- `Test{}` 是 prvalue，并尝试推迟实体化。
- 但访问其成员会强迫它实体化为 xvalue。
- xvalue 的非静态成员仍是 xvalue。

# 值分类与移动语义

- 复制消除（copy elision）
- prvalue 的复制消除
- 返回值优化（return value optimization）
- 隐式移动（implicit move）

# RVO

- 还记得我们的例子吗？
- 逻辑过程：
- `strVec` 创建临时量；
- `strVec` 被析构；
- 临时量赋给 `v`。
- 事实：我们说过由于 NRVO，过程简化为：
- `strVec`（视为临时量）赋给 `v`。

# RVO

- 一个更明显的例子……

# RVO

- 输出：在 Windows/VS2019(msvc19.29)/Release：在 Linux/g++-11/无选项：

RVORVO Construct at 000000AC4FBCFE72Construct at 0x7fff3c079d85 Construct at 000000AC4FBCFE70Construct at 0x7fff3c079d87 Move Assignment at 000000AC4FBCFE72Move Assignment at 0x7fff3c079d85 Destruct at 000000AC4FBCFE70Destruct at 0x7fff3c079d87

NRVONRVO Construct at 000000AC4FBCFE71Construct at 0x7fff3c079d86 Construct at 000000AC4FBCFE70Construct at 0x7fff3c079d87 Move Assignment at 000000AC4FBCFE71Move Assignment at 0x7fff3c079d86 Destruct at 000000AC4FBCFE70Destruct at 0x7fff3c079d87

Done.Done. Destruct at 000000AC4FBCFE71Destruct at 0x7fff3c079d86 Destruct at 000000AC4FBCFE72Destruct at 0x7fff3c079d85

# RVO

- 大体等价，因此不妨选 Linux。
- gcc 有关闭该优化的选项，我们再试一次：NRVO Construct at 0x7ffc5e3d4bc6 In Linux/g++-11/-std=c++11 -fno-elide- Construct at 0x7ffc5e3d4b97 constructors: Move at 0x7ffc5e3d4bc7 Destruct at 0x7ffc5e3d4b97 RVO Move Assignment at Construct at 0x7ffc5e3d4bc5 0x7ffc5e3d4bc6 Construct at 0x7ffc5e3d4b97 Destruct at 0x7ffc5e3d4bc7 Move at 0x7ffc5e3d4bc7 Destruct at 0x7ffc5e3d4b97 Done. Move Assignment at 0x7ffc5e3d4bc5 Destruct at 0x7ffc5e3d4bc6 Destruct at 0x7ffc5e3d4bc7 Destruct at 0x7ffc5e3d4bc5

Object GetObject_RVO() { Object obj;

# RVO

return obj; }
- 对比：`intmain() { Object obj1; obj1 = GetObject_RVO(); }`

NRVONRVO Construct at 0x7fff3c079d85Construct at 0x7ffc5e3d4bc5 Construct at 0x7fff3c079d87Construct at 0x7ffc5e3d4b97 Move Assignment at 0x7fff3c079d85Move at 0x7ffc5e3d4bc7 Destruct at 0x7fff3c079d87Destruct at 0x7ffc5e3d4b97 Move Assignment at 0x7ffc5e3d4bc5 Destruct at 0x7ffc5e3d4bc7
- NRVO 消除了「从临时量到返回值」的移动以及临时量的析构。

# RVO

- 因此 RVO 有两类。
- 若返回对象是 prvalue（例如 `Object{}`），自 C++17 起当然会消除临时量，也常直接称为 RVO。
- 因此不再多谈。
- 否则，若返回对象是 lvalue（如我们前面的例子），则称为 NRVO（Named RVO）。
- NRVO 适用条件有很多限制：
- 必须是名字，不能是别的形式（即 `return x;`）。
- 必须是局部变量，且类型与函数返回类型相同；
- 不能是全局变量：离开函数作用域并不会结束全局对象的生存期，因此无法套用 NRVO 那套与返回值槽位合并的规则。
- 不能是形参。
- 必须是所有 `return` 语句里唯一被返回的变量。

- 例如：
- `return std::move(obj);` 不是名字 不是名字
- `return static_cast<Object>(obj);`
- `Object obj; Object Func() { return obj; }` 不是局部变量
- `Object Func(Object obj) { return obj; }` 是形参
- `Object Func() { Object obj1, obj2; if (condition) return obj1; else return obj2; }` — 并非所有 `return` 都返回同一变量。
- `Object Func() { Object obj1; if (condition) return obj1; // …… return obj1; }`
- `Object Func() { int m = 1; return m; }` — `m` 的类型（`int`）与返回类型（`Object`）不同。

# NRVO

- 这就是为什么我们说 `return std::move(x)` 可能降低性能……

有 NRVO：因此没有中间临时量。无 NRVO：因此会创建中间临时量（intermediate temporary）。后者会多一次移动构造函数调用。
- 某些编译器提供检查选项，例如 `gcc -Wpessimizing-move` 与 `–Wredudant-move`

# NRVO

- 解释前面的代码：
- 为什么这里写 `std::move` 是合适的？
- 原因：`name_` 不是局部变量！
- 因此 `return name_;` 不会触发 NRVO，会导致拷贝到临时量。

# 隐式移动（implicit move）

- 有时 NRVO 不可能发生，但拷贝也不必要……
- 例如

- 虽然无法触发 NRVO，`obj1` 与 `obj2` 也不必拷贝到临时量；它们可以「移动」到临时量。
- 这与写 `std::move(obj1)` 与 `std::move(obj2)` 一样，只是省得你反复手写 `std::move`。
- 这类优化称为「隐式移动（implicit move）」。

# 隐式移动（implicit move）

- 隐式移动的历史：
- 从 C++11 到 C++20，会进行「两次重载决议」。
- 即先尝试把结果当作 rvalue；失败再当作 lvalue。
- C++11 要求返回类型必须完全相同。
- C++14 放宽，但仍要求返回类型能显式接受返回值类型的右值引用。
- C++20 再放宽，允许接受值类型。

- 然而规则含糊（导致不同编译器行为不同）且难记，于是 C++23 给出最终方案。
- 即：所有带自动存储期的非 `volatile` 变量（含形参）或其右值引用，在 `return` 时都会被视为 xvalue。
- 允许额外加括号。

注意：截至 2024/11，msvc 尚未实现该规则。

# 隐式移动（implicit move）

- 破坏性变更：某些悬空引用会导致编译错误。
- 原因：xvalue 不能绑定到左值引用。
- 但悬空的 `const&` 与 `&&` 仍可能合法通过编译。

- 思考：`Test()` 的返回类型是什么？
- 错误答案：C++23 起是 `Object&&`，之前是 `Object&`，因为分别是 xvalue / lvalue。
- 原因：这里等价于 `decltype(obj1)`，即推导**变量**的类型。它只推导类型，与值类别无关。
- 正确答案：始终是 `Object`。

# 隐式移动（implicit move）

- 若是：

- 现在不是变量，而是表达式。
- 因此 `decltype` 会考虑值类别。
- C++23 之前，它只是 lvalue，推导出 `Object&`。
- 而自 C++23 起，

- 于是它是 xvalue，推导出 `Object&&`。
- 这里两种推导都危险，因为引用悬空；但若 `obj1` 是形参 `Object& obj1`，则没问题。

# 隐式移动（implicit move）

- 练习：

- 这会触发 NRVO 吗？不是名字
- 这会触发隐式移动吗？不是名字
- 这会触发拷贝吗？
- 仍然不会，因为 `A{}` 是 prvalue，实体化为 xvalue 后，访问其成员仍是 xvalue。
- 因此这只是移动，并非由隐式移动引起。

# 复制消除（copy elision）

- 总结：对返回值：
1. 检查是否为 prvalue，从而可能触发 RVO；
2. 检查是否为满足那些限制的局部变量，从而可能触发 NRVO；
- 若都不是，你可以手动对每个 `return` 写 `std::move`，或在下列情形中省略它们：
1. 检查返回的是否只是局部变量，从而隐式移动；
2. 检查表达式是否为 rvalue。

- 若全为假，而你仍想移动返回的对象（例如前面 `Person` 里的 `return name_;`），就必须显式加 `std::move`。
- 否则会发生拷贝。

# 值分类与移动语义

分析移动语义下的性能

# 分析性能

- 我们用函数形参选择来练习分析移动语义。
- 我们说过这个构造函数实现很朴素。
- 这里可以加 `std::move`。
- 事实上参数还有另外三种选择（为简便起见省略 `salary`）：
- `Person(std::string& init_name) : name{init_name} {};`
- `Person(const std::string& init_name) : name{init_name} {};`
- `Person(std::string&& init_name) : name{std::move(init_name)} {};`
- 哪一种最好？
- 请自己动手：用不同值类别的实参传入并分析！

# 场景分析

- 首先，由于 prvalue 大体就是临时量，总会调用构造函数——要么直接构造形参，要么构造临时量再绑定到形参，因此下面略去 prvalue 情形。
- `std::string`
- 对 lvalue：先拷贝到形参，再把形参移动到成员。
- 1 次拷贝构造 + 1 次移动构造 + 1 次空状态析构；
- 对 xvalue：先移动到形参，再把形参移动到成员。
- 2 次移动构造 + 1 次空状态析构；
- 对 prvalue：形参直接构造，再移动到成员。
- 1 次移动构造 + 1 次空状态析构。

# 场景分析

- `std::string&`
- 对 lvalue：传参时无事发生，形参再拷贝到成员。
- 1 次拷贝构造。
- 不能接受 rvalue。
- `std::string&&`
- 不能接受 lvalue。
- 对 xvalue：传参时无事发生，形参再移动到成员。
- 1 次移动构造
- 对 prvalue：构造临时量并绑定到引用；再移动到成员，最后析构。
- 1 次移动构造 + 1 次空状态析构

# 场景分析

- `const std::string&`
- 对 lvalue：传参时无事发生，形参再拷贝到成员。
- 1 次拷贝构造
- 对 xvalue：传参时无事发生，形参再拷贝到成员。
- 1 次拷贝构造
- 对 prvalue：构造临时量并绑定到引用；再拷贝到成员，最后析构。
- 1 次拷贝构造 + 1 次析构

# 普通构造函数的形参选择

- 汇总成一张表：lvalue xvalue prvalue 值类型 1 次拷贝 + 1 次移动 +2 次移动 + 1 次空1 次移动 + 1 次空 1 次空析构析构析构 & 1 次拷贝 否 否

const & 1 次拷贝 1 次拷贝 1 次拷贝 + 1 次析构 && 否 1 次移动 1 次移动 + 1 次空析构
- 不能同时重载值类型与引用版本（会歧义），因此只有两种选择：
- 值类型；
- 用 `const&` + `&&` 重载
- 由于匹配优先级，rvalue 会先匹配 `&&`，因此：lvalue xvalue prvalue const & + && 1 次拷贝 1 次移动 1 次移动 + 1 次空析构

# 普通构造函数的形参选择

- 于是对满足下列条件的构造函数形参：
- 与成员类型相同；
- 可移动；
- 并用于直接初始化该成员；
- 看起来显然该选 `const&` + `&&`。
- 然而……
- 关键观察 1：这两种选择的性能差距只是 1 次移动构造 + 1 次空状态析构。
- 关键观察 2：类里通常有很多数据成员；若每个成员都引入两套重载形参的选择，构造函数的重载组合数会随成员数指数增长。

# 普通构造函数的形参选择

- 结论：
- 若形参的移动构造与空状态析构足够便宜，用值类型完全可以接受。
- 否则更好用引用重载。
- 例如：`std::vector<std::string>` 更接近前者，`std::array<std::string, 1000>` 更接近后者。
- `std::vector` 只持有 3 个指针，移动构造只是复制 3 个指针；
- 而 `std::array` 持有整块数组，移动构造必须把每个元素移到另一对象里，因此移动构造成本相对很高！
- 移动语义通常把成本降到「浅拷贝」，但若浅拷贝本身也很贵，就仍然低效。

# 分析性能

- 第二个例子是 Setter。
- Getter 已通过引用限定符（或 Deducing this）解决。

- 对 setter，同样可以：
- 按值类型定义；
- 用 `const&` + `&&` 重载。
- 看起来唯一差别只是 1 次移动构造 → 1 次移动赋值。

# 分析性能

- 然而，移动赋值会丢弃原有资源……
- 有时并不合适，例如对 `std::string`，原有内存可能更大。
- 例如若之后向字符串追加内容，更大的空间更可能避免再次重分配。
- 总结：
- 对构造全新对象（如构造函数），可用值类型 / `const&` + `&&`；
- 对赋值到已有对象（如 setter），需要你自己权衡后续操作的效率。
- 若保留现状更好，`const&` 与拷贝可能就够；
- 否则同上。

# 分析性能

- 特别地，这里假设类既可拷贝又可移动。
- 对仅可移动的类，形参只能是值类型或 `&&`；后者成本总是更低。
- 只有在你总要放弃调用方所有权时，才用值类型。
- 以上都是「把形参直接赋给某处」的情形；还有其他形参情形：
- 只读：`const&`，或值类型（若足够小）；
- 可写：`&`；
- 创建对象并向外给出：用返回值。
- C++98 里常用 `&`，但 NRVO 与隐式移动让返回值既便宜又方便。

# 小结

- 值类别（value category）
- lvalue、xvalue、prvalue
- `decltype`、`decltype(auto)`
- 引用限定符（reference qualifier）
- Deducing this
- 复制消除（copy elision）
- prvalue 的复制消除
- 返回值优化（return value optimization）
- 隐式移动（implicit move）
- 性能分析

# 下一讲……

- 模板基础！
- 关于特化、concept、类型推导等。
- 并回到移动语义，讨论万能引用（universal reference）。
