补充与总结（Supplementary and Summary）

## 现代C++基础

## 现代 C++ 基础（Modern C++ Basics）

梁佳明，北京大学本科生

自 2024 年 9 月起于北京大学攻读研究生 :-)

### • 文件系统（File system）

### • 时间工具（Chrono）

### • 数学工具（Math utilities）

### • 总结与展望（Summary and future prospect）

# 补充与总结（Supplementary and Summary）

文件系统

# 概览

- 文件（File）表示存于非易失性存储（non-volatile storage）中的有组织数据，使程序能在不同运行之间共享数据。
- 文件是数据的命名集合。
- 目录（Directory）用于构建文件层次结构。
- 目录是文件与目录的命名集合。

- 文件系统是操作系统（OS）提供的抽象层，使用户能通过路径（path）访问文件与目录。
- 它记录文件与目录的元数据（大小、修改时间、所有者等），使其有序且呈层次结构。
- C++17 在 `<filesystem>` 中纳入了相关工具。

# 补充

- 文件系统
- 路径操作
- 概览
- `std::filesystem::path`
- 文件系统操作

# 路径概览

- 本质上，路径是表示文件位置的字符串。
- 路径有两种：
1. 绝对路径（absolute path）：始终指向同一位置。
2. 相对路径（relative path）：相对于当前进程当前工作目录（CWD, current working directory）的位置。
- 改变 CWD 后，进程解析到的位置可以不同。
- 路径由以下部分组成：
1. 根名（root name，可选）：如 Windows 上的盘符（`C:`, `D:`）；或 UNC（`//machine`）等。
2. 根目录（root directory，可选）：目录分隔符（Windows 上 `\`，Linux 上 `/`，经典 MacOS 上 `:`）。
3. 相对路径：由目录分隔符分隔的文件名序列。

# 路径概览

- 除不得包含分隔符外，文件名还有许多平台相关特性或限制。例如 Windows：

例如：某些 Windows 函数中著名的 260 限制。

例如：Windows 上的 `CON`。

例如：Linux 区分大小写而 Windows 不区分。

特别地，`.` 与 `..` 分别表示当前目录与父目录。

# 路径概览

- 为使程序可跨平台，C++ 规定了一种使用 POSIX 约定的「通用格式」（generic format）。
- 即：将上述三部分直接拼接成路径。
- 并且将 `/` 视为通用分隔符。
- 此外，C++ 还允许依赖文件系统的「原生格式」（native format）。
- 例如 OpenVMS——银行曾用的遗留系统（若其真支持 C++17 工具的话）。

例如：`DKA0:[JDOE.DATA]test.txt`

# 路径概览

- 注 1：「通用」仅表示在所有系统上都是合法格式；其指向的位置未必相同。
- `D:\sub\path` 是什么？
- Windows：D 盘上的绝对路径，包含 `sub` 与 `path` 两个分量。
- Linux：相对路径，文件名为整个字符串 `D:\sub\path`，即整串是一个分量。
- `/home/user` 是什么？
- Linux：绝对路径，包含 `home` 与 `user` 两个分量。
- Windows：位于 CWD 所在盘上的相对路径，包含 `home` 与 `user` 两个分量。
- 注 2：依赖相对路径不太安全，它像全局变量，可被其他线程与外部库改变。
- 即：相对路径解析到的位置可被任意修改。

# 路径概览

- 注 3：「相对」或「绝对」只表示是否受 CWD 影响；仍可能存在多条路径指向同一位置。
- 例如：`/home/user`、`/home/user/.`、`/home/user/dir/..`、…。
- 为统一各种表示，可做路径规范化（normalization）。
- 规范化有两类：
- 词法（lexical）：字符串层面的替换，不改变路径是相对还是绝对。
- 因此 `/home/user`、`/home/user/.`、`/home/user/dir/..` 均规范化为 `/home/user`；`./user`、`./user/.`、`./user/dir/..` 均规范化为 `user`。
- 依赖文件系统：将路径规范化为唯一的绝对路径。
- 设 CWD 为 `/home`，`./user`、`./user/.`、`./user/dir/..` 均规范化为 `/home/user`。
- C++ 将此类规范化称为「规范路径」（canonical）。

# 路径概览

- 具体而言，规范化路径要求：

POSIX 可能（因 UNC 路径与否）保留 `//`。

出处：C++17 the Complete Guide, Nicolai M. Josuttis.

# 补充

- 文件系统
- 路径操作
- 概览
- `std::filesystem::path`
- 文件系统操作

为简洁起见，下文使用 `namespace stdfs = std::filesystem;`。

# 路径

- C++ 用 `stdfs::path` 表示路径。
- 它本质上是某种原生编码的字符串，例如 Linux 上 UTF-8，Windows 上 * UTF-16。
- 底层字符类型可查 `stdfs::path::value_type`，通常为 Linux 上 `char`、Windows 上 `wchar_t`。
- 且 `stdfs::path::string_type = std::basic_string<value_type>`。
- 字符串以原生格式存储路径。
- 可以常量方式访问底层字符串：

*：严格来说，文件系统通常并不真正「尊重」编码；只是把路径当作字节序列（即便不是合法 UTF-8/16）。故「原生编码」本质上指「可直接传给文件系统系统调用的字符串」，例如 Windows 上以 2 字节为单位的序列。

# 路径

- 不过，可用任意编码与格式构造路径。

- 对格式而言，本质上是 `stdfs::path` 中的限定枚举（scoped enumeration），含三个枚举项：

- 默认使用 `auto_format`，即自动判断输入为原生还是通用格式并在必要时转换。

# 路径

- 模板允许使用任意字符类型，构造函数会转换到原生编码。

- 对 `wchar_t`，Windows 不做转换而 Linux 需要；
- 对 `char`，Linux 不做转换而 Windows 需要。
- 实际上，Windows 文件 API 将 `char` 视为 ANSI（或活动）代码页（ACP, Active Code Page）。
- 这与编译器选项交互时会导致复杂行为…

# Windows ACP（Windows 活动代码页）

- 假设 Windows 上存在文件路径 `D:\试验.txt`。
- 且使用默认中文 PC，即 ACP 为 GBK（编号 936）。
- 给定 `main.cpp` 为：

检查某路径是否存在，等价于：若 `p` 是文件而非目录，则等价于 `std::ifstream{p}.is_open()`。
- 情况 1：msvc 不加任何选项，且 `main.cpp` 编码为 GBK。
1. `D:\试验.txt` 在源文件中是 GBK，msvc 按 GBK 正确读入。
2. 执行字符集为 GBK，故 `D:\试验.txt` 在二进制 exe 中仍为 GBK。
3. 当前 ACP 为 GBK，故 `stdfs::path` 将其从 GBK 转为原生编码（UTF-16）存储；
4. 路径正确，文件系统报告存在。

# Windows ACP

- 情况 2：msvc 加 `/utf-8`，且 `main.cpp` 为 UTF-8。
1. `D:\试验.txt` 在源文件中是 UTF-8，msvc 按 UTF-8 正确读入。
2. 执行字符集为 UTF-8，故 `D:\试验.txt` 在 exe 中为 UTF-8。
3. 当前 ACP 仍为 GBK，故 `stdfs::path` 仍按 **GBK** 去解释并转到原生编码（UTF-16）；
- 但 UTF-8 串并非真正的 GBK 串，对应二进制按 GBK 会显示为 `D:\璇曢獙.txt`。
4. 路径不正确，文件系统报告不存在。
- 情况 3：msvc 加 `/source-charset:utf-8`，且 `main.cpp` 为 UTF-8。
1. `D:\试验.txt` 在源文件中是 UTF-8，msvc 按 UTF-8 正确读入。
2. 默认执行字符集为 ACP，故 `D:\试验.txt` 在 exe 中为 GBK。
3. 路径仍正确，文件系统报告存在。

# Windows ACP

- 情况 4：msvc 加 `/utf-8`，且 `main.cpp` 为 GBK。
1. 源中 GBK 的 `D:\试验.txt` 不是合法 UTF-8，msvc 警告 C4828（UTF-8 非法字符）并静默保留原始字节。
- 于是偶然地，exe 里仍是 GBK。
2. 于是偶然地，路径正确，文件系统报告存在。
- 情况 5：设 ACP 为 UTF-8（65001），msvc 不加选项（等价于加 `/utf-8`），且 `main.cpp` 为 GBK。
1. 同情况 4，即串在 exe 中为 GBK。
2. 但 GBK 不是合法 UTF-8，故 `path` 构造函数抛异常（MS-STL 中为 `std::system_error`）。

# 路径构造

- 因此要得到合法路径，首先需要：
- 确保编译器知道如何读取源文件（字符串字面量），即源字符集（source charset）应正确指定文件编码。
- 然后两种方式：
1. 使执行字符集（针对字面量）与串编码（例如 `std::string` 中存储的其他串）都与 ACP 一致。
2. 改用 `char8_t[]` / `char16_t[]` / `char32_t[]`。A. 任意 ACP 均可，因 `char8_t` 作为独立类型在构造时总按 UTF-8 解码。B. 任意执行字符集均可，因 `char8_t` 在二进制 exe 中总为 UTF-8。

- 若明知 `std::string` / … 本质为 UTF-8 而 ACP 不是，可用 `reinterpret_cast` 避免错误转换。

# 路径构造

- 另一方面，在 Linux + gcc 上，无论执行字符集如何，`char[]` 都不会做转换。
- 因 `char` 即原生编码，libstdc++ 假定字节正确。
- 相反，`-fwide-exec-charset` 指定 `wchar_t` 的编码并自动转为 UTF-8。
- 若要显式指定编码与转换，可用 locale：

- 因转换在 locale 中显式指定，模板只接受 `char` 序列。

# 路径构造

