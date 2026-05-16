// 模块 11 的小演示：std::optional 与 C++23 起提供的 monadic 风格 API。

#include <iostream>
#include <optional>
#include <string>

namespace {

std::optional<int> parseDigit(char c) {
    if (c < '0' || c > '9') {
        return std::nullopt;
    }
    return c - '0';
}

void printMonadicSamples() {
#if defined(__cpp_lib_optional) && __cpp_lib_optional >= 202110L
    std::cout << "--- C++23 monadic 操作（and_then / transform / or_else）---\n";

    auto const chain = parseDigit('5')
                           .and_then([](int d) -> std::optional<int> {
                               return d == 0 ? std::nullopt : std::optional<int>{d * 10};
                           })
                           .transform([](int v) { return std::string{"x"} + std::to_string(v); })
                           .or_else([] { return std::optional<std::string>{"fallback"}; });
    std::cout << "and_then+transform 链: " << chain.value_or("(无值)") << '\n';

    auto const bad = parseDigit('x').or_else([] { return std::optional<int>{-1}; });
    // parseDigit('x') 无值后由 or_else 填入 -1，链在此处必有值；用 if 明示前置条件以通过静态检查。
    if (bad.has_value()) {
        std::cout << "or_else 在失败路径: " << *bad << '\n';
    }
#else
    std::cout << "当前工具链未报告 __cpp_lib_optional >= 202110L，跳过 monadic 样例。\n";
#endif
}

}  // namespace

int main() {
    std::cout << "--- 基本构造与 has_value / value_or ---\n";
    std::optional<int> empty{};
    std::optional<int> full = 7;
    std::cout << "empty: " << (empty.has_value() ? "有值" : "无值") << ", value_or -> "
              << empty.value_or(-1) << '\n';
    std::cout << "full: " << *full << ", value_or -> " << full.value_or(-1) << '\n';

    printMonadicSamples();
    return 0;
}
