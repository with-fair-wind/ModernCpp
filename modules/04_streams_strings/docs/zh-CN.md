字符串与流 String and Stream

# 现代C++基础

# Modern C++ Basics

### 梁嘉铭，北京大学本科生

### • 字符串与 string view

### • Unicode 支持与 locale

### • 打印函数与 Formatter

### • 流（Stream）

### • 正则表达式（Regular expression）

# 在那之前…

### • 我们先复习并扩展字符与字符串

### 字面量中的一些内容。

- 首先，字符字面量就像 `'a'`，或是转义形式 `'\n'`、`'\\'`，或是八进制表示 `'\0'`、`'\123'`，或是十六进制表示 `'\x12'`。
- 自 C++23 起，你也可以使用 `'\o{12}'`、`'\x{12}'`。
- C 风格字符串：例如 `"abc\x12\n"`。
- 它以空字符结尾，这意味着它实际上有 6 个字符。
- 这把它与“纯字符数组”（例如 `char a[] = { '1', '2' }`）区分开来。
- 尽管它实际上是 `const char[]`，赋给 `auto` 变量时会退化为 `const char*`；在 C++ 中你不能在没有 `const_cast` 的情况下把它赋给 `char*`。
- 你可以拼接它们，例如 `"123" "456"` 实际上与 `"123456"` 相同。
- 如果你需要很长的字符串，这很方便——只要换行即可！

# 在那之前…

- 原始字符串：任何字符都不会被转义。
- 例如，`"\\\n\""` 实际上表示 `"\\n"`，但 `R"(\\\n\")"` 恰好是六个字符。
- 这对例如 Windows 文件系统路径非常有用，Windows 用反斜杠 `\`（而不是像 Unix 那样用斜杠 `/`）作为分隔符。
- 如果你想要 `'\n'`，那就直接输入真正的换行，例如 `R"(This is a new line)"` 会得到 `"This is a \n new line"`。
- 为了防止解析错误（例如 `"` 异常），边界由 `"( 和 )"` 决定。
- 但如果我们想在字符串里放 `)"` 呢？
- 那你可以通过加入一些内部字符来改变边界，例如 `R"+(I want a )"!)+"`。

### • 此外，我们把以转义形式显示所有字符的字符串（例如用 `\n` 表示换行而不是真正的换行）称为转义字符串。

# String and Stream

### String and string view

# String and stream

- string
- string view

# string

### • `std::string` 本质上就是对 `std::vector<char>` 的增强。

- 基本实现通常与 `vector` 相同，例如重分配。
- 连续区间。
- 随机访问迭代器，即 `(c)begin/end`、`(r)begin/end()`；
- 比较、`swap`、`std::erase(_if)`。
- `.empty` / `.size` / `.max_size` / `.capacity`；
- 也提供 `.length()`，与 `.size()` 完全相同。
- `.at` / `operator[]` / `.front` / `.back()`；
- `.clear` / `.shrink_to_fit` / `.reserve` / `.resize` / `.push_back` / `.pop_back()`；
- 特别地，`std::string` 里的 `reserve` 以前可用于在扩容前收缩内存；但自 C++20 起，它与 `std::vector` 一样，即当参数 `<= capacity` 时不起作用。
- `.assign` / `.insert` / `.erase` / `.insert_range` / `.assign_range`（C++23）；
- 也提供 `.append` / `.append_range`，就像 `insert(str.end(), …)`。
- 但它返回新字符串的引用。

# string

```cpp
string& insert(size_t pos, StringView str, size_t subpos=0, size_t sublen=npos);
```

### • 此外，它还提供适合字符串的独有 API：

- 对字符串而言更常用的是下标，因此 `.assign` / `.insert` / `.erase` / `.append()` 也提供基于下标的版本。
- 第一个参数从迭代器改为下标；若下标 `> size`，会抛出 `std::out_of_range`。
- 对于插入/赋值，你可以提供 string / C 风格字符串 / string view，会插入/赋值整体内容。
- 对于 C 风格字符串，你也可以提供计数，以显式指定要插入的字符个数。
- 当然，如果你事先已知长度，传入长度可能比只传 C 风格字符串更高效。
- 对于 `string` / `string_view`，你可以再提供另一个下标作为拷贝起点，以及可选的（默认为末尾）计数。
- 对于 `erase`，只提供 `(index, count)`。
- 对于 `append`，与插入相同，只是第一个下标参数等于 `str.size()`。

# string

- 所有基于下标的方法返回 `std::string&` 而不是迭代器。
- 更多其它与字符串相关的方法。
- `operator+` / `+=` / `hash`；
- `.starts_with` / `.ends_with`（C++20）；`.contains`（C++23）；
- 参数可以是 `char` 或任意字符串类型。
- `.substr(index(, count))`：返回与子串相同的新字符串。
- `.replace`：用新字符串替换字符串的一部分，返回 `std::string&`。
- “一部分”由前两个参数指定：迭代器对 `(first, last)` 或 `(index, count)`。
- “新字符串”由后续参数指定，基本上与 `insert` 相同（例如所有字符串类型、带计数的 C 风格字符串、带计数的字符、带 `(index(, count))` 的字符串等）。
- 总之，若不确定请看 IDE 提示。
- C++23 还支持 `.replace_with_range()`，即 `(first, last, range)`。

# string

- `.data()` / `.c_str()`：取得底层指针（即 `const char*`）。
- 自 C++17 起，对非 `const` 的 `string`，`.data()` 返回 `char*`（即非 `const`，与 `vector` 类似）。
- 查找：`.find` / `.rfind` / `.find_first(_not)_of` / `.find_last(_not)_of()`。
- 它们与标准库算法类似，但返回下标而不是迭代器。
- 若未找到，返回 `std::string::npos`（即 `static_cast<size_t>(-1)`）。
- `.find` / `.rfind` 用于查找字符/子串，下一个参数为起始位置（默认 0）。
- 例如：`str = "PKU>THU"`；`str.find("TH")` 得到 4，`str.find("TH", 5)` 得到 `std::string::npos`。
- 例如：`str = "PKU>THU"`；`str.find_first_of("TH")` 得到 4（因为 `str[4]` 是 `"TH"` 中的 `'T'`，是 `"TH"` 中任一字符的首次出现），`str.find_first_of("TH", 5)` 得到 5（即 `'H'`）。

# string

- 最后，所有计数都可以用 `std::string::npos` 代替，表示“直到字符串末尾”。

### • 好吧，API 可能让你觉得枯燥，

### 所以至少对它们有个大致印象，

### 并善用 IDE 给你的提示！

### • 现在我们来聊一些

### 关于 `string` 的有趣之处…

### • 注 1：还记得复习课里

### 我们提到过的内容吗？

# string

### • 注 2：`std::string` 保证底层字符串以空字符

### 结尾。

- 你也可以在字符串里包含 `'\0'`，因为它不像 C 风格字符串那样靠 `'\0'` 判断结束，而是靠 `.size()`。

### • 注 3：通常 `std::string` 有 SSO（小字符串优化）。

- 也就是说，若字符串较小，可能不在堆上分配内存，而是直接使用栈上的空间。
- 还记得吗？与例如整数加法相比，`new`/`delete` 是相当昂贵的操作。因此当字符串很小时，为此去堆分配并不划算。
- 具体阈值并不由标准规定，但在 x64 上 libstdc++/VC 为 15，libc++ 为 22。
- 即便有 SSO，对象总尺寸通常也足够小；它们往往小于 40 字节，意味着拷贝它们基本上与拷贝几个 `std::uint64_t` 一样便宜。

# string

### • 当字符串足够长时，存储会分配在

### 堆上，此时它基本上与 `std::vector<char>` 相同。

### • 注 4：C++23 为 `resize` 引入了另一种优化。

- `resize` 只会用固定字符（默认 `'\0'`）填充，这通常还需要后续赋值；那是性能损失…
- 当然，有时可以用 `reserve` + `insert`；
- 但应该有一种更方便（且可能更高效）的 API，让用户能重新分配并写入！
- `.resize_and_overwrite(newSize, Op)` 就是为此设计的。
- `Op` 应接受 `(char* ptr, size_t len)`，然后覆盖写入。
- `len` 等于 `newSize`，且 `[ptr, ptr + min(oldSize, newSize))` 与 `[.data(), .data() + min(oldSize, newSize))` 相同。
- 返回 `realSize`（`<= newSize`），使得最终 `.size() == realSize`。
- 它不应抛出任何异常。

# string

### • 注 5：你也可以把字符串与数字互转。

- `std::stoi` / `sto(u)l` / `sto(u)ll(string, std::size_t* end = nullptr, int base = 10)`；
- 它会在第一个解析出的数字末尾停止，并尝试把停止下标写入 `*end`（例如 `"123 456"` 会写 `*end = 3`；越界则为 `size`）。
- 若第一个数字无效（即未找到数字就停止），会抛出 `std::invalid_argument`。
- `Base`（进制）可以是 2–36；当 `base > 10` 时，会用字母表示数位。
- `Base=0` 表示按前缀自动识别进制（即 `0` 开头为八进制，`0x` 为十六进制，否则为十进制）。
- 若结果无法用 `int`/… 表示，会抛出 `std::out_of_range`。
- `std::stof` / `stod` / `stold(string, std::size_t* end = nullptr)`
- `std::to_string()`，接受浮点数或整数。
- 自 C++26 起的破坏性变更：将与 `std::format("{}", val)` 相同；我们稍后会讲 `format`。变更细节见 P2587。
- 以前对浮点数会显示定点形式（即小数点后固定 6 位），这可能无法用例如 `stof()` 再转回去。例如 `std::to_string(1e-7f)` 在 C++26 之前会得到 `"0.000000"`，之后会变为 `"1e-7f"`。

# String and stream

- string
- string view

# string view

### • 有时，你可能在函数里只读字符串，于是

### 使用 `const std::string&` 作为参数。

- 然而，若你传入 C 风格字符串，例如 `"PKU"`，那就必须创建一个临时 `string`。但我们只想读取，为什么要承受这种性能损失？
- 自 C++17 起，定义在 `<string_view>` 中的 `std::string_view` 就是为了解决这个问题！
- 以前，你可能不得不为 C 风格字符串写重载（这也是 `std::string` 的方法有这么多重载的原因之一…）
- `std::string` 与 C 风格字符串可以隐式转换为 `std::string_view`，你也可以用迭代器对构造它。
- 它有点像 `span<const char>` 的特化，即只有 `const char*` 与长度。
- 因此，对 `std::string` 的方法用 `std::string_view` 代替没有长度的 `const char*` 可能提升性能。
- 它拥有 `std::string` 几乎所有非修改性方法。

C++26 很可能为拼接 `std::string` 与 `std::string_view` 增加 `operator+`，见 P2591。

# string view

- 随机访问迭代器，即 `(c)begin/end`、`(r)begin/end()`；
- 比较、`swap`、`hash`。
- `.empty` / `.size` / `.length` / `.max_size`；
- 没有 `capacity`，因为它是视图；它只观察内存。
- `.at` / `operator[]` / `.front` / `.back()`；
- `.clear` / `.shrink_to_fit` / `.resize` / `.push_back` / `.pop_back()`；
- `.starts_with` / `.ends_with` / `.contains()`；
- `.find` / `.rfind` / `.find_first(_not)_of` / `.find_last(_not)_of()`；
- `std::string_view::npos`。
- `.substr(index(, count))` / `.remove_prefix(n)` / `.remove_suffix(n)`；
- 它们都创建 `std::string_view` 而不是 `std::string`，因此是 𝑂(1)。
- `.data()`：始终返回 `const char*`，没有 `.c_str()`；
- 它不支持 `operator+`，因为需要创建 `std::string` 才有意义，但那很昂贵，所以你必须显式去做。

# string view

### • 一些例子：

- 透明比较运算符：

- 按子串 `std::sort`：
- 与 `s1.substr()` 相比这是优化，因为它不会创建临时字符串，而只是指向其中一部分的廉价视图！
- 以前你必须使用 `std::lexicographical_compare`。

# Caveats on string view

### • 1. `std::string_view` 不要求以空字符结尾。

- 毕竟，它只是带长度的 `const char*`！
- 因此，把 `.data()` 传给要求空结尾的 API（例如 C 风格字符串 API）可能不安全。
- 若你确实需要，有两种可能做法：
- 构造 `string_view` 使得 `.back() == '\0'`，例如用迭代器对。
- 但这会让 `.size()` 与 `strlen` 不同，因为它额外计入了终止符。
- 或者确保 `str[size] == '\0'`；
- 这应由用户保证，因为你在逻辑上在读越界，这是“逻辑上”错误的。
- 用流输出不要求空结尾。