- 当原生编码为 `wchar_t`：
- 在 locale 中用 `codecvt<wchar_t, char, std::mbstate_t>` 将 `char[]` 转为原生 `wchar_t[]`。
- 例如 Windows：GBK → UTF-16。
- 当原生编码为 `char`：
- 先用 locale 中 `codecvt<wchar_t, char, std::mbstate_t>` 将 `char[]` 转为 `wchar_t[]`；
- 例如 Linux：GBK → UTF-32
- 再把 `wchar_t` 转回原生 `char`（如 UTF-8）。
- 例如 UTF-32 → UTF-8，等价于直接用 `wchar_t[]` 构造。

默认为空串。
- 最后还有一些省略的重载与 `operator=`，仅列于此。

# 路径

- 除构造外，也可取得不同格式与编码的字符串。
- 原生格式 + 原生编码：用 `.native()`，如前所述。
- 原生格式 + 转换后编码：

- 通用格式 + 转换后编码：

- 特别地，这些函数要求使用 `/` 作为目录分隔符。

# 路径

- 还有带分配器（allocator）的模板版本：

- 故要「通用格式 + 原生编码」，用 `.generic_string<stdfs::path::value_type>()`。
- 最后，可用 `static constexpr stdfs::path::preferred_separator` 查看首选分隔符。
- Windows 为 `\`，Linux 为 `/`。

注：纯扩展名的文件名（如 `"D:\\.gitignore"`）不算扩展名（即 stem = filename =

# 路径分解

`.gitignore`，extension = `""`）

- 还有一些观察函数用于查询路径；涉及拼接分量、路径本身、以及追加（append）后的结果等
- 均返回新的 `stdfs::path`。

根路径（Root path） 相对路径（Relative path）路径（Path）

根（Root）根（Root）父路径（Parent path）文件名（Filename）目录（Directory）主名（Stem）扩展名（Extension）（带点）

# 路径分解

- 特别地，此类分解是词法的，甚至不与真实文件系统交互。
- 故「父路径」并不返回父目录的真实路径，只是去掉最后一个分量。
- 当路径以目录分隔符结尾时，最后一个分量为空，故 `parent` 只去掉分隔符。

- 例如：

注：根目录的父路径仍是根目录（即 `"/"` → `"/"`）；但纯文件名的父路径为空（即 `"data.txt"` → `""`）。

# 路径规范化

- 要找真实父目录，需先做路径规范化。
- 有两种方式：
- 词法：用 `.lexically_normal()`；规范化过程：设路径为 `D:/..\sub\.\/path\..\file.txt`。`D:/..\sub\.\/path\..\file.txt` `D:\..\sub\.\/path\..\file.txt`

`D:\..\sub\.\path\..\file.txt` `D:\..\sub\path\..\file.txt`

`D:\..\sub\file.txt` `D:\sub\file.txt`
7. 适用于例如 `..\..\`。
1. 与 8. 空路径规范化后仍为空；非空但实质为空则规范化为 `.`。

# 路径规范化

- 依赖文件系统：`stdfs::canonical` / `weakly_canonical`；将路径规范化为唯一绝对路径。
- 路径先由 `stdfs::absolute(p)` 转为绝对路径；
- 再做词法规范化。

- `canonical` 会检查路径是否真实存在，而 `weakly_canonical` 只做规范化。
- 全局 API 的两种形式后文再述。

示例

末尾的 `/` 不会被去掉，即便规范化后亦然。`..` 只去掉尾随的 `/`。

# 路径规范化

- 注意：词法规范化的「物理」父路径在规范化结果仍含 `..` 时仍可能错误。
- 只有 `(weakly_)canonical` 才能保证物理父路径正确。

- 最后，可用 `current_path()` 获取或设置 CWD：

- 故当 `path` 为相对时，`absolute()` 本质上为 `current_path() / path`。

`cd`：改变当前目录

`"D:"`：切换到 D 盘当前目录。

# DOS 目录

`"C:"`：切换到 C 盘当前目录。

- 还值得注意的是，DOS 为每个驱动器维护独立的「当前目录」。
- 故 `C:` 与 `D:` 实际是相对路径，而 `C:\` 与 `D:\` 是绝对路径。
- Windows 将当前目录统一为单一 CWD 所知路径。
- 但 CMD 通过「奇怪的环境变量」假装它们仍存在。
- Windows 继承 DOS 行为，把 `C:`、`D:` 视为相对路径；但真实 CWD 只有一个。

其他非 CWD 盘符会返回根。

1：术语「相对化」（relativization）来自 Java `nio`。

# 路径相对化

- 与规范化相反，也可通过把绝对路径转为相对路径来「反规范化」。
- 更一般地：给定路径 `b`，如何变换成路径 `a` 且分量最短。
- 例如：`path{ "/a/d" }.relative("/a/b/c")` 会得到 `"../../d"`。
- 类似地，两种方式：
- 词法：用 `a.lexically_relative(b)`；过程：① 检查是否可能把 `b` 变换为 `a`。
- 若不可能（见下述条件），直接返回空路径。

例如 Windows 上不同盘符的两条路径。绝对路径 + 相对路径，无法仅靠词法变换。例如 Windows 上 `bar` 与 `/foo`，即无法在不知 CWD 时从 `/foo` `cd` 到 `bar`。

例如 Windows 上 UNC 路径，`\\.\C:\Test` 以 `C:\Test` 为相对路径。即 UNC 不参与词法相对化过程。

`a = D:\test\test.txt` `b = D:\test\test2\test.txt\..`

# 路径相对化

② 确定两条路径第一个不匹配的分量（类似 `std::mismatch`）。
- 设 `a` 剩余不匹配区间为 \([a_1, a_2)\)，`b` 为 \([b_1, b_2)\)；
- 上例中 \(a_1 = test.txt\)，\(b_1 = [test2, test.txt, ..]\)。
- 若无不匹配分量（即 \(a_1 = a_2\)，\(b_1 = b_2\)），返回 `path{ "." }`；
- 否则，设在 \([b_1, b_2)\) 中有 \(n\) 个 `..` 分量与 \(m\) 个非 `..`、非 `.`、非空分量。
- 上例中 \(n = 1, m = 2\)。
- 若 \(n > m\)（即词法规范化结果只有 `..`），返回空路径。
- 若 \(n = m\)（即词法规范化结果为 `.`），则：
- 若 \(a_1 = a_2\)，返回 `path{ "." }`；
- 否则返回 \([a_1, a_2)\)。
- 若 \(n < m\)，则返回路径：1. 重复 `m - n` 次 `".."`；2. 接上 \([a_1, a_2)\)。`result = ..\test.txt`

# 路径相对化

- 实际上，该算法在理论上可变换时也可能误报空路径。\(n = 1, m = 0\)
- 例如：

- 理论上本可以是 `..`。
- 若要始终正确的词法变换，需先做词法规范化。

- 第二种方式是依赖文件系统的 `stdfs::relative(a, b)`，总在文件系统意义上保证相对路径正确。
- 等价于对两个 `weakly_canonical` 路径做 `lexically_relative`。

# 路径近接（proximate）

- 最后还有近似（proximation）：「能相对化则相对化，否则返回原路径」。
- 效果上：

- `stdfs::proximate(a, b)` 也等价于对两个 `weakly_canonical` 做 `lexically_proximate`。
- 另：`relative` 与 `proximate` 未提供 `b` 时，会用当前目录。

# 路径组合

- 对路径 `./sub/path/file.txt`，本质上是层次分量的组合。
- C++ 提供两种工具组合分量。
1. 追加（append）：必要时用目录分隔符连接两个分量。
- 例如（Linux 上）：

- 然而存在大量边角情形… ① 子路径为绝对路径：C++ 选择覆盖（替换）左侧。

- 例如：

- 但这可能令人吃惊：

原因：`"/.fonts"` 是绝对路径。

# 路径组合

- Windows 上类 DOS 行为在子路径为相对时也会产生意外结果。② 单个盘符不插入分隔符；仍是相对路径。

③ 追加含不同盘符的相对路径会替换整条路径；

④ 追加同盘相对路径时，如同左侧为 CWD 般追加。

⑤ 将「有根目录无盘符」的相对路径追加到有盘符路径时，会保留左侧盘符。

# 路径组合

2. 拼接（concatenate）：如同直接拼接底层字符串；不额外插入分隔符。

这些重载意在模仿 `std::string::operator+=` 的重载。

- 奇怪的是没有 `operator+`；但通常此操作用于与字符串拼接，故可先对所有字符串用 `operator+`。
- 或只能用 `((stdfs::path{a} += b) += c)…` 或 `.native()` 再用 `std::basic_string` 的 `operator+`。

# 路径组合

- 注 1：由于结合性，`p / "a" / "b"` 合法而 `p /= "a" /= "b"` 非法。
- `p / "a" / "b"`  `((p / "a") / "b")`，而
- `p /= "a" /= "b"`  `(p /= ("a" /= "b"))`。
- 两个字面量之间不能 `/=`。
- 必须加许多括号；故用 `((stdfs::path{a} += b) += c)…`。
- 注 2：还有一些布尔观察函数检查存在性。
- `empty` 表示底层字符串为空。
- `has_xxx` 表示 `xxx` 是否非空。

# 路径迭代

- 作为多种分量的组合，路径也可按通用格式迭代（按分隔符分组）。
- 提供 `.begin()` 与 `.end()`，返回 `path` 常量迭代器。
- 只遍历根名、根目录与文件名。
- 例如：

解引用结果（即 `iterator::value_type`）是另一个 `path`。

# 路径迭代

- 看似双向迭代器，标准实际只要求为输入迭代器（input iterator）。
- 原因：C++20 之前，前向迭代器（forward iterator）有如下规定：

- 即：每个分量应有固定来源，使每次解引用都指向该来源。
- 这要求 `path` 存储分量容器，使任意 `path` 构造都很昂贵…
- libstdc++ 如此实现，故为双向。
- 然而 `path` 迭代很像对字符串按分隔符 `std::views::split`！
- 另一做法（libc++ 与 MS-STL）是在迭代器内缓存区间，只有使用迭代器时才开始解析。

Issue 2674：`path::iterator` 要求双向迭代器代价很高

# 路径迭代

- 因此标准改为：

- 使其只满足输入迭代器，故无法对某些 `<algorithm>` 函数使用。

- **但是**，此类要求已不属于 C++20 的 `bidirectional_iterator`！
- 故理论上应可使用约束算法，即 `std::ranges::xxx`。
- 好吧，问题更复杂…见作业讨论。
- 总之 libc++ 已为其添加 `iterator_concept`，而 MS-STL 不能。

# 路径修改

返回对 `*this` 的引用。

- 还有一些简单的非常量成员：
1. `.make_preferred()`：对原生格式与通用格式相同的路径，把分隔符转为首选分隔符。
- 例如：

2. `.remove_filename()`：去掉最后一个分量（若存在），使 `.has_filename()` 为 false。
- 故删除后路径为空或以分隔符结尾。

`"foo"` 也会被转为 `""`。

# 路径修改

3. `.replace_filename(const path& rep)`：等价于 1. `this->remove_filename()`；2. `(*this) /= rep`。
4. `.replace_extension(const path& rep = {})`：等价于下述代码：
- 例如：

# 路径

逐元素比较前，需先判断这些条件。

- 最后还有一些简单工具，仅列于此。例如 `"D:/Test"`
- 可比较（用 `<=>` / `==` 或 `.compare`）；按分量比较。`== "D://Test"`
- 可哈希（`std::hash` 或友元 `hash_value`）；按分量哈希。
- 用 `>>` / `<<` 输入输出；
- 对 `basic_iostream<CharT, Traits>`，等价于用 `std::quoted` 输入输出 `.string<CharT, Traits>()`，空格不会打断输入。
- 回顾：`quoted` 会转义引号与转义符，故 `\` 变为 `\\`。
- C++26 起可格式化（formattable）。

# 路径格式化

- 规定与 `.string()` 略有不同。

即 `char` 的编码；即明确规定在 UTF-8 字面量中，按指定执行字符集；Windows 上，编译器执行字符集中非法字符会转为 U+FFFD。

作为 C++26 特性，尚未实现，故「由实现定义」尚不明确（但很可能可用 `std::print` 打印）。

# 最后说明

- 注 1：C++17 还有 `.u8path()` 从 UTF-8 串构造，C++20 起已弃用。
- 原因：C++20 引入 `char8_t`，用模板区分 UTF-8 与 `char`，无需新方法。
- 同理，`.(generic_)u8string` 在 C++17 返回 `std::string`，C++20 返回 `std::u8string`。
- 注 2：要把 `path` 用作 `map` 的键，通常需先用 `canonical` 规范化。
- 原因：`path` 的比较与哈希对底层分量是词法的。
- 不规范化时，两条等价路径可能被当作两个键。
- Windows 上甚至可能需要对大小写不敏感路径做 `to_lower`。
- 更昂贵但恒正确的方式是用 `stdfs::equivalent` 比较，需系统调用检查两路径是否等同。后文再述。

# 补充

- 文件系统
- 路径操作
- 文件系统操作
- 概览
- 文件状态查询与目录迭代
- 修改操作

# 概览

- 本节多数函数与底层文件系统交互，位于全局命名空间 `stdfs::`。
- 对比之下，上一节的函数纯词法（因而更便宜），是 `stdfs::path` 的成员函数。
- 几乎每个函数提供两个版本：
1. 错误码版本：最后参数为 `std::error_code&` 返回文件系统错误（若仅文件系统错误则可能为 `noexcept`）；
2. 异常版本：抛出 `stdfs::filesystem_error` 表示错误。

- 原因：文件系统操作易产生 TOC/TOU（检查时间与使用时间）问题，预先检查无法防止错误。
- 因此难以预测错误是否在热路径，异常有时不合适。

# 概览

- 例如要写「若存在则 chmod」。
- 伪代码可为：

- 但文件系统跨进程，事件顺序可能是：
- 我们检查文件确实存在（TOC）；
- 另一进程删除该文件；
- 我们改文件权限，对象已不存在导致错误（TOU）。
- 多进程访问同一文件系统对象（竞态）时，具体行为由实现定义。

- 更糟的是，攻击者易利用 TOC/TOU。
- 著名例子是 `stdfs::remove_all`（Rust 里也有！），递归删除目录下所有文件但跳过符号链接（symbolic link）内的删除。
- 符号链接后文再述；一般可理解为指向另一目录的对象。
- 如此跳过可防止误删其他文件夹中的文件。
- 三个标准库最初都实现为：
1. 检查对象是否为符号链接；
2. 若不是，则递归删除其下文件。
- 假设黑客想删 `sensitive/` 但无权限；而系统上以高权限运行的程序周期性对无权限检查的 `recyclebin/` 调用 `stdfs::remove_all`。
1. 黑客先在 `recyclebin/` 建目录 `temp`；
2. `remove_all` 检查到不是符号链接；
3. 黑客删除 `temp` 并建符号链接 `temp` 指向 `sensitive/`。
4. `remove_all` 删除 `temp` 下所有文件，即删光 `sensitive`。黑客得逞！

# 概览

- 同理，用户代码也可能因 TOC/TOU 变得脆弱…
- 核心问题：「路径名」是可变属性；更稳健地指向同一文件系统对象的方式应是某种句柄（handle）。
- 这在 P1883（底层文件 I/O 库）中有提案；
- 但这会使文件系统 API 很难用。-------- 回到标准 ---------
- `stdfs::filesystem_error` 的 API 很简单：
- 此处不深究 `std::error_code`；详见作业。

# 补充

- 文件系统
- 路径操作
- 文件系统操作
- 概览
- 文件状态查询与目录迭代
- 修改操作

# 文件状态

- 文件具有以下属性：
- 名称，用于路径中；
- 类型；C++ 使用 POSIX 约定，但允许各系统略有
- 权限；定制（如 Windows）。
- 大小；
- 最后修改时间。
- POSIX 还规定下列文件类型（体现在 `stdfs::file_type`）：
- 实现可添加新类型（如 MS-STL 中的 junction）。

# 文件类型

这些类型各是什么？

# 链接

- 文件系统中有多种链接。
- 本质上，链接就是重定向到其他对象的对象；
- 不同链接只是在不同层次上重定向。
- 文件系统处理对象的简单示意：

`/home/dir1/file.txt` `/home/dir1` `/home/home/dir2/file1.txt` 文件系统操作树。`/home/dir2` 如同 `shared_ptr`；`/home/dir2/file2.txt` 当指向节点的指针都消失时，节点也会… 文件系统对象节点被释放。

# 硬链接

1. 硬链接（hard link）：如同给底层数据节点增加引用计数。
- 文件系统无法区分硬链接与原对象（故没有叫「硬链接」的类型！）。
- 删除 `/home/dir2/file2.txt` 时，内容未必真删；`/home/dir1/hardlink.txt` 仍保留该节点。
- 对象的硬链接数可用 `stdfs::hard_link_count(p)` 查询。`/home/dir1/hardlink.txt`

`/home/dir1/file.txt` `/home/dir1` `/home/home/dir2/file1.txt` 向 `/home/dir2` 添加硬链接 `/home/dir2/file2.txt`；`/home/dir2/file2.txt`

…

# 硬链接

- 多数文件系统不易允许用户对目录建硬链接。
- 核心原因：文件系统通常假定是树形。
- 但若在某目录的后代中为该目录建硬链接，会在图中成环，不再是树。
- 某些系统程序遍历文件系统时无特殊检查；环会导致无限循环。
- Linux：`ln hd.txt a.txt`；不允许对目录硬链接。
- MacOS：`ln hd.txt a.txt`；无目录的原生命令但可用 POSIX `link()`；
- MacOS 的 `link` 实现会检查新目录硬链接是否成环；若不会则允许（但可能需 root）。
- Windows：`mklink /H hd.txt a.txt`；不允许对目录硬链接。

# 硬链接

- 注 1：目录的 `hard_link_count` 由实现定义。
- Unix：`"."` 与 `".."` 都视为硬链接；故新目录 `hard_link_count` 为 2，且父目录 `hard_link_count` 自增。
- Windows：恒返回 1。
- 返回类型：`uintmax_t`。
- 注 2：硬链接还有其他限制：
1. 部分文件系统不支持硬链接（尤其 FAT；多数 U 盘使用它）。
2. 硬链接不能跨文件系统，因不同文件系统数据结构可能不同。硬链接仅存于同一文件系统内。
- 特别在 Windows 上须在同一卷（通常即盘符）。
3. 部分文件系统对每文件的硬链接数有限制。

# 符号链接

2. 符号链接（symbolic link，软链接）：如同指向文件系统表项（filesystem entry）。
- 或可视作对底层节点的 `weak_ptr`。
- 当真实表项 `/home/dir2/file2.txt` 被删，内容也会删，尽管符号链接 `/home/dir1/softlink.txt` 仍存在。
- 之后对软链接的重定向操作（如文件 I/O）会失败。

`/home/dir1/softlink.txt`

`/home/dir1/file.txt` 向 `/home/dir1` `/home/dir2/file2.txt` 添加符号链接；`/home/home/dir2/file1.txt` `/home/dir2` `/home/dir2/file2.txt`

…

文件系统能力可查 MS 文档。注意 UDF 广泛用于光盘。

# 符号链接

- 文件系统将其视为独立文件类型，但多数原生 API 会自动重定向到真实表项。
- 即：用户（而非文件系统！）无法区分符号链接与普通对象，除非使用少数特殊函数。
- 因此文件系统允许对目录建符号链接。
- Unix：`ln -s sym a`；
- Windows：`mklink /D sym a`；特别在 Windows 上创建符号链接常需管理员权限。

- 注：部分文件系统不支持软链接（尤其 FAT）。
- 但因指向文件系统表项，可跨文件系统。
- 故可在 NTFS 上建指向 FAT 的软链接，因 NTFS 支持（反之则未必）。

# 联接点（Junction）

3. 对非 root 权限，Windows 允许用一种新类型「联接点」（junction）链接目录。
- `mklink /J junc a`。
- 与符号链接类似，真实节点删除后联接点仍存在但失效。
- 联接点虽可跨文件系统，但只允许链接本机目录。
- 即不支持网络路径与 UNC。

- 多数 `stdfs` API 会跟随上述所有链接并解析到最终真实表项。
- 例如：`stdfs::exists`；`stdfs::absolute`；因而 `stdfs::canonical`；`stdfs::relative`；`stdfs::proximate`。

Windows 上各类链接细节可参考相关博客。

# 快捷方式

- 另：Windows 上还有「快捷方式」（.lnk），实际是「用户态符号链接」。
- 它是带 `.lnk` 扩展名的文件；文件系统只当作普通文件（也确实如此）。
- 其内容是真实文件系统对象路径、启动方式等。
- 猜猜：点击快捷方式时如何重定向到真实表项？
- 对，Windows 资源管理器（Explorer）界面帮你完成！
- 资源管理器程序自动把点击当作点击真实表项；不做特殊处理的其他程序无法重定向。
- 对比之下，链接由文件系统自动重定向，即便程序不做特殊处理。

# 字符设备与块设备文件

- 插入 USB（鼠标、磁盘等）时，操作系统如何识别与设备交互的方式？
- 通过驱动程序（driver），若你学过嵌入式系统。
- 要支持新外设，可编写驱动，提供一组函数（如 `open`、`write` 等）让 OS 知道如何与之交互。
- 最终驱动需暴露设备文件（device file），使用户能通过 OS API 打开、写入等。
- 字符文件与块文件即此类设备文件。
- 对它们读/写时，如同从驱动取数/向驱动送数，从而与设备交互成功。
- 字符文件表示「I/O 直接传递、不经缓冲」；例如终端：`/dev/tty`
- 块文件表示「缓冲 I/O，在合适时机再传递」。例如 SSD：`/dev/sda`

Windows 也支持字符文件（如 `CON`），但 MS-STL 对这两种类型恒返回 false，因检测代价高。

# 管道（FIFO）

- FIFO 即命名管道（named pipe）。
- Linux 上可用 `|` 把一个程序输出作为另一程序输入（如 `cat data.txt | grep "info"`）。
- 管道让数据驻留内存，而不写入真实文件（从而真实存储）以实现进程间通信。
- 有时创建匿名管道并传给其他程序不便。
- 可在文件系统中创建命名管道，一些进程写、另一些读（仍在内存中，不落到真实存储）。
- 例如命令行：`mkfifo test`；`cat data.txt > test &`；`grep "info" test`；或用 POSIX API `mkfifo`。
- 故 `test` 仅为 fifo 类型，在 POSIX 系统中有特殊解释。

填充网络位置。

# 套接字（Socket）

- 类似地，有时用网络套接字跨进程通信不便。
- 可创建套接字文件（Unix domain socket，UDS）。
- 我们知道如何在 Linux 上创建网络服务器套接字。
- 对 UDS，只需改标志与位置为本地套接字文件：

`bind` 之后，进程会在 CWD 创建 `sockfile.sock`，文件类型为 socket。

- 但 `bind` 要求地址未被使用，故套接字文件在 `bind` 前不应存在（故多为临时文件）。

# FIFO 与套接字（Socket）

- 注 1：fifo 与 socket 的区别：
1. 套接字文件可用 `send`/`recv`，与网络套接字一样；fifo 只能用 `write`/`read`。
2. 套接字可用数据报而非字节流（`SOCK_DGRAM`），fifo 只用字节流。
3. 套接字双向，服务端与客户端可互发；fifo 单向，发送端只发、接收端只收。
4. 通常 fifo 用于两端用户，socket 可用于多客户端。
- 注 2：Windows 自 Windows 2000 Professional 起支持 fifo，自 Windows 10 17063（2017/12）起支持 UDS。
- 但 MS-STL 对 `is_fifo` 与 `is_socket` 恒返回 false。

原因：UDS 在 Windows 上不像 UNIX 那样创建真实文件。

# 权限

- POSIX 上权限分三级：用户（user）/ 组（group）/ 其他（all）。
- 每级可有读/写/执行权。
- Linux bash 中可用 `chmod` 改权限；如 `chmod 764` 表示所有者可读写执行，当前组可读写，其他人只读。
- 因每级可用三位表示，C++ 中可用八进制字面量表示权限。
- 如字面量 `0764`，前导 `0` 表示八进制。
- 但 Windows 权限系统（访问控制列表 ACL, Access Control List）与 POSIX 不兼容，故用简化 DOS 权限：
1. 所有用户可读写执行（777）；
2. 所有用户可读执行（555）。

# 权限

- POSIX（再次强调，非 Windows）还有三种特殊权限：setuid、setgid 与粘滞位（sticky bit）。
- setuid：执行二进制时使用所有者用户 ID（从而可执行仅所有者可执行的代码）。
- 典型例：`/bin/passwd`，会写 `/etc/shadow`。用户可用 `/bin/passwd` 改密码，但无权写 `/etc/shadow`（否则可改任意人密码！）。
- 故 `/bin/passwd` 带 setuid，即以 root 权限写 `/etc/shadow`。
- 因 `/bin/passwd` 控制其访问，仍安全。
- setgid：执行二进制时使用所有者组 ID。
- 典型例：`/usr/bin/wall`，需要组权限写他人 tty。

# 权限

- sticky bit：通常表示「作用于目录时，即便对目录有写权限，也只有文件所有者能删自己的文件」。
- 典型例：`/tmp`；所有用户对 `/tmp` 有写权限以放临时文件，之后可能删自己的缓存。
- 但这也允许用户删他人文件，可能随机搞挂他人应用。
- 方案 1：每人用如 `chmod 007` 保护自己的 `/tmp` 文件。
- 太麻烦…
- 方案 2：对 `/tmp` 设 sticky bit！
- 则只有文件所有者能删自己的文件。

- C++ 用枚举定义这些权限说明。

# 权限

- 权限值定义为 `enum class perms`，枚举项如下：
- 并重载了位运算符。

# 文件状态查询

- 要测试文件类型，可用这些 API：
- 参数：`(const path& p[, error_code])`。

其他文件：存在但不是普通文件 / 目录 / 符号链接。

# 文件状态查询

- 更高效的方式是一次查询类型并缓存，之后每次测试只是廉价整数比较。
- 用 `stdfs::status` 即可！
- 例如：

出处：C++17 the Complete Guide, Nicolai M. Josuttis.

# 文件状态查询

- `status` 本质上返回 `file_status`，`.type()` → `file_type`，`.permissions()` → `perms`：
- 另一例：

注：`is_xxx`（如 `is_directory`）也可接受 `file_status`，为 `noexcept`，因只是整数比较。

# 文件状态查询

- 但若对符号链接使用 `status`：

- 哎呀，类型不是 symlink，而是 directory！
- 原因：文件系统 API 会自动跟随符号链接。
- 要查询浅层状态而非跟随链接，应使用 `symlink_status`：

注 1：`is_symlink(p[, ec])` 内部用 `symlink_status`。注 2：硬链接仍无法区分。

此处将 `Dir2` 建为 junction，MS-STL 列为新枚举项，故 `is_symlink` 仍为 false（虽可用 `symlink_status` 查询）。

MS-STL：返回 0；libc++ 与 libstdc++：抛错（「is a directory」）。

# 文件状态查询

- 最后两种状态是大小与最后修改时间，也可通过路径查询：
- `stdfs::file_size(p[, ec])` → `std::uintmax_t`；
- 对目录的结果由实现定义。
- `stdfs::last_write_time(p[, ec])` → `stdfs::file_time_type`；
- `std::chrono` 中的具体工具后文介绍。
- 注：除类型外，其他状态可直接修改。
- `stdfs::resize_file(p, newSize[, ec])`；
- 若 `newSize` 大于当前大小，新空间填 0；否则截断文件。
- 注：在支持稀疏文件（sparse file）的文件系统上，只要「填 0 的新空间」未真正写入，更大的文件未必减少可用存储。

# 文件状态查询

- `stdfs::last_write_time(p, newTime[, ec])`；
- 但文件系统粒度可能与程序时间不同，故可能有舍入误差。
- `stdfs::permissions(p, newPerm[, opt[, ec]])`；
- `opt` 为 `enum class stdfs::perm_options`，同样支持位运算。

- 默认 `opt` 为 `replace`。

# 目录项

- 嗯，有没有办法一次查完所有属性再从缓存取？
- 用 `stdfs::directory_entry`！由 `path` 构造，
- 例如：若属性存在，构造函数会查询全部。

注：libstdc++ 中需 `#include <chrono>` 才能输出时间（C++20，后述）。

