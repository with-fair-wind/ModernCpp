// <charconv>：无区域设置、无分配的高速数值↔文本转换。
//
// 关键点：
//   to_chars / from_chars 通过 std::to_chars_result / from_chars_result
//   返回指针区间与 std::errc；与 iostream 不同，它不做填充/本地化。

#include <array>
#include <charconv>
#include <iostream>
#include <string_view>
#include <system_error>

namespace {

void reportErrc(std::string_view label, std::errc ec) {
    std::cout << label << " -> " << std::make_error_code(ec).message() << '\n';
}

void demoIntRoundtrip() {
    int value = 255;
    std::array<char, 32> buf{};
    auto to = std::to_chars(buf.data(), buf.data() + buf.size(), value, 16);
    if (to.ec == std::errc{}) {
        std::cout << "to_chars(255, hex): ";
        std::cout.write(buf.data(), static_cast<std::streamsize>(to.ptr - buf.data()));
        std::cout << '\n';
    } else {
        reportErrc("to_chars int", to.ec);
    }

    int parsed = 0;
    std::string_view hex{"ff"};
    auto from = std::from_chars(hex.data(), hex.data() + hex.size(), parsed, 16);
    if (from.ec == std::errc{}) {
        std::cout << "from_chars(\"ff\", 16) = " << parsed << '\n';
    } else {
        reportErrc("from_chars int", from.ec);
    }
}

void demoBases() {
    int v = 42;
    std::array<char, 64> b{};
    for (int base : {2, 8, 10, 16}) {
        auto r = std::to_chars(b.data(), b.data() + b.size(), v, base);
        if (r.ec == std::errc{}) {
            std::cout << "base " << base << ": ";
            std::cout.write(b.data(), static_cast<std::streamsize>(r.ptr - b.data()));
            std::cout << '\n';
        }
    }
}

void demoFloat() {
    // Apple Clang 的 libc++ 尚未实现浮点 from_chars/to_chars
#if defined(__cpp_lib_to_chars) && (__cpp_lib_to_chars >= 201611L)
    double x = 1.25e-3;
    std::array<char, 64> buf{};
    auto r = std::to_chars(buf.data(), buf.data() + buf.size(), x, std::chars_format::general);
    if (r.ec == std::errc{}) {
        std::cout << "to_chars double: ";
        std::cout.write(buf.data(), static_cast<std::streamsize>(r.ptr - buf.data()));
        std::cout << '\n';
    } else {
        reportErrc("to_chars double", r.ec);
    }

    double y = 0.0;
    std::string_view text{"0.00125"};
    auto f = std::from_chars(text.data(), text.data() + text.size(), y, std::chars_format::general);
    if (f.ec == std::errc{}) {
        std::cout << "from_chars double: " << y << '\n';
    } else {
        reportErrc("from_chars double", f.ec);
    }
#else
    std::cout << "浮点 from_chars/to_chars 不可用（跳过）。\n";
#endif
}

void demoErrors() {
    std::array<char, 3> tiny{};
    int big = 123456;
    auto r = std::to_chars(tiny.data(), tiny.data() + tiny.size(), big, 10);
    reportErrc("buffer too small for to_chars", r.ec);

    int v = 0;
    std::string_view bad{"12x34"};
    auto f = std::from_chars(bad.data(), bad.data() + bad.size(), v, 10);
    std::cout << "partial parse value=" << v << " ptr offset=" << (f.ptr - bad.data()) << '\n';
    reportErrc("from_chars with junk tail", f.ec);
}

}  // namespace

int main() {
    demoIntRoundtrip();
    std::cout << "---\n";
    demoBases();
    std::cout << "---\n";
    demoFloat();
    std::cout << "---\n";
    demoErrors();
    return 0;
}