但自 C++23 起，你不能用

# Caveats on string view

`nullptr` 直接构造

`std::string_view`；你需要 `{nullptr, 0}` 或默认构造函数。

### • 2. 它包含的指针可以是 `nullptr`（默认构造即是）。

- 这通常发生在 `.size()` 为 0 时，因此要特别检查。

### • 3. 若你想把

### `std::string_view` 作为返回值，真的要非常谨慎。

- 它就像 `span` 或 `ref_view`；若所指对象生命周期结束，那就是悬垂引用！
- 有时这很隐蔽，例如：
- 若你传入临时 `string`（例如 `"PKU"`），则函数结束后其生命周期结束，于是 `std::string_view` 悬垂。
- Ranges 有类似问题，但会对右值使用 `owning_view`，因此略安全（但此例不是右值，仍然危险）。
- 另一例：`auto s = CreatePerson().GetName();` 很危险，因为临时 `person` 已析构，指向其名字的视图无效。

# Caveats on string view

- 相反，返回 `std::string` 更安全。
- 所以，若你真的要返回 `std::string_view`，请在文档与函数命名上写得显式且醒目！

### • 4. 可能与 `std::string_view` 相关的模板参数

### 也要注意返回类型。

- 若传入两个 `std::string_view`，返回物仍是 `std::string_view`。
- 用 `auto` 代替。

### • 5. 若你反正要创建字符串（例如在构造函数里），传

### `std::string_view` 并不是好主意。

- 等我们学了移动语义后，下一讲会解释原因！

# User-defined literals

### • 你可能觉得从 C 风格字符串创建 `string`/`string_view`

### 很麻烦。

- 例如 `std::string_view{ "PKU" }`。
- 为什么不能用类似 `1ull` 的方式表示类型？
- 用户定义字面量就是为此而生！
- 有一些预定义的标准字面量，例如对 `string`/`string_view` 可以用 `"PKU"s` 与 `"PKU"sv`。
- 除字符串外，还有另外两类标准字面量：
- 时间相关：`1s` 表示秒，`1.1ms` 表示毫秒，`1d` 表示 1 天，等等。
- 复数相关：`1i` 表示纯虚数，`1.2if` / `2.5id` 表示显式类型（float/double 虚部）。
- 记得在局部作用域写 `using namespace std::literals;`！

# User-defined literals

### • 你也可以定义自己的字面量，例如我写过的一个缓存模拟器：

### • 你需要定义字面量运算符：

- 建议自定义字面量后缀为 `_xx`，即以下划线开头，以减少与标准字面量冲突的可能。
- 这里假设 `< 4GB`，因此返回类型用 `unsigned int`；一般情况用 `unsigned long long` 更好。
- 我们以后会详细解释 `constexpr`；基本上它表示“编译器会尽量在编译期计算”。

# User-defined literals

### • 参数类型是受限的：

- 对整数，只允许 `unsigned long long`。
- 因为它通常是最大的整数类型，能统一所有整数而不必定义大量重载。
- 转换不会造成运行时性能损失，因为它们在编译期完成。
- 对浮点数，只允许 `long double`。
- 对字符，例如 `char` 与 Unicode 字符类型，都可以。
- 对 C 风格字符串，需要 `(const CHAR*, std::size_t)`，其中 `CHAR` 为任意字符类型。
- 因此你可以利用长度来最大化效率！
- 最后也提供 `(const char*)`，作为整数与浮点的回退。
- 会把它们当作字符串处理。
- 不过这很少用。

# charconv

### • 最后，`stoi`/`to_string` 会创建新的 `std::string`；我们可能

### 想自己提供存储。

- 例如 `stoi(std::string{view})` 很昂贵，因为我们只读字符串。
- 而且它们可能抛出异常，有时那也很昂贵。

### • 你可以在 `<charconv>` 中使用 `std::from_chars` 与 `std::to_chars`！

- `std::from_chars(const char* begin, const char* end, val)` 会尝试把结果写入 `val`（整数或浮点）。
- 它返回 `std::from_chars_result`，包含 `.ptr` 作为停止位置，`.ec` 作为错误码。
- 当 `ec == std::errc{}` 为成功；也可能是 `ec == std::errc::invalid_argument`，或 `std::errc::result_out_of_range`。
- 你可以使用结构化绑定，例如 `if(auto [ptr, ec] = xx; ec != std::errc{})`。注意 C++26 可以省略 `ec != std::errc{}`，因为 `std::from/to_chars_result` 可以直接转换为 `bool`。

# charconv

越界仍会得到前进后的指针！

# charconv

- 也可以把 `base`/`std::chars_format` 作为最后一个参数提供。
- 对整数，`base` 应在 `[2, 36]` 中，默认 10。
- 注意只会识别负号；例如前导空白、`"0x"` 等不会被识别。
- 对浮点，应提供 `std::chars_format::xx`。
- `scientific`：形如 `(-)d.ddde±dd`；
- `fixed`：形如 `(-)d.ddd`；
- `hex`：形如 `(-)h.hhhp±hh`；（十六进制浮点很少用，此处不展开）。
- `general`：`scientific | fixed`，两者皆可。
- 特别地，`"NAN"`、`"INF"`（大小写不敏感）都可以。
- 结果舍入为最近值。

不幸的是，即便在 2024 年 6 月，libc++ 仍不支持浮点的 `std::from_chars`（见实现状态）。libstdc++/MS-STL 均已支持。

# charconv

- `std::to_chars(char* begin, char* end, value)` 会尝试把 `val`（整数或浮点）写入 `[begin, end)`。
- 注意不会写入空终止符！
- 它返回 `std::to_chars_result`，同样包含 `.ptr` 与 `.ec`。
- 当 `ec == std::errc{}` 为成功；也可能是 `ec == std::errc::value_too_large`（`ptr == end`，`[begin, end)` 内容未定义）。
- 也可以把 `base`/`std::chars_format` 作为最后一个参数提供。
- 对整数，`base` 应在 `[2, 36]` 中，默认 10。
- 对浮点，应提供 `std::chars_format::xx`。
- 你也可以提供 `int precision`，指定小数点后的位数。
- 特别地，对为整数的浮点，或某些可被精确表示的值（如你在 ICS 学过的 `2.0f`、`2.125f`），在 `std::chars_format::general` 下会截断而不是对齐填充。
- 例如 `(general, 4)` 会得到 `2` 或 `2.125`，但 `(fixed, 4)` 会得到 `2.0000` 或 `2.1250`。

# charconv

### • `precision` 为 10 表示小数点后需要 10 位数字，

### 这对 `std::array<char, 10>` 不可能。

- 改成 `3.14` 仍会失败！
- 此外，不会写入空终止符，所以我们不要直接把 `str.data()` 当作 C 字符串用。
- 自己在 `ptr` 处追加空终止符，或像这样构造 `string_view`。

### • 最后，不带精度限制的 `from_chars`/`to_chars` 在同一平台上

### 可以做往返转换。

我们尚未展示完整函数体，因为还没学如何传入可变参数。

# String and Stream

### Unicode support and locale

# String and stream

### • Unicode 支持与 locale

- Unicode
- C++ 中的 Unicode 支持
- Locale

Credit: CppCon 2014: "Unicode in C++", James McNellis

# Unicode

### • 我们以前用的字符只是 `char`，通常使用

### ASCII。

- 然而 ASCII 无法表示所有符号，例如法语、中文等。
- 人们提供了许多编码标准，例如中文的 GBK（国标扩展码），韩文的 EUCKR（Extended Unix Code for Korean）等…
- 但它们通常不可跨平台；例如 GBK 文件在某些终端会完全乱码（你用 Python 时可能遇到过）。
- 因此 Unicode 作为统一字符编码被提出。

### • Unicode 也在逐步演进，这里只做粗略介绍。

- UTF1.0：每个字符 16 位；也称为 UCS-2。
- 既然需要 2 字节，就要确定字节序；因此若文件可能在不同字节序的机器上使用，可以提供 BOM（Byte-Order Mask）`0xFE 0xFF`。

# Unicode

### • UTF2.0：考虑到 2¹⁶ = 65536，对全世界字符仍嫌不足。

- 对象形文字（象形文字）是常见情况，例如 GBK 有 21003 个汉字。
- Emoji 也编码在 Unicode 里；规模更大！
- 因此引入 UTF2.0，并有不同的编码表示。
- 它们有相同的 Unicode 码位（例如 ☞ 是 `0x1F449`），但在计算机中有不同编码表示。
- UTF-32：让每个字符占 32 位；直接但有用，因为最多可使用约 40 亿个字符。
- 但它可能占用太多空间，例如 `"abc"` 以前只需 4 字节，现在却可能要 16 字节！
- 它也需要 BOM。

# Unicode

- UTF-8：为解决空间浪费，UTF-8 使用变长编码。
- 不同字符长度有不同编码前缀（类似我们学过的 Huffman 树）以保证无歧义。
- 这会浪费一些码空间，因此有些字符可能需要超过 4 字节。
- 例如 ASCII 字符，包括空终止 `'\0'`，在 UTF-8 中仍占 1 字节（编码也相同）。
- UTF-8 是现代系统中最常用的字符集。
- UTF-16：为兼容 UTF-1.0（UCS-2），也引入 UTF-16。
- 它与 UTF-8 类似，但以 2 字节扩展（例如 2 字节不够时用 4 字节）。
- 16 位码元与 UCS-2 完全相同。
- UTF-16 用作 Java、C# 等语言中字符串的默认内部编码。

注意 UTF-8 不需要 BOM，但 Windows 很久以前就通过 BOM 识别 Unicode（UCS-2 需要），这已成为 Windows 的一部分难以改变，因此 Windows 可能要求你给 UTF-8 加 BOM。

这只在自动检测编码时发生；笔记本类软件通常允许你显式指定编码，此时 BOM 不必需。

# Unicode

### • 字符规范化（normalization）：

- 许多字符充当“修饰符”，例如拼音里的声调，`ÀÁÂÃÄÅ` 上的附加符号等。
- Unicode 也支持组合字符，即把修饰符与字符组合（例如把 `'` 与 `A` 组合得到 `À`）。
- 它们编码为两个字符，但只显示为一个。
- 然而，有些组合字符可能已有独立 Unicode 码位，因此同一符号可能有不同表示。
- 若要比较两个 Unicode 字符串，可能需要规范化以得到统一表示。

### • Unicode 字符也可能有别名，例如 GREEK CAPITAL LETTER OMEGA 表示 Ω。可在此查阅。

# Unicode

### • 总之，Unicode 包含这些基本要素：

- Byte，即计算机表示。
- Code unit，即表示一个字符所用的（字节数 ÷ 最小字节数）（UTF-8 为 1，UTF-16 为 2，UTF-32 为 4）。
- Code point，即每个 Unicode 字符的码位。
- Text element，即人类在屏幕上真正看到的文本单元。
- 要在文本单元层面比较两个字符串，需要规范化。

### • 例如，忽略空终止，`1Ä` 有 3 个文本单元、4 个码位（因为有组合字符），并且：

# String and stream

### • Unicode 支持与 locale

- Unicode
- C++ 中的 Unicode 支持
- Locale

# Unicode support in C++

### • 不幸的是，C++ 对 Unicode 的支持相当弱。

- 有 `char8_t`/`char16_t`/`char32_t` 用于 UTF-8/16/32，但它们是作为单个 code unit，而不是单个 code point。
- 它们至少为 8/16/32 位以容纳一个 code unit。
- 可以分别用 `u8`/`u`/`U` 前缀。
- 但一个 code point 可能（且通常）由多个 code unit 组成，因此会出现一些怪异现象：
- 也有 `std::u8string` 等类型，但它们都以 code unit 为单位！
- 这让遍历非常困难，例如 `for(auto ch : std::u8string{u8"刘"})` 不会得到 `"刘"`，而是多个 code unit。
- `.size()`、`.find_first_of()`… 也都是针对 code unit。

# Unicode support in C++

### • 所以事实上，例如 `std::string`、`std::string_view` 只是

### 模板的实例化！

- 更具体地，即 `std::basic_string<char>` 与 `std::basic_string_view<char>`。
- 所以例如 `std::u8string`、`std::u8string_view` 就是 `std::basic_string<char8_t>` 与 `std::basic_string_view<char8_t>`。
- 由于 `char8_t` 本身是 code unit，字符串也表示 code unit 的向量。
- 它还接受 `Traits = std::char_traits<charT>` 作为第二模板参数，用于规定 `basic_string` 使用的字符操作（例如比较）。因此理论上（但通常不用）你可以定义自己的字符类型（只要它是标准布局且平凡），再配以 `YourString` 与 `NewTraits<NewCharType>`。
- 你也可以像 STL 一样提供分配器；我们以后会讲。