# 目录项

- 它几乎包含前述所有查询方法：
- 还有一些工具（**仅**作用于底层路径！）：
- 可比较
- `operator<<`

`.is_symlink` 用 `symlink_status` 而其他用 `status`。故 `.is_symlink && .is_directory` 可同时为 true。

# 目录项

- 还有一些简单修改器：

即 `path.replace_filename(…)` 与 `.refresh()`。即通过文件系统调用重新查询属性。
- 调用这些方法**不会**在文件系统中重命名底层文件，只是重新查询！
- 实际上，`directory_entry` 是 `directory_iterator` 的值类型，可用于遍历路径。

1. 除 `operator++` 外，迭代器还定义 `.increment(ec)` 接受错误码。

# 目录迭代

2. 本质上，`begin` 返回自身副本，`end` 返回默认构造的迭代器。

- 有友元 `begin` 与 `end`，自然构成范围，可直接遍历：路径会包含根（构造函数传入）作为前缀。

- 构造函数还可加第二参数 `stdfs::directory_options` 控制迭代行为：同样是可位运算的限定枚举。

# 目录迭代

- 要递归遍历目录，可用 `recursive_directory_iterator`。
- 它会以由实现定义的方式（通常 DFS）遍历所有表项。
- 例如：

`Dir2` 是指向 `Dir1` 的 junction，且我们跟随符号链接，故 `Dir1` 内容会再打印一遍。

