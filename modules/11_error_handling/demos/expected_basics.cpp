// 模块 11 的小演示：把 std::expected 当作"按值"返回的错误通道。
//
// std::expected<T, E> 表示"要么是 T，要么是 E"；与异常不同，失败路径
// 写在函数签名里，且成功路径上没有任何额外开销。

#include <expected>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

namespace {

std::expected<int, std::string> parse_positive(std::string_view s) {
    if (s.empty()) {
        return std::unexpected("empty input");
    }
    constexpr int kMax = std::numeric_limits<int>::max();
    int value = 0;
    for (char const c : s) {
        if (c < '0' || c > '9') {
            return std::unexpected(std::string{"non-digit: "} + c);
        }
        int const digit = c - '0';
        // 在乘加运算溢出为 UB 之前先拒绝。
        if (value > (kMax - digit) / 10) {
            return std::unexpected("overflow");
        }
        value = (value * 10) + digit;
    }
    if (value == 0) {
        return std::unexpected("zero is not positive");
    }
    return value;
}

}  // namespace

int main() {
    for (std::string_view const s : {"42", "", "12a", "0", "7", "9999999999"}) {
        auto const r = parse_positive(s);
        if (r) {
            std::cout << "ok    \"" << s << "\" -> " << *r << '\n';
        } else {
            std::cout << "error \"" << s << "\" -> " << r.error() << '\n';
        }
    }
    return 0;
}