# Unicode support in C++

### • Unicode 支持弱主要是因为它很复杂，那为什么

### 不用 Unicode 组织提供的标准库？

- C++ Core Guidelines 建议使用 ICU（International Components for Unicode）、Boost.Locale 等以获得完整 Unicode 支持。
- 你也可以用 `utf8cpp`，它按 code point 操作 UTF-8，因为 ICU 对小项目来说太大。

### • 此外，在 C++ 里输入/输出 Unicode 很麻烦。

- C++23 有所改进（但仍主要输出 UTF-8），我们稍后会提到。

标准库也不支持 Unicode 正则；我们会在正则一节提到。

# Unicode support in C++

### • C/C++ 里还有宽字符 `wchar_t`。

- 它依赖平台，标准只规定“它与某种整数类型具有相同属性”。
- 实践中，Windows 上是 UTF-16，Linux 上是 UTF-32。
- 因此若你的平台一致，可以使用它。
- 宽字符/宽字符串字面量前缀是 `L`，例如 `L"爱情可以慷慨又自私"`。
- 你也可以使用 `std::wcout`/`wcin`/`wstring(_view)`。
- 好吧，C++ 里没有 `std::u8cout` 等。

# Unicode support in C++

### • 这里假设所有字符类型都能无损转换为 `int`。

`\Uxxxxxxxx`（8 位十六进制）或 `\uxxxx`（4 位）表示 Unicode 码位，不受计算机内部表示影响。`\U0001F449` 是 ☞。

自 C++23 起，你也可以使用 `\u{1F449}`，其中位数可变。有别名的符号可以写成 `\N{…}`（例如 `"\N{GREEK CAPITAL LETTER OMEGA}"`）。

# Unicode support in C++

### • 最后，你可能以前就在 C 字符串里写非 ASCII 字符，

### 仍能得到正确的输入/输出。

- 那为什么？它们不应该是 Unicode 吗？
- 事实上，C 字符串或 `std::string` 更像字节数组，而不是 ASCII 字符串。
- 例如，大多数中文电脑默认使用 GBK 字符集：
- 但如果在 VS 里指定 `/utf-8`，你会得到：

- 你需要把文件存为 UTF-8 才能消除警告。

GBK 码表可在此查找。

对没有任何前缀的字符串字面量，编码由执行字符集决定。`我放手` 在 GBK 执行字符集与输入字符集下，所有字符使用相同编码。

`int main() { std::cout << "我放手"; const char8_t str[] = u8"我任走"; const char16_t str2[] = u"假洒脱"; /* 编译器工作… */ const wchar_t str3[] = L"谁懂我" L"多么不舍得"; }` 设想另一位程序员要读你的代码。若你的源码是 GBK，但他/她以 UTF-8 打开，就会出现“锟斤拷”！

编译器就是“另一位程序员”；输入字符集用于让它知道如何读取。`我任走` 在 UTF-8；`假洒脱` 在 UTF-16；`谁懂我多么不舍得` 在宽字符编码（Windows 上为 UTF-16）

`/utf-8` == `/source-charset:utf-8` + `/execution-charset:utf8`

# Unicode support in C++

### • 要把 UTF-8 指定为字符串字符集，

### 需要进入 项目 -> 属性，然后像这样添加

### `/utf-8`：

### • 此外，你需要把文件保存为

### UTF-8（否则若使用非 ASCII 字符会提示警告，因为 GBK

### 无法识别 UTF-8）。

- 一种方法是 扩展 -> 搜索UTF -> 下载：

- 另一种方法是使用 VS 设置，更多细节见此处。

# Unicode support in C++

### • 对 gcc/clang，可以使用 `-finput-charset=utf-8` `-fexec-charset=utf-8` 来指定。

- Clang 只支持 UTF-8 作为执行字符集。

### • VSCode 可在右下角轻松选择编码：

- 然而，对 Windows 控制台输出，你可能需要在 `<Windows.h>` 中使用 `SetConsoleOutputCP(CP_UTF8)`；控制台尚不支持 UTF-8 输入（无论设置如何）自 2023 年 5 月 Windows 11 的终端 v1.18 起支持 UTF-8 输入。

### • 最后一句：若你真的要做本地化相关、并以复杂方式操作

### Unicode 字符串，建议使用 ICU 的 `UnicodeString` 或 Qt 的 `QString`。

- 我们会简要介绍 C++ 的 `locale`，但也建议使用其它本地化库。在 xmake 中可以使用 `set_encodings("utf-8")`。

# String and stream

### • Unicode 支持与 locale

- Unicode
- C++ 中的 Unicode 支持
- Locale

# Locale*

- 前言：理解 `locale` 与 `facet` 的极佳资源是 Apache 的文档（第 24–26 章）。若想深入了解，不妨去读！

### • `Locale` 用于本地化。

- 每个 `std::locale` 通过一组 `std::facet` 对象包含一组与文化相关的特性信息。
- 也就是说，你可以把 `std::locale` 看作指向各个 `std::facet` 的指针集合，每个 `std::facet` 包含某种文化的一个特性方面。
- 标准 `facet` 分为六类：
- Collate：即字符如何比较、变换或哈希。
- 它们不影响默认比较/哈希行为，你仍需显式传入相应函数对象。
- Numeric：`num_get`、`num_put`、`numpunct`。
- `num_get` 影响 `std::cin` 如何工作，`num_put` 影响 `std::cout`，`numpunct` 只影响标点。

# Locale*

- 例如，在德语里 `1.234,56` 表示 `1234.56`；若使用德国 `locale`，输入 `1.234,56` 会正确解析为 `1234.56` 而不是 `1.234`。
- Time：`time_get`/`time_put`；
- Monetary：`money_get`/`money_put`/`moneypunct`；
- Message：把某些错误信息变换为另一种语言。
- Ctype：`ctype`/`codecvt`，字符如何分类与转换，例如是否大写。
- 顺便一提，`<codecvt>` 在 C++17 被弃用、在 C++26 被移除，但事实上标准 `codecvt` 定义在 `<locale>` 中，因此影响不大。
- 以前 `codecvt` 也有 Unicode 转换，但自 C++26 起弃用。见 LWG3767，现在需要等待标准 Unicode 库来做这件事。

### • 它们会影响输入、输出、字符识别与

### 正则表达式如何工作。

- 例如，要对 `wstring` 输出中文，你可能需要 `.imbue`：

按名称创建 `locale`（名称依赖操作系统）。本机 `locale` 可用 `std::locale("")` 取得。

`<cctype>` 里有类似函数，使用全局 `locale`（默认 `classic()`），但你可以用 `std::locale::global(Loc)` 来

# Locale*

设置另一个，因此可能不安全。

### • `<locale>` 也定义字符分类函数。

- 它接受 `locale` 作为第二参数（随后用 `ctype` facet 来分类），于是你可以例如使用法语版的 `std::toupper`。
- ASCII 版本是 `std::locale::classic()`。

# Locale*

- `Locale` 拷贝很便宜；拷贝时使用引用计数并指向同一组 facet。
- Facet 用静态成员 `id` 的地址分组；例如若要新的 facet，需要继承 `std::locale::facet` 并拥有 `static std::locale::id id`。

继承并重写父类方法。

替换原 `locale` 的某个 facet。它与 `numpunct` 同属一组，因为它们有相同的 `id`。

Facet 在不再被引用时删除，因此 `new` 是正确的。它也使用引用计数。

新的 `id` 地址，新的 facet 组。

# Locale*

### • 创建新的 facet 组：

- 可扩展性很好。`std::ctype<char>` 由标准规定，可直接使用而不必检查。

当 facet 组不存在时，会抛出 `std::bad_cast`。

### • 作业里会有一道来自

### Apache 文档第 26 章的 `locale` 练习。它是可选项，仅供

### 兴趣。

# String and Stream

### Print function and formatter

# Format

### • 当你写一长串 `<<` 给流时可能很烦。

- 例如 `std::cout << "a=" << a << ",b=" << b << ",c=" << c << '\n';`
- 但 `printf` 就像 `printf("a=%d,b=%d,c=%d\n", a, b, c);`
- 有时你也听说 `printf` 比 `std::cout` 快得多…
- 然而 `printf` 不是类型安全的，即类型无法与 `%d` 等匹配检查。
- 而且 `printf` 不能自定义，但流可以重载 `operator<<`。
- 既然 C++ 如此强大，为什么不能有类型安全、可定制、方便且快的函数？
- 这就是 `std::format`！
- 定义在 `<format>`。
- 你可以 `std::cout << std::format("a={},b={},c={}\n",a,b,c);`
- C++23 引入 `std::print`，它也利用 `format`。

# Format

### • `std::format` 甚至比 `sprintf()` 更快！

- 对 msvc，这需要例如 `/utf-8`，因为：
- Release，x64，msvc：

# Format

### • 那么，先告诉你如何使用这些 API。

- 最基本的形式像 `std::format("a={},b={},c={}\n",a,b,c)`。
- 要输出 `{` 或 `}`，需要使用 `{{` 或 `}}`。
- 你也可以显式指定顺序，例如 `std::format("c={1},a={0},c={1}\n",a,c)`。

### • 除此之外，你可以在 `:` 后写格式说明符，例如 `c={:xx}`

### 或 `a={0:xx}`（`0` 是顺序）。

# Format

### • 格式形如 fill – align – sign - # - 0 – width -

### .precision – L – type，各部分都是可选的。

- `fill`、`align`、`width` 用于填充额外字符以保持固定宽度。
- `width` 表示总字符数（包含元素本身）。
- `fill` 表示用来填充的字符（默认空白）。
- `align` 可以是左(`<`)、右(`>`)或居中(`^`)（默认左对齐，整数与浮点默认右对齐）。
- 例如：`std::format("{:0^8}", 1)` 得到 `00010000`，`std::format("{:08}", 1)` 得到 `00000001`。
- 注 1：若元素本身长于 `width`（例如这里传入 `10000000`），这些说明符不起作用。
- 注 2：若只给一个数字，则视为 `width` 而不是 `fill`。
- 注 3：对大多数东亚字符与许多 emoji，`width` 计为 2 而不是 1。

震惊，易经六十四卦出现在了C++标准中（

# Format

- `sign`：可以是 `-`（默认，即仅在数为负时显示符号，包含 `-0.0`）、`+`（总是显示符号）、以及空格（仅显示负号，但非负数前会填一个空格）。
- 例如：`std::format("{:0^+8}", 1)` 得到 `000+1000`，`std::format("{:+8}", 1)` 得到 `+1 `，`std::format("{: }", 1)` 得到 ` 1`（前导空格）。
- `type`：如何把元素显示为多种形式。
- 整数：`b`/`B`/`d`/`o`/`x`/`X`；默认 `d`（十进制）。默认不输出前缀。
- 特别地，`bool` 另有 `s`（默认），会得到 `true`/`false`；
- `char`/`wchar_t` 另有 `c`（默认），会输出字符；自 C++23 另有 `?`，会输出原始字符（例如 `'\n'` 而不是换行）。
- 浮点：`e`/`f`/`g`/`a`/`E`/`F`/`G`/`A`；默认 `g`（general）。
- 与 `std::chars_format::scientific`/`fixed`/`general`/`hex` 相同，而 `e`/`f`/`g`/`E`/`F`/`G` 的默认精度参数为 6。

还记得 `general` 下精度的特殊情况吗？

# Format

- 字符串（包含 C 风格字符串、`std::string` 等）：`s`；默认 `s`。
- C++23 增加 `?`，输出转义字符串（也会输出两侧的 `""`）。
- 指针：`p`；默认 `p`。与 `reinterpret_cast<std::uintptr_t>(ptr)` 相同。
- 会额外输出 `0x`。
- C++26 增加 `P`，强制十六进制字母大写。
- 例如：`std::format("{:m^+8x}", 32)` 得到 `mm+20mmm`；`std::format("{:e}", 32.F)` 得到 `3.200000e+01`。
- `#`：替代形式（alternative form）。
- 对整数且类型为 `x`/`X`/`o`/`b`/`B`，会添加前缀 `0x`/`0X`/`0`/`0b`/`0B`。
- 对浮点，总是显示小数点（例如 `42.f` 默认是 `"42"`，带 `#` 会得到 `"42."`）。
- 特别地，对显式 `#g`/`#G`，会显示所有零，例如 `42.0000`。
- 例如：`std::format("{:m^#8x}", 32)` 得到 `mm0x20mm`。

`std::format("{:+06}", 120)` -> `+00120` `std::format("{:0>+6}", 120)` -> `00+120`

# Format

