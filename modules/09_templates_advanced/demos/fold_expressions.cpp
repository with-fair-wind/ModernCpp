// C++17 折叠表达式：压缩对形参包的二元运算，左/右折叠与带 init 形式。

#include <iostream>
#include <string>

namespace {

template <typename... Args>
[[nodiscard]] auto sumRight(Args... args) {
    return (... + args);  // 一元右折叠：(a1 + (a2 + (...)))
}

template <typename... Args>
[[nodiscard]] auto sumLeft(Args... args) {
    return (args + ...);  // 一元左折叠：(((a1 + a2) + ...) + aN)
}

template <typename... Args>
[[nodiscard]] auto sumWithInit(Args... args) {
    return (args + ... + 0);  // 二元右折叠：(a1 + (a2 + (... + init)))
}

template <typename... Args>
[[nodiscard]] auto sumBinaryLeft(Args... args) {
    return (0 + ... + args);  // 二元左折叠：((((init + a1) + a2) + ...) + aN)
}

template <typename... Args>
[[nodiscard]] auto commaFoldToString(Args const&... fragments) {
    return (std::string{} + ... + std::string{fragments});  // 二元左折叠拼接
}

}  // namespace

int main() {
    std::cout << "一元右折叠 (...+args): " << sumRight(1, 2, 3, 4) << '\n';
    std::cout << "一元左折叠 (args+...): " << sumLeft(1, 2, 3, 4) << '\n';
    std::cout << "二元右折叠 (args+...+init): " << sumWithInit(10, 11) << '\n';
    std::cout << "二元左折叠 (init+...+args): " << sumBinaryLeft(1, 2, 3, 4) << '\n';
    std::cout << "字符串二元左折叠        : " << commaFoldToString("fold", "-", "rocks") << '\n';

    return 0;
}