# 目录迭代

- 该迭代器还有更多成员方法：

→ `directory_options` 观察：→ `int`（从 0 起）

→ `bool`

修改器：

`pop([ec])`：丢弃子树后续迭代，回到上一层的下一项。该操作会使先前所有副本失效；当 `.depth() == 0` 时迭代器变为 `end()`。`disable_recursion_pending`：当前表项为目录时，下一次 `++` 不进入目录。随后 `recursion_pending` 为 false。自增会把 `recursion_pending` 重置为 true。

# 目录迭代

`pop`

`depth == 0`

`disable_recursion_pending` `depth == 1` 然后 `++`。

`depth == 2`

# 目录迭代

- 注 1：它们的 `operator*` 返回 `const&`，故不能不拷贝就修改 `directory_entry`。
- 注 2：它们是输入迭代器。
- 回顾：输入迭代器不是前向迭代器，因是单次通行（one-pass）；复制后遍历副本可能得到不同结果。
- 这里类似，目录内容可能被其他进程改变，故非多通行（multi-pass）。
- 实际更复杂，详见作业。

- 注 3：在创建 `(recursive_)directory_iterator{ "A" }` 之后若目录 A 中新增文件/目录，是否会被迭代由实现指定。

# 最后说明

- 还有一些次要只读方法：
1. 路径操作：
- `current_path()` → `p` 表示 CWD；
- `temp_directory_path()` → `p`（Linux 上如 `/tmp`）；
- `equivalent(p1, p2)` → `bool` 判断两路径是否实质相同（跟随符号链接）。
- `read_symlink(p1)` → `p2` 将符号链接路径转为目标路径（`p1` 非符号链接则错误）。
2. 文件系统操作：
- `is_empty(p)` → `bool`，`p` 为空文件或空目录时为 true；
- `space(p)` → `space_info`，确定 `p` 所在挂载文件系统的空间。
- Linux 上可用 `df -h` 查看路径所在文件系统：
- Windows 上每个驱动器视为一个挂载文件系统。