- `.precision`：对浮点与字符串有效。
- 对浮点，与 `to_chars` 相同，即小数点后的位数。
- 对字符串，是输出的最大字符数。
- 例如：`std::format("{:.4e}", 32.F)` 得到 `3.2000e+01`。
- `0`：在符号与前缀之后对整数与浮点填 `0`（与 `fill` 不同！），且未指定 `align` 时生效。
- `L`：对整数与浮点按当前 `locale` 应用本地化信息（你也可以把 `std::locale` 作为第一个参数传入以指定它）。

### • fill – align – sign - # - 0 – width - .precision – L – type。

- 若你非常熟悉 `printf` 的格式，或其它语言的格式化，就很容易理解 C++ 的格式化。

### • 当格式字符串无效时，编译会失败；若内存不足，会抛出

### `std::bad_alloc`。

更灵活地，`width` 与 `precision` 可在运行时决定，即你可以使用 `std::format("{:{}.{}e}", 32.F, 3, 10)`（顺序由左花括号出现位置决定）或 `std::format("{0:{2}.{1}e}", 32.F, 10, 3)`，二者都等价于 `std::format("{:10.3e}", 32.F)`。

# Format

### • 有了格式字符串，我们可以轻松使用 `std::format("xx", yy)`

### （或 `std::format(locale, "xx", yy)`，或宽字符串版本）。

- 它生成 `std::string`，但我们可能已准备好缓冲区以避免动态分配（就像用 `to_chars` 代替 `to_string`）。
- 然后你可以使用 `std::format_to(OutIt, …)`，其中 `…` 与 `std::format` 相同，会把字符串输出到 `OutIt`。
- 你必须确保目标区间足够大；所需大小可用 `std::formatted_size(…)` 得到。
- 当然你也可以用 `std::back_inserter`。格式函数甚至为此做了优化：不会逐个插入而是一次插入整体。
- 若无法保证缓冲区大小，可用 `std::format_to_n(OutIt, n, …)`，若超过 `n` 会截断。
- 注意：这两个函数都不写入空终止符！

只能直接传入；如何在移动语义里做我们会以后再讲！

# Format

### • `std::format` 只接受可在编译期确定的格式字符串；若我们想要用户自定义的格式

### 字符串呢？

- 自 C++26 起，可以使用 `std::runtime_format`。
- 格式字符串可能无效，因为它无法在编译期确定。
- 因此可能抛出 `std::format_error`。

注意 VS2019 应更新到最新版本，否则即便编译期检查通过，`std::format` 仍可能抛出 `std::format_error`。

# Format

- C++26 之前，可以使用 `std::vformat(_to)`…
- 类似地有 `std::vprint(_unicode/nonunicode)` 对应 `std::print`。
- 参数略有变化，即应使用 `std::make_format_args`。
- 例如 `std::string fmt{ "{}, {}\n" }; std::vformat(fmt, std::make_format_args(a, b))`。
- 但这很危险！
- 原因：`std::make_format_args` 存储引用。
- `auto args = std::make_format_args(std::string{"Dangling"}); std::vformat(fmt, args)` => 临时字符串已经析构。
- 因此自 C++26 起右值也会被拒绝，见 P2095R2。
- 总之，自 C++26 起我们不应再自己调用 `vformat`。它只供 `<format>` 实现者用于“减少代码膨胀”。
- 我们会在模板一讲里介绍这种技巧！

# Format

### • C++23 强化了 `std::format` 的功能，即它可以格式化

### 一个 range。

- 是的，它省去了 `for(const auto& ele : vec) std::cout << ele << ' ';` 的负担。
- 默认情况下，range 输出为 `[…]`（即 sequence）。
- 例如 `std::vector v{1,2,3}` 得到 `[1, 2, 3]`，`std::vector<std::vector<int>> v{{1,2},{3,4}}` 得到 `[[1, 2], [3, 4]]`。
- 某些 range 有特化：
- 对容器适配器（即 `stack`、`queue`、`priority_queue`），它们不提供迭代器因此以前很难打印；现在也支持 `std::format`。
- 对 `std::vector<bool>`，代理类型以前不支持输出。
- 对 `std::pair`/`std::tuple`，你会得到 `(xx, yy, …)`。
- 对关联容器，你会得到 `{xx:yy, aa:bb,…}` 或 `{xx, aa}`（取决于 `map`/`set`）。

# Format

- 特别地，若 range 的元素是 `char`/`string`，会输出转义字符/字符串（即如同 `{:?}`）。
- 你可以用 `std::format_kind<R>` 检查 range 如何被格式化；会得到 `std::range_format::xx`，其中 `xx` 为 `disabled`/`map`/`set`/`sequence`/`string`/`debug_string`（即转义字符串）。
- 若你熟悉 Python，你可能会很高兴它们相同！

### • 当然，你可以为元素与 range 添加格式说明符。

- 对 range，类型说明符只有 5 种选项：
- `s`/`?s`：仅对字符串的 range 有效，输出字符串/转义字符串。
- `m`：仅对大小为 2 的 `pair`/`tuple` 的 range 有效，输出为 `{k1: v1, k2: v2, …}`（类似 `map` 的元素）。
- `n`：去掉外围包裹，例如 `[xx, yy]` 变成 `xx, yy`（以便自定义括号）。
- `nm`：组合 `n` 与 `m`，即输出为 `k1: v1, k2: v2, …`。
- 所以例如可以用 `std::format("{:n}", v)`。

# Format

- 若要指定元素的格式，可以使用多个冒号，即 `std::format("{:n:x}", v)`。
- 若元素仍是 range，就继续指定；总之，每个冒号指定一层元素的格式。
- 例如，对 `vector v{vector{'a'}, vector{'b', 'c'}};`，若你想以十进制扁平输出，可以用 `"{:n:n:d}"`。
- 通常你会得到 `[['a'], ['b', 'c']]`；
- 第一个 `n` 表示去掉 `v` 的 `[]`，即得到 `['a'], ['b', 'c']`。
- 第二个 `n` 表示去掉 `v` 的元素的 `[]`，即得到 `'a', 'b', 'c'`。
- 最后的 `d` 表示把字符转为十进制数值，即得到 `97, 98, 99`。
- 注意对字符串元素，`"{::}"` 与 `"{}"` 不同，因为前者会禁用转义字符串输出。
- 最后，range formatter 也支持 `fill`、`align`、`width` 说明符，因此完整可用 fill – align – width – type。
- 例如 `"{:*^14n:n:d}"` 会得到 `**97, 98, 99**`。

# Print function

### • 借助 `format`，C++23 在 `<print>` 中引入 print 函数。

- `std::print("{}", v)`，或 `std::println("{}", v)` 额外打印一个 `'\n'`。
- C++26 增加无参的 `std::println()`，只输出换行；三大实现都视其为 DR23。
- 它使用 C 的输出目的地（即 `stdout` 而不是 `std::cout`），但比 `printf` 更快。
- 因此当你解绑它们时，若同时使用 `std::cout` 与 `std::print` 可能有同步问题（我们会在流一节说明如何解绑）。
- 也可以把 `stream` 或 `FILE*` 作为第一个参数指定，从而输出到文件。
- 你也可以输出到 `std::cout`，仍比直接用 `std::cout <<` 更快。VS2019，debug

Mac Clang -O3：

# Print function

### • 此外，`std::print`/`println` 支持 UTF-8 输出。

- 但有点讽刺的是，它接受 `char[]` 而不是 `char8_t[]`。
- `std::print("\U0001F449")` √
- `std::print(u8"\U0001F449")` ×（编译错误）。
- `std::print("{}", u8"\U0001F449")` ×（编译错误）；
- `char8_t[]` 不支持 format。

### • 当输出流出错时它们可能抛出 `std::system_error`，

### 或 `std::(v)format` 的其它异常。

- 特别地，若 Unicode 字符串无效，则为 UB（但鼓励库实现用例如 `assert` 诊断）。

C++26 接受两个提案（很可能为 DR23）使 `std::print` 更快（20% ~ 200%）。见我的分析文章！

# User-defined format

### • 还有一件事… 流可以重载因此比 `printf` 更方便；那我们如何为自定义

### 类型定制 format？

### • 本质上，解析格式分两个阶段：

- Parsing：formatter 应解析 `{}` 里的内容，并记录必要状态。
- Formatting：根据解析记录的状态，从后向前插入字符串。

### • 因此，自定义格式同样需要这两个方法。

### • 下面用输出作用域枚举名来举例！

# User-defined format

### • 你需要特化 `std::formatter<T>`。

- 先记住即可，我们以后会讲模板特化。
- 然后定义 `constexpr auto parse(const std::format_parse_context&)`。
- 你可以把参数看作方法严格受限的 `std::string_view`；迭代器可用 `.begin()`/`.end()` 取得。
- 迭代器类型实际是 `std::format_parse_context::const_iterator`。
- 对 `{:…}`，它指向 `…}`（右花括号仍在！）。
- 对 `{:}`/`{}`，对 libstdc++ 指向 `}`，对 MS STL 则是空 `string_view`（可看作指向 `nullptr` 且大小为 0 的视图）。
- `parse` 的返回类型也是 `const_iterator`，指定后续解析继续的位置。
- 确切地说，你通常应返回 `context.end() – 1`；若收到的 `string_view` 为空，则返回 `context.end()`。
- 否则会编译错误（或对 `vformat()` 抛出 `std::format_error`）；你可以利用这点检查用户是否输入多余说明符。

# User-defined format

- 我们假设只接受说明符 `'x'` 或 `'s'`，前者显示颜色十六进制 id（即 `#rgb`，例如纯红为 `#FF0000`），后者（默认）显示字符串（例如 `"Red"`）。
- 啊太长，直接看代码！
- 解析时我们先保存指定状态。

默认为 `'s'`。

为跨平台能力做双重检查。

应为 `context.end()`
- 1, otherwise compile error.

- 注意 `constexpr` 在编译期调用有许多限制，例如你不能打印 `[begin, end)` 来查看 context 内容。
- 你可以在某种条件下抛异常（这里 `type != 'x' && type != 's'`），但若条件满足会编译错误（因为异常是运行时事物！）。
- 总结：
- `{}`、`{:}`、`{:x}`、`{:s}` 都可以。
- 编译错误（或对 `std::vformat` 抛异常）：
- `{:c}`、`{:11}`，因为我们在 `type != 'x' && type != 's'` 里检查。
- `{:x11}`，因为返回的 `it != context.end() - 1`。

# User-defined format

### • 解析之后，我们需要用保存的状态填充字符串。

- 这在运行时完成，因此不需要 `constexpr`；但保存的状态不应被改变，因此要加 `const`。
- 也就是说，你需要 `auto format(const T&/T, auto& context) const { … }`，其中 `T` 是对象类型（这里是 `Color`）。
- 你可以用 `.out()` 取得输出迭代器，需要在其后追加新内容；函数应返回新的迭代器。

你可以用 `std::format_context& context` 作为参数以帮助 Intellisense，全部完成后可改成 `auto&`。

# User-defined format

### • 此外，我们需要默认分支，因为用户可能用例如

### `Color{0x1000}` 显式创建枚举！

- 注意我们有 `White = 0xFFFFFF` 因此合法。
- 还记得吗？作用域枚举的取值范围是 `(1 << (MSB(MaxEnum) + 1))- 1`，例如若只使用 `{R, G, B}`，则允许的最大值是 `(1 << (MSB(B)+1))`
- `1 = (1 << (MSB(2) + 1)) - 1 = 4 – 1 = 3`；否则 UB。
- 所以：

# User-defined format

### • 结果类似：

- 注意 `context` 还有其它成员函数（可选，很少用）：
- `.advance_to(newIt)`，使得下次调用 `.out()` 时返回 `newIt`。
- 标准库在你返回新迭代器后会用它。
- `std::format_parse_context` 也有类似的 `.advance_to`。
- `.locale()`。
- `.arg(size_t id)`，返回第 `id` 个参数；它其实是 `std::basic_format_arg`，可看作带 `std::variant<…>`。有兴趣可见此处。

# User-defined format

### • 注 1：现在你可能明白为什么它能比 `printf` 更快！

- 它尝试在编译期解析，只在运行期填充字符串。
- C 与 C 函数无法利用编译期求值。

### • 注 2：你可以在 `parse` 里把 context 字符串存到成员里，

### 然后在 `format` 里打印以调试，这样就省去无法在 `parse` 里输出的麻烦。

- 例如：

### • 注 3：一般参数其实是

### `std::basic_format_context<T>` / `std::basic_format_parse_context`

### `<T>`，也可以支持例如 `std::wstring`；若你需要，

### 参数类型用 `auto&` 与 `const auto&` 也可以。

# User-defined format

### • 注 4：也可以继承或拥有另一个

