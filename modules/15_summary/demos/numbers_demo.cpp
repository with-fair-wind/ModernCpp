// C++20 `<numbers>` 数学常数：`double` 别名与模板 `*_v`。

#include <cmath>
#include <iostream>
#include <numbers>
#include <type_traits>

namespace {

template <typename T>
void writeConstant(char const* label, T value) {
    std::cout << label << "=" << static_cast<double>(value) << '\n';
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    std::cout << "--- double 常量别名 ---\n";
    writeConstant("pi", std::numbers::pi);
    writeConstant("e", std::numbers::e);
    writeConstant("sqrt2", std::numbers::sqrt2);
    writeConstant("ln2", std::numbers::ln2);
    writeConstant("phi", std::numbers::phi);

    std::cout << "\n--- 模板 *_v<float> ---\n";
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(std::numbers::pi_v<float>)>, float>);
    constexpr float kPiF = std::numbers::pi_v<float>;
    std::cout << "pi<float> =" << kPiF << " (字节数 " << sizeof(kPiF) << ")\n";

    std::cout << "\n--- 与 `<cmath>` 配合 ---\n";
    auto const hypot_from_constants =
        std::hypot(std::numbers::sqrt2_v<long double>, std::numbers::sqrt2_v<long double>);
    std::cout << "hypot(sqrt2, sqrt2) 应接近 2: " << hypot_from_constants << '\n';

    std::cout << "exp(ln2) 近似 2: " << std::exp(std::numbers::ln2) << '\n';

    return 0;
}