# 最后说明

- cppreference 上的示例程序：

# 补充

- 文件系统
- 路径操作
- 文件系统操作
- 概览
- 文件状态查询与目录迭代
- 修改操作

# 创建

- 修改操作分四类：
1. 创建：

① 目录：
- `create_directory(p[, existing_p[, ec]])` → `bool`；
- 给定 `existing_p` 时，创建的 `p` 会从另一目录 `existing_p` 复制 OS 相关属性（Windows 不做任何事）。`p` 的父目录须已存在（即只创建单层目录）。
- `create_directories(p[, ec])` → `bool`；
- 创建路径中所有不存在的目录元素。故当 `p` 的父不存在时也会创建。

若目录已存在（不算错误）或发生错误则返回 false。

注意：路径符号链接所指的是**从符号链接自身视角**。（硬链接不如此。）

# 创建

② 链接：
- `create_hard_link(original_path, link_path[, ec])`；
- `create_symlink(original_path, link_path[, ec])`；
- `create_directory_symlink(original_path, link_path[, ec])`；
- 某些 OS 对普通文件与目录的符号链接需不同 API，故对目录推荐 `create_directory_symlink`。POSIX 不区分二者。
- 注：这两法只建软链接，Windows 上常需管理员权限。且标准库无法在 Windows 上创建 junction。③ 普通文件：不在 `stdfs` 中；可用 `std::ofstream`。示例：

# 复制

2. 复制：

- 复制支持 `copy_options`（同样是可位运算的限定枚举）配置行为。
- `copy(from, to[, opt[, ec]])`：按 `opt` 复制。
- `copy_file(from, to[, opt[, ec]])`：按 `opt` 复制普通文件。
- 可比 `copy` 更便宜、检查更少，且相比 `if(!is_directory(a)) copy(a, b)` 不引入额外 TOC/TOU。
- 错误条件（跟随符号链接之后，不含权限等 OS 错误）：
- `!is_regular_file(from)`；或
- `exists(to) && (equivalent(from, to) || !is_regular_file(to) || opt == copy_options::none)`。

`copy` 的具体行为（含错误条件）过长，此处不表。详见 cppreference。

# 复制选项枚举

（也适用于对文件的 `copy`。）

# 复制

- `copy_symlink(from, to[, ec])`：把符号链接复制到另一位置，而非像 `copy` 那样跟随链接复制内容。
- 等价于 `create_(directory_)symlink(read_symlink(from), to)`。
- 注：`copy_symlink` 在 Windows 上不复制 junction；只能用 `copy(from, to, copy_options::copy_symlinks)`。
- `copy` 示例：

# 删除与重命名

3. 删除：
- `remove(p[, ec])` → `bool`：要求 `p` 为普通文件或空目录，不跟随符号链接；删除成功返回 true。
- `remove_all(p[, ec])` → `uintmax_t`：删除 `p` 下所有内容（含 `p`），不跟随符号链接；返回删除项数（错误时为 -1）。
4. 重命名：
- `rename(old_p, new_p[, ec])`：
- 若 `new_p` 是 `old_p` 的硬链接，则无操作；
- 若 `old_p` 为普通文件：若 `new_p` 存在且为普通文件则删除 `new_p`，再重命名为 `new_p`。
- 若 `old_p` 为目录：若 `new_p` 存在且为目录则删除 `new_p`，再重命名为 `new_p`。`new_p` 的父须存在且 `new_p` 不应以分隔符结尾。
- 注：其他进程不应观察到此类删除。

# 补充与总结（Supplementary and Summary）

Chrono

所有工具定义在 `std::chrono`；为简洁，`namespace stdc = std::chrono;`。

# 概览

- 我们知道多数物理量需要单位。
- 1 分钟 = 60 秒；1 秒 = 1000 毫秒；等。
- 其比例可用有理数表示。
- 在 `<chrono>` 库中，时间单位也用此类比例。

`intXX`：至少 XX 位的有符号整数。

# 补充

- Chrono
- 编译期有理数
- 时间
- 日期与时区

`num`：分子（numerator） `den`：分母（denominator）

# 有理数

- 有理数即可表为两整数之比的实数。
- 其运算是精确的，即不产生舍入误差。
- 那是否只需 `struct R { int num; int den; }`？
- 并不完全…
1. `3/2`、`6/4`… 本质相同，需要约分。
2. 非类型模板参数（NTTP, non-type template parameter）类 C++20 才引入，而 `<ratio>` 自 C++11 就有。
- 故引入新类型 `std::ratio<Num, Den>`。
- 约分后定义量：

例如 `std::ratio<6, 4>` 有 `num = 3`, `den = 2`, `type = std::ratio<3, 2>`。

- 在类型上定义运算：

这些运算会自动约分结果类型（故不必显式写 `::type`）。

SI 单位别名：

# 补充

- Chrono
- 编译期有理数
- 时间
- 时长（Duration）
- 时间点（Time point）
- 日期与时区

使用这些字面量需 `using namespace std::literals` 或

# 时长

`std::chrono_literals`。

- 物理学区分时刻（time point）与时间（duration）。
- 时间是两时刻之间的间隔。
- C++ 中用 `stdc::duration` 表示该间隔：

`Rep`：如何表示数值，如 `long long` 或 `double`。`Period`：时间单位（相对秒），如 `std::milli`（`std::ratio<1, 1000>`）表示毫秒。默认即秒。
- 它本质上只存一个 `Rep` 类型的数。
- 用 `.count()` 取得（按值，非引用）。
- 还提供若干用户定义字面量，相应调整 `Period`：

`auto` 为 `stdc::milliseconds`，即 `stdc::duration<intXX, std::milli>`。猜猜 `auto m = 10.0ms` 如何？

# 时长

- 也可用 `stdc::duration_cast` 在不同表示与周期间转换：

- 例如：

- 为保持数值精度，会先取两类型的「最大」公共类型。
- 最后才转换到 `ToDuration` 的表示，即所有计算完成之后。

*：`decay` 之后，若无特化。

# 公共类型

*
- 基本上，`std::common_type<A, B>` 就是条件运算符的类型（即 `cond ? std::declval<A>() : std::declval<B>()`）。
- 详细规则很复杂；一般选能隐式从另一方转换来的 `A` 或 `B`。
- 若都不能或都能，则编译错误。
- 对算术类型，因都可隐式转换，按 usual arithmetic conversions。
- 即：浮点优先于整数；位宽大的优先。
- `std::common_type<T0, T1, …>` 迭代求公共类型。
- 即 `std::common_type<std::common_type<T0, T1>, …>`。
- 故两整数 `duration_cast` 间计算常用 `intmax_t`。

# 时长

- 构造函数：

即浮点不能用于构造 `stdc::duration<Int>`（保证无损）。

即要么 `duration` 用浮点表示，要么 `Period2` 可被 `Period` 整除（无损转换）。

# 时长

- 还定义许多算术运算。
- 注 1：`operator++/--` 仅当底层表示支持时可用。
- 注 2：注意隐式转换。
- 例如：

- 输出：
- 原因：这些类型是整数！
- 故 `1.5f` 隐式转为整数 `1`。
- 类似：`auto m = 10ms; m *= 1.5f;`
- 解决：让表示为浮点。
- 如 `auto m = 10.0ms`，或显式指定表示类型。

# 时长