### `std::formatter<T>` 作为成员，从而复用其解析

### 或格式化。例如：

通过继承：`.parse()` 与 `int` 相同。

constexpr

通过成员：致谢：Nicolai Josuttis《C++20 - The Complete Guide》。

# User-defined format

### • 注 5：C++23 还增加 `std::range_formatter<T>`，用于被

### `std::formatter<Container<T>>` 继承以定制输出 `T` 的 range。

- 是的，你通常不应特化 `std::range_formatter<C<T>>`，而是让 `std::formatter<C<T>>` 以 `std::range_formatter<T>` 为基类。
- `std::range_formatter` 有 `.set_brackets(left, right)` / `.set_separators(sep)`，以及 `.underlying()` 取得元素的 `std::formatter`，还有 `.parse()`/`.format()`（因此继承后通常不必再写新的 `parse`/`format`）。
- 特别地，`pair` 与 `tuple` 不是 range，因此它们的 `std::formatter` 特化加入了这些新方法，且没有 `.underlying`。

# User-defined format

你会在模板一讲里明白为什么能这样写模板参数！

### • 例如：

从模板基类继承会使 `this->` 成为必需；原因也会在模板一讲中说明。

# Possible future of format

### • 如你所见，format 与 print 函数非常强大

### 且方便。

- 但仍有一些小地方可以改进。
- 1. 你不能为 `pair`/`tuple` 的元素指定不同格式。
- 2. 词汇类型（vocabulary types）的 format？

- 此外，我们只介绍了输出方法，输入方法在哪里？
- `std::scan` 在 2023 年于此提出，据说也更快更方便。希望它能进入 C++26。

注意 C++ 对时间与线程 id 有不同格式；讲到它们时会再提。

# String and Stream

### 流（Stream）

# String and stream

### • 流

- 流概览
- 输出流
- 输入流
- 双向流与流绑定
- 标准流

# Stream

### • Stream 是对可读写缓冲区的高级抽象；

### 无论底层实际是控制台、文件还是内存，用户通常

### 不必关心细节，只需把它看作

### 可连续读写的缓冲区。

### • 这是 C++98 里成功的 OOP 设计，但不幸的是控制台

### 相关（即 `std::(w)cout`/`(w)cin`）因其糟糕效率而不受许多人待见。

- 然而，学习其它流仍然值得（例如文件流与 `FILE*` 之间不应有巨大性能差距）。
- 也有 `<iomanip>` 可操纵输出格式，但建议直接使用 `std::format`/`print` 的格式说明符，因为它更高效、更友好，因此我们可能只粗略涉及 `iomanip`；若你能用 C++20，它也仅是可选项。

# Stream

### • 在继续之前，先谈谈操作系统中的 IO 体系。

- 你会在操作系统课里系统学习，这里只给简要视图。
- 粗略地说，IO 体系可分为以下部分：

你在 ICS 里学过并使用的（文件描述符、Linux 文件 API）。

Credit: Prof Jin Xin @ PKU

# Stream

### • 我们知道系统调用相对昂贵，因此若用户每次从文件取数据都走 syscall，

### 就会非常慢。

- 是的，这与磁盘存储类似；你需要内存里的缓存！
- 也就是说，我们准备一个缓冲区，即使用户还没需要，也先从文件读入大量数据到缓冲区，然后读取时只需调整指向缓冲区的指针。
- 一旦指针到达末尾，就需要再从文件装入新内容。
- 这给用户造成从文件连续读取的错觉。
- 因此空间局部性在 IO 中也很重要，可减少系统调用。
- 类似地，当用户写入流时，并不会立刻落到磁盘，而是先保存在缓冲区；写缓冲区满后，再把一切 flush 到磁盘。

“Flush 到磁盘”可能不严谨，因为它依赖 OS；也就是说，应用层的 flush 仍可能留在 OS 缓冲区，需要依赖 OS 的 API 才能真正落盘。

# std::endl v.s. ‘\n’

### • 了解这些之后，一个重要而基础的问题可以

### 解决：`std::endl` 与 `'\n'` 有何区别？

- 你的第一个 C++ 程序很可能是 `std::cout << "Hello, world!" << std::endl`。

### • `std::endl` 事实上是一种特殊的 IO 操纵符，含义是

### “输出换行，并 flush 流”。

- 也就是说，若你输出 `'\n'`，文件/控制台可能不会立刻收到，直到合适时机；`std::endl` 会强制立刻送达。
- 因此反复调用 `std::endl` 会显著变慢。
- 若你不想如此频繁 flush，建议使用 `'\n'`，并在真正需要时 `<< std::flush`。
- 流析构时也会自动 flush。注意还有 `std::cerr`，它类似 `stderr`（而不是你在 ICS 学过的 `stdout`/`stdin`）且无缓冲，因为它希望立刻向用户警告错误；若你想要带缓冲的，可以用 `std::clog`。

# Design overview

`std::num_get`/`num_put`/`num_punct` in locale will work for numbers `std::codecvt<CharT,char>` in locale will work. I/O manipulators will also work

`std::basic_streambuf <CharT, Traits>`

flush Formatted I/O *Stream buffer External device (`operator>>`/`<<`) sync

`std::basic_istream`/ Only for `CharT` and `CharT*`. `ostream<CharT, Traits>`

Unformatted I/O*: `>>`/`<<` `std::basic_streambuf*` is (`.write`/`.put`/read unformatted. `/get`/…)

# String and stream

### • 流

- 流概览
- 输出流
- 输入流
- 双向流与流绑定
- 标准流

# Output stream

### • 流有两种：输出流与输入流。

### • 对输出流，它继承自 `std::basic_ostream`。

- 它其实是模板，即 `<Char, CharTraits = std::char_traits<Char>>`。
- 格式化输出：使用 `<<`（有时叫“插入运算符”），如你对 `std::cout` 所做。
- 非格式化输出：不受 io manipulator 影响。
- 仅用于原始字符（及其数组）输出；返回 `*this` 的引用。
- `.put(ch)` 与 `.write(chArr, len)`。
- 它们类似 C 的 `putc()`、`putchar()`、`puts()`。
- 例如，`.write(fmtStr.data(), fmtStr.size())` 与无 manipulator 的 `<< fmtStr` 相同。
- 有时你可能想跳到其它位置（如同移动整个缓冲区里指针的位置）。
- 你可以用 `.tellp()` 与 `.seekp()` 取得或设置“写（put）”位置。

注意 `std::cout` 等对

# Output stream

`.seekp`/`.tellp()` 没有意义，行为依赖平台。

- `.tellp()` 返回、`seekp()` 接受 `std::basic_ostream<xx>::pos_type`，可看作有符号整数（其实是类 `std::streampos`）。
- 此外，可以用相对位置 seek，即 `.seekp(off_type off, std::ios_base::seekdir dir)`。
- `std::ios_base::seekdir` 有三个全局常量值（不是枚举类型！）。
- `beg`、`end` 与 `cur`。
- `off_type` 通常是有符号整数类型，因此可以前进或后退指针。
- 例如，`.seekp(-1, std::ios_base::end)` 把指针放到流最后一个字符；`.seekp(0,std::ios_base::beg)` 回绕到开头；`.seekp(2, std::ios_base::cur)` 把指针前进 2。

:Depend on implementation to use the one or the another or both. text :Called on condition of text.Output stream Pointer positioning Output functions functions

Locale functions tellp (out mode) seekp put write `operator<<` imbue flush For relative position For absolute position ostream_pubimbue pubsync iterator

sputc sputn Public functions pubseekoff pubseekpos

Called when buffer is full Stream buffer

overflow xsputn Protected functions seekoff seekpos ostreambuf imbue sync _iterator

### • `std::basic_streambuf` 也自动处理缓冲，因此

### 派生类不必关心“缓冲区何时满”。

- 通常派生类有默认内存缓冲区，用户可用 `.pubsetbuf()` 自行设置。
- 这会调用派生类的 `.setbuf()`，因此是否生效取决于派生类实现。
- 缓冲区抽象为三部分：`begin`、`curr`、`end`。
- 对输出，是受保护方法 `pbase`/`pptr`/`epptr`。
- 例如，`sputc` 会直接写入 `*pptr` 并前进 `pptr`。
- 仅当 `pptr == epptr`（即缓冲区满，无法再 put）才会调用虚函数 `overflow` 让派生类提供额外空间。
- 例如对文件，把所有内容 flush 到磁盘并通过 `setp` 把 `pptr` 回绕到 `pbase`。
- 若你想手动调整 `pptr`，可用 `.pbump(int off)`。

pbase Stream buffer epptr

pptr

Real buffer in memory

`[pptr,epptr)` 为空且可插入。`[begin, end)`

# Output stream

### • 流缓冲区可用 `.rdbuf()` 取得，返回

### `std::basic_streambuf*`。

- 它是带 protected 构造的基类，用多态访问实际缓冲区（例如文件流的 `std::basic_filebuf`）。
- 这便于你可能的缓冲区定制。
- 如前所述，对输出缓冲区，有几件事可用或可定制（这些函数是受保护的）：
- `.pbase()`/`.pptr()`/`.epptr()`：返回 begin/cur/end 指针。
- `.setp(pBeg, pEnd)`：把 begin/cur/end 指针设为 `pBeg`/`pBeg`/`pEnd`。
- `.pbump(int off)`：`cur += off`；`cur` 可以越出 `[begin, end)`。
- 以及可在派生类重写的虚函数…
- 注意除非明确说明，基类实现的虚函数通常无效果或只返回失败（如 `eof`），需要派生类（如 `basic_ifstream`）实现有意义的行为。

# Output stream

- `.overflow(int_type ch = Traits::eof())`：若 `ch` 为 `eof` 则什么都不做；否则把 `ch` 放到合适位置。
- 失败返回 `eof`，成功返回其它值。
- 这仅在 `cur` 到达/越过 `end` 时调用；你可能需要在 flush 后用 `.setp` 重新定位指针。
- `.xsputn(const Char* arr, std::streamsize cnt)`：把字符串输出到合适位置。
- 基类默认实现是循环调用 `.sputc`（并在需要时 `.overflow`）。
- 若你的实现能更高效地输出一大块，就直接实现 `.xsputn`；例如若字符串很长，可能只需输出大块字符串而不必在缓冲区里反复缓存再 flush。
- 返回成功写入的字符数。

最一般地，你可以用 `Traits::eq_int_type(ch, Traits::eof())` 判断 `eof`；但对大多数情况 `ch != Traits::eof()` 足够。

# Stream

### • 对输入与输出缓冲区，某些受保护定位操作可

### 被重写。

- `.seekoff(off_type off, seekdir, std::ios_base::openmode mode)`：按相对偏移 seek；缓冲区应把指针调到合适位置；返回 `pos_type` 的绝对位置，失败返回 `pos_type(-1)`。
- 输入/输出缓冲区由 `mode` 指定；若是 `std::ios_base::in` 则调整输入缓冲区；若是 `std::ios_base::out` 则调整输出缓冲区。
- 我们稍后会讲同时有输入与输出缓冲区的流，此时也允许使用 `in | out`（也是默认参数）。
- 通常也是惰性的，即仅当下次读超出缓冲区时才重新装入。
- 这由公共方法 `.pubseekoff()` 暴露。
- 注意 `.tellg()` 也使用此方法，即返回 `.pubseekoff(0, std::ios_base::cur)`。

# Stream

- `.seekpos(pos_type pos, openmode mode)`：按绝对位置 seek；缓冲区应调整指针；返回 `pos_type` 的绝对位置，失败返回 `pos_type(-1)`。
- 由公共方法 `.pubseekpos()` 暴露。
- `.sync()`：对输出流写回缓冲区，对输入流清空缓冲区。由 `.pubsync()` 暴露。
- 输入缓冲区是否立刻重新装入依赖实现。
- `.setbuf(Char* s, std::streamsize n)`：允许手动设置底层缓冲区；可用它设置更大缓冲区。返回 `this`。由 `.pubsetbuf()` 暴露。

# Output stream

我们会在作业里展示一个更有用但更复杂的例子。

# String and stream

### • 流

- 流概览
- 输出流
- 输入流
- 流状态
- 双向流与流绑定
- 标准流

注意：自 C++20 起，你只能 `>> char[N]` 而不能 `>> char*`，因为后者可能溢出。

# Input stream

### • 对输入流，它继承自 `std::basic_istream`。

