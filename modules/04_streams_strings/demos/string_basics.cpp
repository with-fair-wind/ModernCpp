// std::string 常用 API：构造/赋值、查找与切片，以及 SSO 与小字符串现象的直观观察。
//
// 关键点：
//   SSO（小字符串优化）常把较短字符序列放在 string 对象的内部缓冲区，
//   此时 data() 与对象地址接近；长于阈值的缓冲往往会走堆分配，
//   data() 将指向别处（实现相关，本节只做演示）。
//   C++23 起可用 resize_and_overwrite 在安全前提下复用缓冲区填充内容。

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>

namespace {

void printSep() {
    std::cout << "---\n";
}

[[nodiscard]] bool byteInObjectStorage(std::string const& s) {
    // 粗略判断：data() 是否落在对象本体的字节区间 [obj, obj+sizeof(s)) 内。
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto* const obj = reinterpret_cast<unsigned char const*>(&s);
    const auto* const end = obj + sizeof(s);  // NOLINT(bugprone-sizeof-container)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto* const ptr = reinterpret_cast<unsigned char const*>(s.data());
    return ptr >= obj && ptr < end;
}

}  // namespace

int main() {
    // 构造、赋值、移动。
    std::string a{"modern"};
    std::string b(4, '!');
    std::string c{a};             // 拷贝构造
    std::string d{std::move(c)};  // 移动构造，c 被置为空（合法但未指定状态）
    std::cout << "a=" << a << " b=" << b << " d=" << d << '\n';
    printSep();

    a.assign("streams");
    b = a;
    std::cout << "assign / copy-assign: " << b << '\n';

    std::string e;
    e += b;
    e += ' ';
    e.append("rocks");
    std::cout << "append 拼接: " << e << '\n';
    printSep();

    // 查找与子串。
    std::string hay{"abracadabra"};
    std::cout << "find('bra'): " << hay.find("bra") << '\n';
    std::cout << "rfind('abra'): " << hay.rfind("abra") << '\n';
    std::cout << "find_first_of(\"dcb\"): " << hay.find_first_of("dcb") << '\n';
    std::cout << "substr(4, 3): " << hay.substr(4, 3) << '\n';
    printSep();

#ifdef __cpp_lib_constexpr_string
    std::cout << std::boolalpha;
    std::cout << "starts_with(\"abra\"): " << hay.starts_with("abra") << '\n';
    std::cout << "ends_with(\"abra\"): " << hay.ends_with("abra") << '\n';
    std::cout << "contains(\"cad\"): " << hay.contains("cad") << '\n';
    std::cout << std::noboolalpha;
#else
    std::cout << "starts_with/ends_with/contains 需要 C++20 及配套标准库\n";
#endif
    printSep();

    // SSO 演示：短串 data 往往落在对象内部；长串通常走堆。
    std::string short_s{"hi"};
    std::string long_s(64, 'x');
    std::cout << "short_s.length=" << short_s.length() << " data-inside-object? "
              << byteInObjectStorage(short_s) << '\n';
    std::cout << "long_s.length=" << long_s.length() << " data-inside-object? "
              << byteInObjectStorage(long_s) << '\n';
    std::cout << "short_s.capacity=" << short_s.capacity()
              << " long_s.capacity=" << long_s.capacity() << '\n';
    printSep();

#if (__cplusplus >= 202302L) && defined(__cpp_lib_string_resize_and_overwrite)
    // C++23：resize_and_overwrite — 由回调实际写入，避免先默认构造再覆盖。
    std::string buf;
    buf.resize_and_overwrite(8, [](char* p, std::size_t n) {
        static char const pat[] = "ABCDEFGH";
        std::memcpy(p, pat, std::min(n, sizeof(pat) - 1U));
        return std::min(n, sizeof(pat) - 1U);
    });
    std::cout << "resize_and_overwrite: " << buf << '\n';
#else
    std::cout << "当前翻译单元/标准库未启用 resize_and_overwrite 演示（需 C++23）\n";
#endif

    return 0;
}