- 注 3：其他运算符用两 `duration` 的公共类型。
- 特化为底层表示的公共类型。

# 时长

- 注 4：可比较：
- 也会先转到公共类型再比较 `.count()`。

- 注 5：C++17 增加若干简单数学函数：
- 例如：

# 时长

- C++20 增加更多工具：
1. `days`、`weeks`、`months`、`years`：因每月每年长度不同，使用平均值。
- 精确：`1 days = 86400s`，`1 weeks = 7 days`；
- 平均：`1 years = 365.2425 days`，`1 months = years/12`。
- 还有取整后的整数别名：

- 注：它们**没有**字面量；`y` 与 `d` 用于表示年与日，属于日期（时间点）而非时长。
- `years` 与 `days` 是时长，不是 `year` 与 `day`。

另：因 `months` 不是 `days` 的整数倍，`days + months` 不会得到以天为单位的时长；公共类型单位是 54 秒。`days + years` 是 216 秒。

# 时长

2. I/O 与格式化：① `operator<<`：输出 `.count()` 并附加单位，取决于 `Period` 类型。
- 对非特殊类型：

② `formatter`：默认格式等价于 `operator<<`。

- 具体格式说明符 `chrono-spec` 与 C 的 `strftime` 及时间 locale 相同。
- 相当复杂，此处仅简介；详见手册。
- 先给简单例：

历史原因：便于 shell 输入。

`%O?`：locale 的替代数字符号。

`%E?`：locale 的替代纪元表示（纪元/年号纪年法）。

# 时长格式

- 注 1：格式中的「精度」秒数并非格式精度。
- 如上例，`0.5` 秒未输出。
- 但若改为毫秒…
- 哇，现在有三位小数。
- 故设周期为 \(P\)；若存在整数 \(K, M\) 满足 \(P = K \times 10^M\)，则精度定义为 \(\max(-M, 0)\)。
- 例如 \(1 = 1 \times 10^0\)，精度为 0，故浮点以整数输出。
- \(1/1000 = 1 \times 10^{-3}\)，精度为 3。
- 否则精度为 6。
- 例如 \(1/3\) 无法表成上述形式，用 6。

`stdc::hh_mm_ss` 细节后述。

# 时长格式

- 若要更复杂格式，可用 `stdc::hh_mm_ss`。
- 它把 `duration` 等价拆成时分秒的整数，带子秒表示小数。
- 例如：

- 本质上也是精度定义所在：

# 时长格式

- 注 2：目前格式化器中精度说明对 chrono 的效果尚不明确（如 `{:.3%Q}` 合法但 `.3` 无效）。
- 问过 chrono 提案作者后，我认为这是缺陷，日后可能以 DR 修正。
- 注 3：12 小时制示例（Windows）：

回顾：`L` 用于 `Print` 函数不接受 locale 的情况（需改全局 locale），故显式用 `format`。

部分 locale 可能不尊重 `%r`（平台相关）。

Windows locale 通常不处理 `%O` 与 `%E`。

# 时长

- 为保证 12 小时制，C++20 还增加判断与转换函数。
- `makeXX` 对越界时间的结果由实现指定。\(h \in [0,11]\) \(h \in [12,23]\) `result` \(\in [1,12]\)（0 为 12 a.m.）

③ `from_stream`：按格式扫描字符串得到时间。
- 即格式化的逆，很像 `std::scan`（`scan` 尚未入标准）。

最后两参数涉及时区，后述。

除 `%Q/%q` 外，接受与 `format` 相同的 chrono 说明。

# 时长输入

- 例如：

- 读取失败会设流 `failbit` 且不修改传入的时间。
- 还引入操纵符 `stdc::parse` 辅助：

# 时长

3. `stdc::hh_mm_ss<Duration>` 细节：

转回 `duration`

等价于等价 `duration` 的 `"{:L%T}"`。

等价于等价 `duration` 的 `formatter`；默认格式仍同 `operator<<`（即等价 `"{:L%T}"`）。

# 时长

- 最后一些较少用说明：
- 注 1：可特化 `stdc::treating_as_floating_point` 与 `stdc::duration_values` 针对底层表示。
- `treating_as_floating_point`：顾名思义。
- `duration_values`：需定义三个函数。
- 它们会作为 `stdc::duration` 的静态成员暴露。
- 默认：`Rep(0)`；`std::numeric_limits<Rep>::lowest()`；`std::numeric_limits<Rep>::max()`。
- 注 2：C++26 起可哈希，如同哈希底层表示。
- 注 3：若干成员类型别名：

# 补充

- Chrono
- 编译期有理数
- 时间
- 时长
- 时间点
- 日期与时区

# 时间点

- 本质上，时间点是相对参考原点的量。
- 即：可定义为纪元（epoch）加一个时长。
- C++ 中纪元定义在时钟（clock）里，时钟也有滴答分辨率（仍为 `duration`）。

自 C++20 起

# 时间点

- 时钟差异后述；先看时间点。
- 构造函数：

- 可查相对纪元的时长：

# 时间点

- 以及有限算术：
- `operator+=/-=` `duration`；
- `operator+/-` `duration` 得时间点；或 `operator-` 另一时间点得 `duration`。`duration` 类型仍为公共类型。

- `time_point` 的公共类型也有特化，同 `duration`。
- `operator++/--`；
- 可比较；
- 以及：用 `ToDuration` 而非时间点作类型模板。（如 `std::chrono::floor<std::chrono::days>(SysTime)`

`std::chrono::floor<std::chrono::time_point<std::system_clock, std::chrono::days>>(SysTime)`）。

# 时间点

- 显式转换：必须是同一时钟。
- 如同对底层 `duration` 做 `duration_cast`。
- 还有一些次要成员：
- `min/max()`：返回 `.time_since_epoch()` 与对应 `duration` 的 `min/max()` 相同的时间点。
- C++26 起可哈希，如同哈希 `duration`。
- 以及成员类型：

# 时间点

- 例如：

默认情况下 `time_point` 不可打印、不可格式化，除特化外；`steady_clock` 无特化故只能打印其 `duration`。

- 故粗略说，`time_point` 是带有限工具的 `duration`。

# 时钟

- 多数时钟定义静态方法 `now()` 取当前时间点。
- 故简单计时段为：

- 各时钟有何区别？
- 纪元（epoch）；
- 滴答分辨率，由实现定义；
- 是否稳定（steady）；
- 若时间可打印，如何处理闰秒（leap second）。
- 当前闰秒总为正，因地球自转变慢。
- 注：闰秒可能在 2035 年后取消（我不确定），但历史闰秒仍存在。

# 时钟

- Steady 表示外部调整不能影响时间。
- 例如：`auto t1 = now(); auto t2 = now();` 若用户改系统时钟，可能出现 `t1 > t2`。
- 只有 `stdc::steady_clock` 保证单调稳定。
- `stdc::system_clock`：操作系统时钟。
- 纪元：由实现定义；C++20 起规定为 Unix Time。
- 即 1970/1/1 00:00:00 UTC（周四）
- 闰秒：每分钟至多 60 秒；若发生正闰秒，则前一秒变长。
- 故每秒不均匀分配时间（后文示例）。
- 注：可从 `stdc::system_clock` 得到 C 的 `time_t`（此处不表）。

# 时钟

- `stdc::steady_clock`：滴答间隔恒定的单调时钟。
- 纪元：由实现定义；
- Steady：是；
- 不可打印，每秒均匀分配时间。

- `stdc::high_resolution_clock`：分辨率最小的时钟。
- 当前实现中，它是 `steady_clock` 或 `system_clock` 的别名之一。

- 这三个时钟在 C++11；C++20 起引入新时钟。

# 时钟

- `stdc::utc_clock`：协调世界时（UTC, Coordinated Universal Time），亦称格林尼治平均时间（GMT, Greenwich Mean Time）。
- 即日常生活中使用的时间（按时区偏移后）。
- 例如北京时间即 UTC/GMT+8。
- 纪元：Unix Time，即 1970/1/1 00:00:00 UTC（周四）。
- 注：官方 UTC 纪元为 1972/1/1，`utc_clock` 未采用。
- 闰秒：一分钟可有 59 或 61 秒。
- 故每秒均匀分配时间。
- 例如 2017/1/1 北京时间有 07:59:60。
- 可将 `utc_clock` 转为 `system_clock` 以取消闰秒处理，或反之。
- `sys_time` 知道闰秒（因内部 `duration` 增长），但不告诉用户。故 `from_sys` 可得到含闰秒的 `utc_time`。

# 时钟

- `stdc::gps_clock`：GPS 使用的时间。
- 纪元：1980/1/6 00:00:00 UTC。
- 闰秒：每分钟恰 60 秒；若发生正闰秒，则视为下一分钟的下一秒。
- 故每秒均匀分配时间。
- 截至 2026 年，GPS 时钟比 UTC 快 18 秒。
- `stdc::tai_clock`：国际原子时（TAI, International Atomic Time）。
- 与 GPS 时间相同，除纪元外（GPS 也用原子钟）。
- 纪元：1958/1/1 00:00:00 TAI（1957/12/31 23:59:50 UTC）。
- 截至 2026 年，TAI 比 UTC 快 37 秒。

- 可在 `gps_time`/`tai_time` 与 `utc_time` 间互转。

# 时钟

- 例如：

（细节后述）

# 时钟

- `stdc::file_clock`：文件系统用的时间（最后修改时间）。
- 完全由实现定义的 `TrivialClock`；C++20 增加转换器与格式化等工具。

- 之前（C++17）不易打印其时间点。
- 只能依赖 `time_t`，并容忍两个 `now()` 之间的误差。
- 最后，若只想表示「纯时间点」而无任何参考，可用 `stdc::local_t` 作时钟。
- 如高中物理…
- 当时区参考未定时，它也充当伪时钟，后文时区部分会示。

# 时钟

- C++20 增加的更多工具：
1. 类型别名：