- 格式化输入：使用 `>>`（有时叫“提取运算符”），如你对 `std::cin` 所做。
- 非格式化输入：不受 io manipulator 影响。
- 仅用于原始字符（及其数组）输入。
- `.get()`、`.get(ch)` 与 `.read(chArr, len)`。
- 它们类似 C 的 `getc()`、`getchar()`、`fgets()`。
- `.get()` 返回 `int_type`，而 `.get(Char& c)` 把结果写入 `c`。
- 为什么 `.get()` 不返回 `Char`？
- 可能已经没有字符可读（例如文件结束）！
- 此时返回 `Trait::eof()`，它可能不是合法的 `Char` 值。
- C 里的典型做法是返回 `EOF`（负整数），这就是 `getc()` 返回 `int` 而不是 `char` 的原因。
- 对 `.get(Char& ch)`，此情况下字符保持不变。
- 对 `.read(chArr, len)`，剩余字符可能少于 `len`。

# Input stream

- 有时你可能想跳到其它位置（如同移动整个缓冲区里指针的位置）。
- 你可以用 `.tellg()` 与 `.seekg()` 取得或设置“读（get）”位置。
- `.tellg()` 返回、`seekg()` 接受 `std::basic_ostream<xx>::pos_type`；也可以用相对位置 seek，即 `.seekg(off_type off, std::ios_base::seekdir dir)`。

### • 除此之外，输入流提供更多工具函数。

- 更多非格式化输入函数：
- `>> streambuf`（只有这个 `>>` 是非格式化的）。
- `.get(Char* buffer, std::streamsize count, delim = Char('\n'))`：读入缓冲区，直到 `delim` 或字符数达到 `count`。
- `.get(streambuf buf, delim = Char('\n'))`：读入输出流缓冲区，直到 `delim`。

# Input stream

- `.getline(Char* buffer, std::streamsize count, delim = Char('\n'))`：类似 `.get`，但它会吃掉 `delim`，而 `.get()` 会把 `delim` 留在流中（因此下次仍可读得 `delim`）。
- 若你想对 `std::string` 使用 `getline`，不应使用成员函数，而应使用 `std::getline(stream, std::string, delim = '\n')`。
- `.readsome(Char* buffer, std::streamsize count)`：从内存缓冲区最多提取 `count` 个字符（即无需从文件等重新装入）。
- `std::cin.readsome()` 可能填充也可能不填充缓冲区。
- `.ignore(count = 1, delim = Traits::eof())`：最多吃掉 `count` 个字符/直到遇到 `delim`。
- `.peek()`：取得下一个字符但不真正改变流状态（即下次 `.get()` 会得到 peek 的结果）。若流到达末尾返回 `eof`。
- 注意 `cin.peek()` 若没有下一个字符会阻塞而不是返回 `eof`。

# Input stream

- `.unget()`：尝试把最后提取的字符放回；`.peek()` 基本上等价于 `.get()` + `.unget()`。
- `.putback(ch)`：尝试把任意字符放回流。
- `.gcount()`：取得上一次非格式化输入函数实际读到的字符数。
- `.read(chArr, count)` 可能读到少于 `count` 个字符，你可以用 `.gcount` 取得实际长度！

# Input stream

### • 以及一个同步函数：

- `.sync()`：把缓冲区与底层序列同步；例如若文件被写入，输入流可用此方法重新装入。
- 再次注意，调用 `.sync()` 时缓冲区是立刻重新装入还是留到下次读取，由实现定义。
- 成功返回 `0`，失败返回 `-1`。

Input stream Pointer positioning Input functions functions (in mode)

tellg seekg peek ignore get(char) getline get read `operator>>` readsome For relative These functions may use any of four input public functions. position Notice that MS-STL uses sgetn only for `.read`. For absolute position istream_in_avail iterator

snextc sbumpc sgetc sgetn Public functions pubseekoff pubseekpos

Called when Called when buffer is empty buffer is empty Stream buffer

uflow underflow xsgetn Protected functions seekoff seekpos istreambuf_ showmanyc iterator

Input stream eback

Putback functions

Locale functions Real buffer in memory putback unget imbue sync

pubimbue pubsync

sputbackc sungetc Public functions gptr

Called when Called when `[gptr,egptr)` buffer is full Stream buffer buffer is full or contains `*(gptr-1) != ch`. content and is ready to read. pbackfail Protected functions egptr imbue sync

# Input stream

### • 全部列出：

- `.eback()`/`.gptr()`/`.egptr()`：返回 begin/cur/end 指针。
- `.setg(gBeg, gCurr, gEnd)`：把 begin/cur/end 指针设为 `gBeg`/`gCurr`/`gEnd`。
- `.gbump(int off)`：`cur += off`；`cur` 可以越出 `[begin, end)`。
- 以及许多可在派生类重写的方法：
- `.underflow()`：返回下一个字符并把指针调到合适位置。失败返回 `eof`。
- 这仅在 `cur` 到达 `end` 时调用；你可能需要在装入新内容后用 `.setg` 重新定位指针。
- `.uflow()`：`.underflow()` 然后把 `cur` 前进 1。
- 默认实现是调用 `.underflow()` 并递增 `gptr`。
- `.xsgetn(Char* arr, std::streamsize cnt)`：读入字符串到 `arr` 并返回成功写入的字符数。
- 默认：循环调用 `.sbumpc`（并在需要时 `.underflow`）。

# Input stream

- `.pbackfail(int_type ch = Traits::eof())`：当 `eback == gptr` 或 `gptr == nullptr` 或 `*(gptr–1) != ch`（即无法回退）时调用此方法；缓冲区应把字符放回（例如文件缓冲区可能需要向后移动来重新装入缓冲区）。失败返回 `eof`。
- `.showmanyc()`：`stream-how-many-character` 的缩写；返回底层序列估计剩余字符数（不确定则返回 0；基类默认如此）。
- `.in_avail()`：返回 `(end – cur)`，类型为 `std::streamsize`，表示无需重新装入即可立即取得的内容量。
- `.showmanyc` 在缓冲区空（即 `gptr == egptr`）时被 `.in_avail()` 使用；默认 0 表示未知。
- 我能想到的一个有用场景：
- 若我们自己实现 `TCP_streambuf`，并想知道系统缓冲区里立刻能读到多少字符。
- 于是你可以调用系统 API 得知，并由 `.showmanyc` 返回！

# Stream status

### • 我们发现流缓冲区会用 eof 表示失败；

### 但流本身似乎并不直接暴露它！

- 那我们如何知道这种失败？
- 流用流状态来表示！

### • 有三个位：`eofbit`、`failbit`、`badbit`。

- `std::ios_base::eofbit` 常用于表示流结束。
- `std::ios_base::failbit`：常用于解析错误，例如 `cin >> SomeFloat` 但输入了字符 `'a'`；或打开/关闭文件失败。
- `std::ios_base::badbit`：发生不可恢复错误。
- 你可以用 `.rdstate()` 取得状态标志，并用例如 `s & eofbit` 测试，用 `.clear(states = goodbit)` 设置标志，用 `.setstate(states)` 添加标志（等价于 `.clear(rdstate() | states)`）。
- 特别地，`0` 或 `std::ios_base::goodbit` 表示无错误。

若你需要/有兴趣了解流缓冲区失败时这些位何时被设置，可见此处。

# Stream status

### • 然而，直接操作位通常不方便，因此也提供

### 直接的方法：

- `.good()`/`operator bool`：返回流是否良好。
- 结合所有输入/输出方法都返回流自身这一事实，你就可以写 `while(std::cin >> xx)`！
- `operator!` 也被重载。
- `.eof()`：返回是否设置了 `eofbit`。
- `.fail()`：返回是否设置了 `failbit` 或 `badbit`。
- `.bad()`：返回是否设置了 `badbit`。

### • 此外，若你想在错误状态发生时抛出

### `std::ios_base::failure`（派生自 `std::system_error`/`runtime_error`），

### 可使用 `.exceptions(states)`。

- `.exceptions()` 可取得异常状态；默认无。

# Stream status

### • 当流不良好时，任何输入/输出都不会做事；

### 因此若你想继续正常使用流，别忘了 `.clear()`！

- 某些操作（即 `putback`/`unget`/`seekg()`）也可能作为副作用重置 `eofbit`。

### • 最后：所有流只可移动，可移动构造且可交换。

- 我们会在下一讲讲移动语义，现在只需知道它不可拷贝即可。

# String and stream

### • 流

- 流概览
- 输出流
- 输入流
- 双向流与流绑定
- 标准流

# Bidirectional stream

### • 流也可以通过 `std::basic_iostream` 同时输入与输出。

- 它在 `streambuf` 里只有一块缓冲区，但有两套 begin/cur/end 指针。
- 它拥有 `istream` 与 `ostream` 的全部方法，例如 `.tellg`/`seekg`/`tellp`/`seekp()`。
- 它们是否共享同一位置由派生类决定。例如文件流共享，而其它流不一定。

# Stream linking

### • 除此之外，你还可以把一个 `istream` 与一个 `ostream` 绑定，使得

### 当 `istream` 读取时，`ostream` 会自动调用 `flush()`。

- 通过 `.tie(basic_ostream*)`，返回原先绑定的 `basic_ostream*` 指针。
- 由于 `flush` 与 `sync` 都只调用 `pubsync()`，它也间接同步输入缓冲区。

- 注 1：用 `.tie()` 可直接取得原绑定，用 `.tie(nullptr)` 解绑。
- 注 2：你也可以把输出流绑到另一个输出流，但这似乎很少用。

# Stream linking

### • 双向流与流绑定的区别在于后者是两个独立流，使用两套状态、

### 两个 locale 等。

### • 最典型的例子是 `std::cin` 与 `std::cout`；它们会自动绑定（即输入会 flush 输出）。

- 它们还有 `.sync_with_stdio()`，控制在使用 `std::cin`/`std::cout` 时是否还能用 `printf`/`scanf`。
- 因此在一些 ACM 题解里你可能看到 `std::cin.sync_with_stdio(false)`、`std::cout.sync_with_stdio(false)` 与 `std::cin.tie(nullptr)`。
- 但真实项目里通常不用，因为很难保证没有外部库使用 `printf`。
- 而 C++23 的 `std::print` 已经足够快！

# String and stream

### • 流

- 流概览
- 输出流
- 输入流
- 双向流与流绑定
- 标准流

# Standard streams

### • 有三种预定义标准流及其缓冲区，

### 继承自 `std::basic_istream`/`ostream`/`iostream`/`streambuf`。

- 文件流：`std::basic_ifstream`/`ofstream`/`fstream`，缓冲区为 `std::basic_filebuf`。定义在 `<fstream>`。
- 字符串流：`std::basic_istringstream`/`ostringstream`/`stringstream`，缓冲区为 `std::basic_stringbuf`。定义在 `<sstream>`。
- 还有 `<strstream>`，但在 C++98 弃用、在 C++26 移除并由 span stream 取代！
- Span stream（C++23）：`std::basic_ispanstream`/`ospanstream`/`spanstream`，缓冲区为 `std::basic_spanbuf`。定义在 `<spanstream>`。

### • 以及与其它流配合使用的流：

- 同步输出流：C++20，`std::basic_osyncstream`，缓冲区为 `std::basic_syncbuf`。定义在 `<syncstream>`。

# File stream

### • 顾名思义，文件流的底层字节序列是文件。

- 因此构造新文件流需要路径与打开模式。
- 路径可以是 `std::string` 与 `const char*`；你也可以使用 `std::filesystem::path`（表示路径的规范方式）或其 native string view。我们以后会讲。
- 打开模式在许多语言与平台 API 中广泛使用，例如 C、Python、Go 等，名称或功能略有差异。
- 我们见过 `in`/`out`，它们分别是 `ifstream`/`ofstream` 的默认模式；它们指定只读/可写文件；`out` 还会在文件不存在时创建文件，并在文件存在时截断（即删除全部内容）。
- 然而，若使用 `in | out`（`fstream` 的默认模式），截断不会自动发生；需要显式加 `| trunc`。
- 此外，若你想在文件不存在时失败而不是创建新文件，自 C++23 起可使用 `noreplace`（也叫“独占模式”）。

# File stream

- 还有两个说明符影响序列指针的初始位置。
- `ate`：打开文件后立刻把指针 seek 到文件末尾。
- 常与 `out` 一起使用，使其不截断并可能先在末尾追加内容。
- `app`：每次写入前都把指针 seek 到文件末尾。
- 与 `ate` 的区别是：`ate` 只 seek 到末尾一次，而 `app` 总是在末尾追加（即使 `.seekg()` 也不影响它）。

### • 此外，文件可以文本模式（默认）或二进制

### 模式（`| binary`）打开。

- 例如，在 Windows 上，换行会输出 `"\r\n"`（CRLF），尽管你只写了 `'\n'`。
- 若以二进制模式打开，则只写入 `'\n'`。
- 此外，某些不可打印字符在文本模式可能出错。

# File stream

