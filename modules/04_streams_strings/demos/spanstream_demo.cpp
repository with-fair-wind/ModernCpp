// C++23 <spanstream>：在固定长度的 std::span 字符缓冲上套用 iostream 语义。
//
// 关键点：
//   不产生额外堆分配视图；需注意截断与不自动 '\0' 终止（除非手写）；
//   仅在 __cpp_lib_spanstream 定义时编译演示主体。

#include <array>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

#if defined(__cpp_lib_spanstream) && (__cpp_lib_spanstream >= 202106L)
#include <spanstream>
#endif

namespace {

#if defined(__cpp_lib_spanstream) && (__cpp_lib_spanstream >= 202106L)

void demoOSSpanStream() {
    std::array<char, 64> raw{};
    raw.fill('\0');
    std::ospanstream out{std::span<char>{raw.data(), raw.size()}};
    out << "approx=";
    out.setf(std::ios::fixed);
    out.precision(4);
    out << 3.14159;
    out.flush();
    auto const len = strnlen(raw.data(), raw.size());
    std::cout << "ospanstream 写入: \"" << std::string_view(raw.data(), len) << "\"\n";
}

void demoISSpanStream() {
    char text[] = "42 answer";
    std::ispanstream in{std::span<char>{text, sizeof(text) - 1U}};
    int iv = 0;
    std::string word;
    in >> iv >> word;
    std::cout << "ispanstream 解析: iv=" << iv << " word=" << word << '\n';
}

#endif

#if !defined(__cpp_lib_spanstream) || (__cpp_lib_spanstream < 202106L)

void reportNoSpanStream() {
    std::cout << "当前环境与标准库未提供 <spanstream>。\n";
}

#endif

}  // namespace

int main() {
#if defined(__cpp_lib_spanstream) && (__cpp_lib_spanstream >= 202106L)
    demoOSSpanStream();
    std::cout << "---\n";
    demoISSpanStream();
#else
    reportNoSpanStream();
#endif
    std::cout << "\n可把 spanstream 用于嵌入式或大缓冲池中的零拷贝文本编码。\n";
    return 0;
}