# 时钟

2. 时间点 I/O：除 `stdc::steady_clock` / `high_resolution_clock` / `local_t` 外。示例：日语
- `formatter`：可用 `duration` 中除 `%Q/%q` 外的所有格式（因仍保留纪元）表示一日内时间；还提供更多日期格式：2011/9/25 Windows Linux

Windows Linux

基于周的年类似大学教学周；例如第 17、18 周考试。

2011/9/25 在 2011 年第 38/39 周（取决于如何计数），2011 年第 268 天。

特别说明：`operator<<(os, sys_time)` 还有特殊约束（要求整数表示、单位不长于 1 天）：

# 时钟

- 不同时钟有不同缩写，可用 `%Z` 打印：
- 系统时钟 / UTC 时钟 / 文件时钟：`UTC`；
- TAI 时钟：`TAI`；
- GPS 时钟：`GPS`；
- 亦可用 `%z`/`Oz`/`Ez` 打印时区偏移，后述。
- 整个时间点也可用 `%c` 打印：

- `operator<<` 等价于 `{:L%F %T}`，即年-月-日 时:分:秒。
- `operator<<` 为 `sys_days` 增加重载，只打印 `{:L%F}`。
- 例如：

是，希腊语在 Windows 上有完整表，差异仅在 `%O`、`%E` 与 locale 相关输出；Linux 上 2011/9/25 与 2011/9/25 有两套名称。我们用 Linux 上的 10:30:59。

# 时钟

- `from_stream` 接受类似格式（有时大小写不敏感，并增加若干 `%N_` 表示「至多 N 个字符」），故不再重复。
- 仍可用 `parse` 作操纵符。
3. 统一时钟转换：C++20 引入多种时钟，需要转换器。
- 例如可用 `stdc::from_sys`、`stdc::to_utc` 等。
- 但不直接定义任意两时钟间的转换。
- 可用 `stdc::clock_cast`！
- 如前所示：

- 原则：能直接转则直接转，否则以 UTC 与系统时钟为枢纽。

# 时钟

- 若两类型都能从/到 UTC 或系统时钟转换：
- 则 `to_sys` 再 `from_sys`，或 `to_utc` 再 `from_utc`。
- 若两种方式都合法，则二义性编译错误。
- 若源类型可转 UTC 且目标类型可从系统时钟转（或反之，`to_sys` 与 `from_utc`）：
- 则 `to_utc`，UTC 时钟再经 UTC 的 `to_sys` 转到系统时钟，再 `from_sys`；
- 或 `to_sys`，系统时钟经 UTC 的 `from_sys` 到 UTC 时钟，再 `from_utc`；
- 若两种方式都合法，则二义性编译错误。

- 底层转换依赖 `stdc::clock_time_conversion`，已对 UTC 时钟与系统时钟特化。
- 此处不细表。必要时查手册。

# 时钟

- 最后说明：
1. **不要**用非稳定时钟等待事件。
- 原因：非单调会使等待远超预期。
- 例如：`std::condition_variable::wait_for/until()`；
- 起初 libstdc++ 用 `system_clock` 实现，故若外部用户调系统时钟，`wait_for(10s)` 可等 10 分钟。
- 这在 gcc10 中通过 glibc 的 `pthread_cond_clockwait` 并用 `steady_clock` 修复。
2. 给定 UTC 时间点，可用 `stdc::get_leap_second_info(utc)` 检查是否为闰秒。
- 返回

`elapsed`：自纪元以来所有闰秒之和。

# 补充

- Chrono
- 编译期有理数
- 时间
- 日期与时区
- 日期
- 时区

# 日期

- 虽提供许多时间点工具，但未给出构造「日期」的直接方式。
- 可用相对纪元的 `duration` 构造时间点，但需手算（且因月年为平均时长而不精确）。

- C++20 提供大量工具协助。
- 例如要在 2021 年每月第 5 日会面：

# 日期

- 记得我们的注吗？

- 故全过程为：
1. `2021y` 构造类 `year`（而非像 `2021ms` 那样的时长）。
2. 用 `year` 的 `operator/` 得到 `year_month`。
3. 用 `year_month` 的 `operator/` 得到 `year_month_day`。
4. 用 `year_month_day` 的 `operator+=(const months&)` 得下月日期。

# 日期

- 也可把 `day` 放前：
- 即 `day` → `month_day` → `year_month_day`。
- 或 `month` 在前：
- 即 `month` → `month_day` → `year_month_day`。
- 还可用月份别名：
- 除普通 `day` 外，还有三种表示日期方式：
- 最后一日：`year_month_day_last`，从月末倒数而非从月初；如 3 月 31 日是三月最后一天。
- 星期：`weekday`，即周一～周日。第 i 个星期用 `weekday_indexed`，组成 `(year_)month_weekday`。
- 最后一个星期：`weekday_last`，从月末倒数；如 2026/2/23 是 2026 年 2 月最后一个周一。形式为 `(year_)month_weekday_last`。

1. `year_month / last` → `year_month_day_last`
2. `weekday[index]` → `weekday_indexed` `year_month / weekday_indexed` → `year_month_weekday`
3. `weekday[last]` → `weekday_last`

# 日期

`year_month / weekday_last` → `year_month_weekday_last`

- 可像普通 `day` 一样使用：

索引从 1 开始而非 0

- 此类输出无趣，故可用特殊格式：每月最后一日、每月第一个周一、每月最后一个周一等。

故不同类对 `operator+=(months)` 解释不同！

`stdc::last` 的标签类型。

# 日期

- 现可理解各种组合类：
1. 基础类型共享类似方法：
- 构造：由 `unsigned int` 构造。
- `explicit operator unsigned int`（`weekday` 除外）。
- `year` 对上述两者用 `int`，因存在公元前。
- `.ok()`：判断是否存在（细节后述）。
- `operator==/<=>`（同类）。
- `weekday` 无 `operator<=>`。
- `operator++/--/+=/-=/+/-`。
- 定义在合适 `duration` 上，如 `day += days`，`month += months`，`year += years`。
- `formatter`/`operator<</from_stream`。
- 格式说明与时间点相同，但不能使用缺失字段（如对 `day` 用 `%Y` 非法）。

# 日期

- 为何 `weekday` 特殊？
- 星期的顺序并非全球统一…
- 有些文化把周日放最后，有些放最前。
- 故难以定义统一 `operator<=>`。`[0,6]`，周日为 0。
1. C++ 提供两个 getter：`[1,7]`，周日为 7。
- 构造时 0 与 7 也相同。
2. 其算术在（模 7）上定义。
- 例如：`Friday += 2` 为 `Sunday`；`++Sunday` 为 `Monday`；`Sunday - Friday == 2`；`Monday - Friday == 3`。
- 这些整数实为 `days` 时长。
3. 最后，可由时间点 `sys_days` 或 `local_days` 构造 `weekday`。
- 故可知该历日的星期。

# 日期

2. 用 `operator[]` 索引星期得 `weekday_indexed`/`last`：`weekday_indexed` `weekday_last`
- 方法很简单：

- `operator<<` 与 `weekday` 默认格式等价 `%a`（即 locale 相关缩写），`weekday_indexed`/`last` 等价 `%a[index]`。
- 如 `Mon[1]`、`Fri[last]`。

# 日期

- 注 1：`.ok()` 的规定：
- 特别地，内部类型限制取值范围。越界行为未定义。
- 例如：`day{1}` 合法；`day{32}` 非法但行为明确；`day{256}` 由实现指定。
- 因而 `day{1} + days{255}` 的结果由实现指定（而非必定非法）。
- 注 2：对非法日期（`.ok()` 为 false），`operator<<` 与空格式会额外输出「 is not a valid XX」。
- `XX` 可为 day/month/year/weekday/index，视类型而定。
- 注 3：`month` 的算术在模 12 上定义。
- 但存储的月份会 `+1`（即在 `[1,12]`）。

# 日期

3. 复合类型也共享类似方法：
- Getter：从存储字段取属性。
- 如 `year_month_weekday_last`：
- 类型即基础类型。
- `.ok()`：判断该日期能否存在。
- 如 `2021/2/29`（作为 `year_month_day`）不存在；但 `2/29`（作为 `month_day`）可以；`2/31` 不存在，但 `31`（作为 `day`）可以。
- `operator==/!=`（同类）。
- `(year_)month_day(_last)` 还支持 `operator<=>`。
- `formatter`/`operator<<`；`from_stream` 仅用于 `(year_)month(_day)`。
- 同样，格式化不能使用缺失字段（如对 `year_month` 用 `%D` 非法）。
- `operator+/-/+=/-=(years/months)`，仅对 `year_xxx`。
- 不同类有不同解释，如前所述。

# 日期

- 注 1：`operator<<` 与空格式最终输出由 `/` 连接的基础类型。
- 如前例：
- 故非法提示也会连在一起：

- 注 2：`operator+/-/+=/-=` 的结果日期可能非法，如 `2021y/1/31 + months{1} == 2021y/2/31`。
- 注 3：四种完整日期类型可转为 `sys_days` 或 `local_days`。

# 日期

- 调整后的示例：

- **再次强调**：先 `+ months` 与 `sys_days{ first } + months` 不同；时长把月当作平均长度。

# 日期

- 另一例：

我们说过可由 `sys_days` 构造 `weekday`。

- 效果：打印 2020～2024 年间某月第 13 日为周五的日期。

# 日期

- 注 4：`year_month_day` 是特殊类型。
1. 可由 `year_month_day_last` 构造。
2. 可由 `sys_days` 与 `local_days` 构造。
- 对比之下，其他类型只能**转**到 `sys_days` 与 `local_days`。
- 故把其他类型统一转为 `year_month_day` 的方式是：(1) 转到 `sys_days`；(2) 构造 `year_month_day`。
3. `operator<<` 与空格式打印 `%F`（即 `%Y-%m-%d`），而非用 `/` 连接分量。
- 非法时，打印内容也不会落成各分量的非法信息；只额外打印「 is not a valid date」。