### • 注意：这是二进制模式与文本模式之间的唯一区别！

- 在二进制模式下，你仍可使用 `operator<<`（例如 `fout << 1`）。参数会被格式化成字符串再写入字符！
- 由于数字都可打印，因此与文本模式相同。
- 若要写入“真正的二进制数据”，你需要使用：

- 于是 `a` 是字符数组，例如 `0x00, 00, 00, 0a`（取决于字节序）。
- 那若在文本模式使用上面代码呢？
- `0x00` 不可打印，某些系统可能出错。
- `0x0a` 是换行，因此在 Windows 会被转换成 `\r\n`！
- 这就是需要二进制模式的原因。

见 Windows doc1 与 doc2。

# File stream

### • 许多文件应按纯字节序列读取，

### 例如某些压缩文件、视频/图像等。

### • 类似地，你可以用 `.read()` 直接读字节。

- 从理论上 `.read(reinterpret_cast<char*>(&obj), sizeof(obj))` 然后使用 `obj` 因生命周期问题非法，但实践中通常可行。
- 绝对 OK 的做法是：用正确对齐创建字节数组，读入缓冲区，然后 `start_lifetime_as<Object>`。
- 对对象 `.write(reinterpret_cast<const char*>(&obj), sizeof(obj))` 绝对 OK，因为按字节序列读取是合法的。
- 还记得吗？只有“平凡可复制”的对象才能从同一类型对象得到的字节块安全拷贝。打开模式小结。

# File stream

### • 若文件打开失败，例如 `in` 模式下文件不存在，或

### 你没有权限以某种模式打开文件（例如对你只读），

### 则会设置 `failbit`。

- 你可以用 `.is_open()` 显式检查文件流是否关联了文件。
- 你也可以用 `.close()` 关闭文件，但析构函数会自动做。
- 除非你想在同一流对象上打开新文件（通过 `.open(…)`），否则不必显式调用 `.close()`。
- 自 C++26 起，你也可以调用 `.native_handle()` 取得文件流的原生句柄。

# Codecvt

### • 最后我们需要提一提流里的 codecvt…

- 转换发生在去往外部设备时，即内存缓冲区的编码并未改变。
- 文件流里的 codecvt 总是 `std::codecvt<CharT, char>`，即写入时转换为 `char`。
- 注意 MS-STL 在 `.pubsetbuf(CharT*, size)` 时会取消 codecvt，但 libc++/libstdc++ 不会。
- 默认 `locale` 与 `std::locale::global()` 相同，默认是 C locale（即宽字符 -> `char` 总是失败）。
- 文件流的二进制模式仍会经过 codecvt。

# Codecvt

### • 例如，若你想用 `std::wfstream` 输出 `L"你好"`

### 为其原生编码（例如 Windows 上的 UTF-16）。

- 默认会失败，因为 C locale 做不到。
- 切换到二进制模式仍失败，因为二进制模式仍经过 codecvt。
- 在 MS-STL 上若 `pubsetbuf()` + 二进制模式可工作，这会禁用 codecvt 并能正常写入对 `char` 而言“不可打印”的字符。
- 但在 libstdc++/libc++ 上不行。
- 别忘了写 BOM！
- 要跨平台，我们需要自己写 `codecvt` + 二进制模式才能成功。
- 我们把它留在作业里；请查看！

# String stream

### • 字符串流的底层序列只是内存。

- 你可以用任意字符串类型构造它，并可选打开模式（是的，你可以把它当作内存里的文件，没有 flush/sync）。
- 字符串流在 `overflow` 时会重新分配。
- 你可以用 `.str()` 拷贝出来，或用 `.str(newStr)` 替换为新字符串。
- 自 C++20 起，也可以 `.view()` 取得指向它的 `string_view`。
- 但注意未来重分配可能使该视图失效！
- 例如：

还记得吗？`tellx()` 返回 `std::streampos`，可看作无符号整数但其实是一个类。应减去另一个流尺寸以得到 `std::streamoff`，这才是真正的有符号整数。

# Span stream

### • 字符串流总会创建新的底层字符串；但

### 若我们想自己提供缓冲区呢？

- 更改直接作用在缓冲区上，不必担心输出超过缓冲区（只会截断而不是重新分配）。
- 此情况下会设置 `badbit`。
- 然后你需要确保缓冲区生命周期 ≥ `spanstream` 操作它的作用域。
- 构造函数：使用 `span<CharT>` + 模式。
- `rdbuf()->pubsetbuf(CharT*, len)`（或移动构造/赋值）可重置它。
- 取得视图：`.span()`，得到 `span<CharT>`（而不是 `basic_string_view<CharT>`！）
- 注意：若模式包含 `out`，则返回 `[pbase, pptr)`，即已写入部分。
- 否则返回整个缓冲区。字符串流与 span 流没有“外部设备”；`flush` 与 `sync` 什么都不做，且 `codecvt` 永远不会工作。

# Span stream

# Synchronized stream

### • 若你写过多线程程序并使用

### `std::cout`，你可能发现输出乱了…

- 例如两个线程里 `std::cout << "a=" << a << "c=" << c;` 可能以任意交错出现（例如 `"a=a=1c=c=22"`）。
- 以前你可能不得不创建 `std::ostringstream`，再 `std::cout << .str()` 以整体输出。
- `std::format` 也有助于消除该问题。
- 然而这只对标准输出成立，因为它有内部全局锁。
- 其它流，例如你创建的文件流，仍可能数据竞争，因为两个线程可能同时操作文件指针。你需要手动加锁！
- `std::osyncstream` 就是为此而生！

# Synchronized stream

### • 你需要创建两个 `std::osyncstream`

### 每个线程各一个。

- 你可以认为 `osyncstream` 准备了额外缓冲区；当你显式使用 `std::flush_emit` 时，缓冲区会无数据竞争地输出到附加流（此处的 `fout`）。
- 这是通过对相同缓冲区地址的流共享 mutex 完成的；目前只有 OS API 能做到。
- 若你想/不想每次 `std::flush` 都 emit，可使用 `<< std::emit_on_flush` / `<< std::noemit_on_flush`。
- 流析构时也会 emit。
- 你也可以用 `.get_wrapped()` 取得附加流的缓冲区。

# String and stream

### • 流

- 流概览
- 输出流
- 输入流
- 双向流与流绑定
- 标准流

## • IO manipulator*

# IO manipulator*

### • 如前所述，你现在可以用 `std::format` 做格式化，因此

### 大多数 IO manipulator 没那么有用（尤其输出）

### 我们只给简要概览。

- IO 格式 manipulator 令人头疼的一点是许多会保持操纵状态，影响后续所有输出。
- 例如 `std::cout << std::hex << 16; std::cout << 32;` 也会让 `32` 以十六进制输出。
- 若要恢复状态，你要么记住改了什么再改回去（例如 `std::cout << std::dec << 32`），要么在改状态前 `auto flags = stream.flags()`，结束时 `stream.flags(flags)` 恢复全部设置。

### • 首先，它们定义在 `<ios>` 与 `<iomanip>`。

# IO manipulator*

以及另外两个。

# IO manipulator*

### • 对那三个有多种选项的 manipulator，默认值

### 是 `left`/`dec`/`fixed`。

- 你也可以例如 `<< std::resetiosflags(std::ios::basefield/ adjustfield/floatfield)` 恢复默认。
- 也有 `std::setiosflags`；`<< std::setiosflags(std::ios::fixed)` 基本上与 `<< std::fixed` 相同。

### • 对 fill-align-width-type，可以使用：

`setbase` 只接受 8/10/16。`setw` 没有持久副作用！

# IO manipulator*

### • 以及一些与 locale 相关的 manipulator：

### • 以及最后一个 `std::quoted(str, delim='"', escape='\\')`。

- 它会在两侧添加 `delim`，并对 `str` 中等于 `delim` 的字符添加转义。
- 这在 C++23 基本上可用 `?` 类型替代。

# IO manipulator*

### • 最后是非格式化 manipulator：

# String and Stream

### 正则表达式（Regular Expression）

# String and stream

### • 正则表达式

- 正则语法
- C++ 对正则的支持

# Regular expression

### • 我们学过用 `std::search` 里的 searcher 做模式匹配。

- 但模式匹配非常严格；你必须几乎完全匹配整个模式字符串。
- 你无法表达诸如“至少匹配 10 个 `a`”的含义。
- 正则表达式（Regex / Regular expression）就是为此而生！
- 它规定一套语法以某种模式进行匹配。
- 对经典正则，它可以转换为有限自动机（FA，有限自动机），严格为 𝑂(𝑛)。
- 你将在编译原理课学习经典正则与 FA 的理论。
- KMP 本质上是一种 FA。
- 然而经典正则也有局限：
- 其语法受限，仍只能做简单工作。
- FA 可能非常大，其构造通常也需要 𝑂(𝑟)（最坏 2^𝑟、𝑂(𝑟²)，其中 𝑟 为正则长度），也消耗大量空间与时间。
- 编译器的模式固定，因此那里的正则构造时间只是预处理，之后就不那么重要。

# Regex

FA

### • 所以大多数正则引擎不采用基于 FA 的方式；而是

### 使用回溯方式。

- 这能极大扩展正则的功能。
- 然而语义与经典正则略有不同，𝑛 更糟糕的是时间复杂度可能差到 𝑂(2^𝑛)（甚至更高，甚至永不结束）。
- 形式上，它甚至无法完全被上下文无关语言描述，而后者几乎能表达像 C 这样的编程语言。这里是某种上下文相关语言。详见此处。

### • 一些用于练习或实验正则的有用网站：https://regex101.com/

- https://regex101.com/，会分析某个正则如何（失败）匹配字符串以及完成匹配需要多少步。playground
- https://regexlearn.com/zh-cn，学习基础正则并在 playground 测试。
- …

# Regex

### • 经典正则有这些语法：

- 选择（或）：`|`；例如 `a | b` 会匹配 `a` 或 `b`。
- 连接：例如 `a` 匹配 `a`，`b` 匹配 `b` → `ab` 匹配 `ab`。
- Kleene 闭包：`*`，表示匹配零次或多次，例如 `a*` 会匹配 𝜖（即空串）、`a`、`aa`、`aaa`、…
- 这三个运算符的优先级是 `*` > 连接 > `|`，例如 `ab*` 表示 `a` 后跟若干个 `b`。
- 组合：`()`，用于提高某组的优先级，例如 `(ab)*`。

### • 还有一些语法糖：

- 正闭包：`+`，匹配一次或多次。
- 零或一：`?`，匹配零次或一次。
- 次数：`a{1,3}` 匹配 1 到 3 次；`a{3,}` 至少 3 次。
- 字符类：`[atyz]` 表示 `a|t|y|z`；`[^atyz]` 表示除 `a|t|y|z` 外所有字符；`[a-z]` 可表示 `a|b|…|z`。

# Regex

### • 除了这些扩展，还有一些预定义正则

### 简写。

- `\d`：所有数字，等价于 `[0-9]`。
- `\w`：所有英文字母、数字与下划线，即 `[a-zA-Z0-9_]`。
- `\s`：所有空白字符，即空格、制表符与 `\n`。
- `\D`、`\W`、`\S`：`[^0-9]`、`[^a-zA-Z0-9_]`、所有非空白字符。
- `.`（对，就是一个点）：任意字符。
- 当然，你需要转义 `'\'` 才能表示字面反斜杠；考虑到 C 字符串本身也要转义，你需要 `"\\\\"` 实际才是 `"\\"`，从而在正则里表达 `'\'`。
- 因此原始字符串对正则极其重要；你需要 `R"(\\)"` 才能做到。
- 特别地，斜杠也需要转义，即你需要 `R"(\/)"`。
- 对不可打印值，可用八进制 `'\0'` 与十六进制 `'\x…'`。

# Regex

- 注意不同语言可能提供不同正则功能；这里教的基本都被广泛支持；若你想用其它特性，请查阅该语言文档。

### • 一些例子：

- 邮箱：`R"([\w\.\-]+@[\w\-]+\.[\w\.\-]+)"`
- 即 `xx.yy…@aa.bb…`，且 `@` 后至少有一个 `.`。
- 十六进制 rgb：`R"(#?[\da-fA-F]{6})"`
- 即可选的 `#` 开头，后跟 6 个十六进制数字。
- 合法 Linux 文件绝对路径：`R"((\/[^\/\x00]+)+)"`
- 即文件/目录名不能包含反斜杠或 `'\0'`。
- 这里不考虑纯目录路径，例如 `"/a/b/"` 或 `"/"`。

# Regex

### • 回溯正则会扭曲经典正则的原始语义。