# 日期

- 注 5：`year_month_day_last` 额外定义 `.day()`，可直接取最后一日。

- 注 6：`year_month` 定义 `operator-(year_month, year_month)` → `months`，可得两日期相差的月数。

- 再次：具体数值作为时长并不精确。

- 注 7：所有类型 C++26 起可哈希。

# 补充

- Chrono
- 编译期有理数
- 时间
- 日期与时区
- 日期
- 时区

# 时区

- 不同地区有不同时区。
- 用时钟的偏移表示，如北京时间 UTC+8。
- 故 `13:00 Asia/Shanghai` 等同于 `5:00 UTC`，或前一日 `23:00 America/Chicago`。
- 有些文化还有夏令时/冬令时（DST / standard time），亦称 daylight-saving / standard time（日光节约时/标准时）。
- 但时区有特殊性质…
1. 偏移未必是整小时；15/30/45 分钟也常见。
2. 缩写可能歧义，如 `CST` 可指 Central Standard Time（芝加哥）、China Standard Time、Cuba Standard Time 等。
3. 国家决定调整时区时会变（如引入夏令时/冬令时）。

# 夏令时

- 顺便了解夏令时（DST, daylight saving time）。
- 一般旨在让时间与日照匹配。
- 例如冬季日光在 7:00 后出现，夏季在 6:00 后出现，则实行 DST 可使日光总在 7:00 后出现。
- 入夏时时间会跳到更晚时刻；
- 入冬时时间会跳回较早时刻。

- 例如美国部分州，三月第二个周日启用 DST，十一月第一个周日取消。
- 如 2016：`2016-03-13 02:00:00` 跳到 `2016-03-13 03:00:00`，故本地不存在 `2016-03-13 02:30:00`。
- 而 `2016-11-06 02:00:00` 跳回 `2016-11-06 01:00:00`，故 `2016-11-06 01:30:00` 对应两个 UTC 时刻。
- 故本地时间可能**不存在**或**有歧义**！

# 时区

- 时区名有四种，由 IANA（Internet Assigned Numbers Authority）规范：
1. 城市或国家。
- 如 `America/Chicago`、`Asia/Hong_Kong`、`Europe/Berlin`、`Pacific/Honolulu` 等。
2. 偏移：以 `Etc/` 开头，加 `GMT+/-N`。
- 如 `Etc/GMT-8` 是北京时间（对，`Etc/GMT-8` 是 UTC+8，Unix 对 `Etc/GMT` 的符号约定要取反）。
3. 缩写。
- 如 `UTC`、`GMT`、`CST`、`PST`。
4. 已弃用条目（无确定形式）。
- 如 `PST8PDT`、`US/Hawaii`、`Canada/Central`、`Japan`。

- 唯一名称可用于定位时区。

# 时区

- C++ 中，`stdc::time_zone` 表示时区，`stdc::zoned_time` 表示带时区的时间。
- C++ 提供数据库查询 `const time_zone*`；
1. `current_zone()`：取当前时区；
2. `locate_zone(name)`：按名查时区；
- 找不到条目时抛 `std::runtime_error`。

- 构造 `zoned_time` 有两种方式：
1. 把本地时间附到时区；
2. 把带时区时间或系统时间转到某时区。

# 时区

- 例如每周五 18:30（北京时间）会面，但洛杉矶也有参与者。Windows
- 时间安排为：

1. 本地时间附时区；2. 将带时区时间转到另一时区；

Linux

另：也可用 `stdc::floor<stdc::seconds>(…)` 代替 `stdc::time_point_cast`。

# 时区

- 另一例：取当前时区的当前时间。

系统时间转带时区时间

- 也可把 `zoned_time` 转回 `local_time` 与 `sys_time`：

# 时区

- 具体地，`zoned_time` 的构造函数：
- 每种变体有两个重载：按名（效果同 `locate_zone`）或 `const time_zone*`。
1. 本地时间附到时区：

这是什么？
- 我们说过本地时间在 DST 下可能有歧义或不存在。
- `enum class choose` 有 `earliest` 与 `latest`，对歧义时间选最早或最晚。
- 歧义时间无 `choose` 参数，或不存在时间会抛异常。

# 时区

2. 从系统时间转换：未提供时区时默认用 UTC。

3. 从带时区时间转换：

- 实际上还有带 `stdc::choose` 为最后参数的重载，但该参数无效果。

# 时区

- 最后还有一些朴素构造：
- 等价于默认构造的 `time_point`，即 `.time_since_epoch()` 为零。再次，缺省时区为 UTC。

- 赋值：

- 最后两个保留原时区。
- 因 `local_time` 无带 `choose` 的变体，可能抛异常。

# 时区

- `zoned_time` 的其他方法：
- `.get_time_zone()` → `const time_zone*`；
- `.get_info()` → `stdc::sys_info`：取当前时间下该时区信息。
- 包含：
1. 该偏移有效的区间。
2. 相对系统时间的偏移。
3. 可能的 DST 偏移。
4. 时区缩写。
- 可直接 `operator<</format` 打印。
- 例如：原因：中国 1986～1991 年实行过 DST。洛杉矶在 DST 内。洛杉矶不在 DST。北京无 DST。北京有 DST。（应为 GMT+9，但 Windows 似乎错误）而 Linux 正确：

# 时区

- 相等比较：比较时间点与时区指针；
- C++26 起可哈希；
- `operator<<` 与 `format`：同输出本地时间，除了：
1. `%Z` 输出时区缩写，由系统决定；
2. `%z` 输出 `[+/-]HH[MM]` 形式的偏移；
- 如北京为 `+0800`（或等价 `08`、`+08`）；前文 UTC 时钟、TAI 时钟等为 `+0000`（或 `00`、`+00`、`-00`）。
3. `%Oz`、`%Ez` 用 `[+/-]H[H][:MM]` 及其他 locale 相关变换。
- 如北京为 `+08:00`（或 `+8`、`+08`、`8`、`08`）；前文为 `+00:00`。
4. 默认格式为 `%F %T %Z`。
- 如 `2026-03-21 15:15:06 GMT+8`。
- 最后还有成员类型：以及类型别名：

# 时区

- 一般不对 `zoned_time` 直接输入；但可用 `from_stream` 输入本地时间再构造 `zoned_time`。

- 提供 `%Z` 时填充缩写；提供 `%z`/`Oz`/`Ez` 时填充偏移。
- `%Z` 取正则 `[\w|-|+|/]`（即 A-Z、a-z、0-9、`_`、`-`、`+`、`/`）的最长匹配序列。
- 例如：

注：`stdc::parse` 接受引用而非指针。

# 时区

- 注 1：`%Z` 与 `%z` 分开处理，故偏移可能与时区名不一致。
- 注 2：与其他说明符不同，`%Z` 本质上不可逆。
- 即：`%Z` 输出的名通常不能用于定位时区。两种情况：
1. 伪名，不是时区。
- 如 `GPS`、`TAI`。
2. `locate_zone()` 用全名定位，但 `%Z` 输出缩写。
- 如前文例：
- `GMT+8` 不是合法时区名。
- 仅偶尔缩写也是唯一名（如 `UTC`）。
- 解决：格式化时输出 `.name()`，或扫描时区数据库让用户判定（后述）。

# 时区

- 若干最后说明…
- 注 1：`time_zone` 的方法：
- `zoned_time` 中多数方法只是调用它们。
- 当然 `to_sys` 也有带 `choose` 的重载。
- 可见 `time_zone` 也为 `local_time` 提供 `.get_info`…
- 返回 `local_info`，描述把 `local_time` 转为 `sys_time` 的结果：
- 当 `.result` 为歧义或不存在时，`first` 为前一区间的 `sys_info`，`second` 为后一区间。

# 时区

- 注 2：严格说，时区指针是模板的一部分：

- 前文称返回 `const stdc::time_zone*` 的，实质上返回 `TimeZonePtr`。
- 细节见作业，因不太重要。
- 注 3：当前 MS-STL 访问时区信息略慢，未来可能改进。

# 时区数据库

- 最后介绍底层数据库。
- C++ 标准库规定使用 IANA 时区数据库，但 OS 可能只部分采用。
- 甚至可不存在；例如嵌入式系统不关心时区，定制 OS 会移除。
- 特别地，Win10 之前的 Windows 没有该数据库。
- 导致：
1. 数据库不存在时，`locate_zone` 等会抛 `std::runtime_error`。
2. 长时间运行的程序可能数据库过时给出错误时间。
- 例如缺少新闰秒或新时区。

# 时区数据库

- 本质上程序维护一个时区列表。
- 用 `stdc::get_tzdb_list()` → `stdc::tzdb_list` 取得。
- 列表每个元素是一个数据库 `stdc::tzdb`。
- 前文 `locate_zone()` 与 `current_zone()` 只用列表头（等价 `stdc::get_tzdb()`）。
- 可用 `stdc::reload_tzdb()` 更新表头，与 `stdc::remote_version()` 比较版本。
- 若表头版本更旧，会把远程最新版插到列表前面。
- 故长时间运行程序可调 `reload_tzdb` 使 `locate_zone` 保持最新而无需更新 OS。
- 那…标准库为何维护列表？

# 时区数据库

- 原因：程序中可能仍有 `time_zone*` 指向旧数据库条目。
- 直接删除会导致非法访问。
- 但可用手动 `tzdb_list` 方法：

- 故下列语句等价：

`name()` 是替代名，`target()` 是原名。

# 时区数据库

- 最后 `tzdb` 的成员：

→ `sys_seconds`

- `stdc::leap_second` 包含闰秒发生时刻信息：

- 故可写程序检查所有带某缩写的时区：

对缩写 `= "CST"`：

具体列表由系统决定。