- 它不会创建 FA（因此空间消耗与准备时间可能更少），于是需要新规则来规定如何匹配。
- 它引入“贪婪匹配”与“懒惰匹配”！
- 也就是说，`*`、`+`、`?` 会先尽量多匹配，而 `*?`、`+?`、`??` 会先尽量少匹配。
- 例如 `a+.+` 匹配 `aaa`，则 `a+` 匹配全部而 `.+` 匹配空；对 `a+?.+` 匹配 `aaa`，则 `a+?` 只匹配单个 `a` 而 `.+` 匹配 `aa`。
- 若选择无法成功匹配，会回到上一个成功状态尝试另一选项（与 DFS 相同）。
- 因此贪婪或懒惰偏好可能影响性能，取决于被匹配字符串；它更偏好某些分支。

有些正则引擎还支持“占有匹配”；此处不讨论。

# Regex

### • 它也扩展了正则的功能。

- 锚点：`^` 表示匹配串必须在开头；`$` 表示必须直到末尾；`\b` 表示在词边界结束（即匹配串后的下一个字符不应是 `\w`），`\B` 相反。
- 例如 `a*` 可在 `baaa` 中匹配 `aaa`，但 `^a*` 不能；`a*` 可在 `aaab` 中匹配 `aaa`，但 `a*$` 不能。
- 捕获：`()` 额外用于捕获匹配片段，之后可取得这些捕获。
- 例如 `(a+)b` 匹配 `aaab`，则 `aaa` 被捕获；你可以在程序里使用（例如 `matched_result.get(1)`）。
- `id` 由左括号出现顺序决定。
- 整个匹配串通常也视为捕获 0。
- 若不想捕获（可提升性能），用 `(?:)` 代替。

# Regex

- 反向引用（Re2 不支持）：在正则里用捕获组 `\id`；例如 `(a+)b\1` 在 `aabaaa` 中匹配 `aabaa`，因为 `aa` 被捕获且 `\1` 指同一内容。
- 先行断言与后行断言（先行断言和后行断言，统称零宽断言）。
- Re2 也不支持。
- 正先行：`(?=xx)`，表示后续字符必须匹配 `xx` 才算满足（但 `xx` 不是匹配串的一部分）。
- 例如 `"everybody(?= lives)"` 匹配 `"everybody dies but not everybody lives"` 只会匹配第二个 `everybody`，因为其后是 `lives`（包含空格！）。
- 匹配串不是 `everybody lives`，且 `lives` 也不会被捕获。
- 正后行：`(?<=xx)`，表示前面字符必须匹配 `xx` 才算满足。
- 例如 `"(?<=not )to be"` 匹配 `"to be or not to be"` 只会匹配第二个 `to be`，因为它前面是 `not`（包含空格！）。

# Regex

- 负先行：`(?!=xx)`，表示后续字符不应匹配 `xx` 才算满足。
- 负后行：`(?!<xx)`，表示前面字符不应匹配 `xx` 才算满足。
- 先行与后行也有助于剪枝 DFS 树。

### • 最后，正则通常可指定一些模式。

- 你可以通过语言 API 指定它们。
- 最常见的是 global、忽略大小写与 multiline。
- global 表示找到第一次匹配后继续匹配；这如此常见以至于通常作为 API 提供而不是模式。
- 忽略大小写事实上也可手动用交替写法模拟。
- multiline 表示把字符串按 `'\n'` 拆成多行；这影响 `^` 与 `$`，使其有多个可能匹配位置。

### • 现在看看回溯如何工作…

# Regex

### • 例如，`"Computer graphics is awesome!"` 用

### `".*some"` 去匹配，过程类似：

- `.*` 会匹配全部（成功！），然后发现 `s` 什么也匹配不到。
- 然后 `.*` 回退（即匹配除最后的 `!` 外全部），但 `s` 仍匹配不到 `!`。
- 然后 `.*` 再回退（即匹配除 `e!` 外全部），等等。
- 最后 `.*` 只匹配 `"Computer graphics is awe"`，`s` 终于成功匹配，后续匹配继续。
- 匹配结果是 `"Computer graphics is awesome"`。

### • 你可能理解为什么回溯可能导致最坏

### 性能——它可能遍历 DFS 树上的所有路径。

- 当坏情况出现，我们称之为“正则灾难性回溯”；这被用于 ReDOS（正则拒绝服务攻击），曾搞垮 StackOverflow 与 Cloudflare。

# Regex

### • 例如：`^(?:a+)+$` 匹配 `aaaaaab`。

- 这里显然可用 `^a+$`，但真实应用里可能有更复杂正则，简化形式不易找到。
- 过程类似：
- `a+` 先匹配全部 `aaaaaa`，但 `$` 发现末尾不匹配。
- `a+` 回退到匹配 `aaaaa`，`+` 再用另一个 `a+` 吃掉最后一个 `a`；但 `$` 仍发现末尾不匹配。
- `a+` 再回退到匹配 `aaaa`，`+` 再用另一个 `a+` 吃掉 `aa`；但 `$` 仍发现末尾不匹配。
- 这次失败使第二个 `a+` 只匹配单个 `a`，`+` 再用另一个 `a+` 吃掉最后一个 `a`…
- 整体上我们遍历 DFS 树的每个节点。
- 或理论上像 𝑇_{𝑛+1} = 𝑇_𝑛 + 𝑇_{𝑛−1} + ⋯ + 𝑇_1 + 𝑘（𝑛 表示后缀长度），导致 𝑇_𝑛 = 𝑘 ⋅ 2^𝑛。
- 看不懂也没关系，去 regex101 上试试。

# Regex

### • 我个人建议你阅读/观看以下

### 文档/视频，进一步了解正则如何工作以及

### 为何会发生灾难性回溯。

- https://www.bilibili.com/video/BV17N411b72t （此视频为转载但变成了标题党，作者的建议不是“避免使用正则表达式”，而是使用前进行谨慎考虑和实验）
- C# regex doc: Details of regular expression, Backtracking in regular expression, Best practice of regex.
- 若你想看 C# 的特性如何进一步减少回溯，见 Regular expression options – nonbacktracking mode。
- https://www.regular-expressions.info/catastrophic.html（不必看 atomic group 与 possessive matching）。

# String and stream

### • 正则表达式

- 正则语法
- C++ 对正则的支持

# Overview

### • 许多编程语言（如 C#、Python）或

### 库里的正则可以被编译，即尝试优化正则

### 以获得更高性能。

- 但编译需要时间，因此不应把编译用于只用一次的正则。

### • 正则有三类操作：

- Match：匹配整个字符串。
- Search：寻找匹配的子串。
- 匹配到的部分会跳过，例如 `aaaa` 只会匹配两次 `aa`；这类似 `std::search` 用 `end` 迭代器而不是 `begin` 迭代器继续搜索。
- Replace：把匹配串替换为另一串。
- 不同语言可能有不同甚至冲突的 API 名称。

# Overview

### • 不幸的是，C++ 标准库里的正则性能悲剧

### （甚至比 Python 慢得多），且不支持 Unicode 正则。

- 一个小基准显示它在各语言里几乎最慢。
- 我自己测过，确——实——如此。
- 有一些更快的替代方案。
- Boost.Regex，也支持 Unicode 正则。
- CTRE（compile-time regex），在 C++ 编译期编译正则模式并优化匹配过程。
- 缺点：模式必须在 C++ 编译期确定，意味着只能是固定字符串；Unicode 只支持 UTF-8。
- Google 的 RE2，保证线性复杂度；
- NFA 引擎（并带一些额外技术支持捕获等）。
- 缺点：不支持反向引用、先行与后行。
- Intel 的 Hyperscan（仅可用于 x86/x86-64 Intel 平台）。

# Overview

### • 据 CTRE 作者称：

注意：CTRE 的正则字符串不应过长，否则编译器可能抱怨过于复杂

# C++ Regex

并且编译时间会极其漫长。

### • 因此我们只讲 CTRE（`"ctre.hpp"`）与 RE2（`"re2/re2.h"`）。

- CTRE 不直接支持替换操作，因为在 C++ 里替换很慢（若你想要请自己做）。
- RE2 相对 CTRE 的好处是它不支持复杂回溯，从而保证线性时间，因此不会引发 ReDOS；正则模式也可在运行时确定。
- 另外，CTRE 至少需要 C++17，最好用 C++20。
- 文末会有一些关于标准库正则的个人笔记，以防你想学。

- CTRE 不支持的一些特性：八进制形式字符、可选转义字符（例如破折号在括号里要加 `\`，即 `[a\-]`，以与字符类区分）。

# C++ Regex

### • 首先，CTRE 用结构体返回匹配结果，而 RE2 通过

### 传指针。

- `regex_results` 是匹配结果；可用 `.get<ID>()` 取得类型为 `captured_content` 的捕获。
- `.get<0>()` 是整体匹配结果。
- RE2 默认不捕获整体匹配，因此需要额外加一对括号。
- 你可以用 `.view()`/`.to_view()`/`.str()`/`.to_string()` 把 `captured_content` 转为 string view/string。
- 特别地，`.get<0>().str()` 等价于 `.to_string()`。
- 你也可以用结构化绑定取得捕获，例如 `auto [whole, a, b] = m`。

# C++ Regex

### • 对 RE2：

- 你可以编译以做准备，例如 `re2::RE2 regex{"[a-z]"}`；
- 另外，匹配数据 `str` 的类型是 `StringPiece`，可看作 string view；CTRE 则接受任意 range 或迭代器对。

### • 匹配：`ctre::match<regex>(str)` /

### `bool re2::RE2::FullMatch(str, regex, manyCapturesAddress…)`。

- 注意 `manyCapturesAddress` 可接受任意多个参数。
- 例如：`if(auto m = ctre::match<R"(\w+)">("abc")) std::cout << m.to_view();`
- `re2::RE2 r{R"((\w+))"}; std::string s; if(re2::RE2::FullMatch("abc", r, &s)) std::cout << s << '\n';` 对未使用的捕获，传 `nullptr`。

CTRE 还有 `ctre::starts_with`，要求从开头匹配（类似加锚点 `^`）。

# C++ Regex

### • 搜索：`ctre::search<regex>(str)` / `bool`

### `re2::RE2::PartialMatch(str, regex, manyCapturesAddress…)`。

### • 示例：

必须！

自 v3.9（2024/5/17）起，`ctre::range` 重命名为

# C++ Regex

`ctre::search_all`。

### • 若你想取得所有匹配串，需要

### `ctre::range<regex>(str)` /

还有 `Consume()`，

### `bool re2::RE2::FindAndConsume(str*, regex,` 要求从开头匹配

### `manyCapturesAddress…)`。

- `FindAndConsume` 与 `PartialMatch` 的区别是 `FindAndConsume` 会把传入的字符串设为新的状态。

# C++ Regex

### • RE2 另外支持替换：`bool`

### `re2::RE2::Replace(std::string*, regex, replaceStr);`

- 注意第一个参数是 `std::string` 而不是 `StringPiece`。
- 结果会存入 `std::string*`。

### • 若要替换所有串，需要 `int`

### `re2::RE2::GlobalReplace(std::string*, regex, replaceStr);`

- 它会返回匹配子段的数量。

# C++ Regex

### • CTRE 另外支持按正则切分：`ctre::split<regex>(str)`。

- 返回结果仍是 `regex_results`，第一个捕获是切分段，其它捕获是正则里的捕获组。
- 例如这里第一个 `str.get<0>()` 是 `please visit `，`str.get<1>()` 是 `127.0.0.1`，`str.get<2>()` 是 `8999`。

### • 以及分词：`ctre::tokenize<regex>(str)`。

- 即反复匹配直到正则失败。

BTW，CTRE 的“函数”其实都是函数对象。

# Summary

- 字符与字符串字面量。•格式串说明（含 range 格式）。
- 原始字符串。
- 用户自定义格式。
- `std::string` 与 `std::string_view`
- 流（Stream）
- 基于下标的方法、SSO。
- 输入/输出流
- 视图的非空结尾；一些注意点。•双向流与流绑定
- `charconv`
- 流缓冲区（stream buffer）。
- Unicode
- 流状态。
- 字节、码位、码元与文本；
- IO Manipulator* 规范化
- C++ 的 Unicode 支持。• 正则
- `/utf-8`（输入/执行字符集）• 语法与灾难性回溯。
- CTRE 与 RE2
- `std::format` 与 `std::print`

# Next lecture…

### • 我们终于进入移动语义…

- 它是现代 C++ 最基础也最重要的部分之一。
- 我们会用两个半讲来讲；时间充裕！

以上“特化”用语不准确，实际上是填充上模板参数的类型别名。

一些性能陷阱：https://stackoverflow.com/questions/76400707/why-is-stdcmatch-slower-than-stdsmatch-here

