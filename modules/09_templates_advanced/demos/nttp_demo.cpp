// 非类型模板形参（NTTP）：除类型外的模板实参——整数枚举、constexpr 常量、
// C++20 起还可使用浮点数、可作为字面量的类类型以及无捕获 lambda 闭包类型。

#include <array>
#include <iostream>
#include <type_traits>

#if __cplusplus >= 202002L
struct Widget {
    int value{};
    friend constexpr auto operator==(Widget, Widget) -> bool = default;
};

template <Widget const& W>
struct WidgetVault {
    static constexpr int peek() noexcept {
        return W.value;
    }
};

inline constexpr Widget kWidgetSeven{.value = 7};
inline constexpr Widget kWidgetNine{.value = 9};
#endif

namespace {

template <int Value>
[[nodiscard]] constexpr int scale(int x) noexcept {
    return Value * x;
}

#if __cplusplus >= 202002L
// 浮点 NTTP 仅在 C++20 及以后成立。
template <auto Threshold>
    requires std::floating_point<decltype(Threshold)>
struct FloatGate {
    static constexpr decltype(Threshold) kLimit = Threshold;
};

// 无捕获 lambda 变量可作为模板实参：`decltype(kScaler)` 成为闭合类型常量。
inline constexpr auto kScaler = [](int x) noexcept { return x * 100; };

template <decltype(kScaler) Functor>
struct ScaleHouse {
    static constexpr int run(int seed) noexcept {
        return Functor(seed);
    }
};
#endif

}  // namespace

int main() {
    constexpr int kScaled = scale<11>(5);
    std::cout << "scale<11>(5) = " << kScaled << '\n';

#if __cplusplus >= 202002L
    ScaleHouse<kScaler> house{};
    std::cout << "lambda NTTP: " << decltype(house)::run(2) << '\n';

    WidgetVault<kWidgetSeven> vault_seven{};
    WidgetVault<kWidgetNine> vault_nine{};
    std::cout << "Widget NTTP seven: " << decltype(vault_seven)::peek() << '\n';
    std::cout << "Widget NTTP nine  : " << decltype(vault_nine)::peek() << '\n';

    using PiGate = FloatGate<3.14>;
    std::cout << "float NTTP limit : " << PiGate::kLimit << '\n';
#else
    std::cout << "当前模式未启用 C++20——跳过浮点/类类型/lambda NTTP 演示。\n";
#endif

    // 传统「数组大小」类 NTTP：std::array 常用整型常量作长度。
    std::array<int, 4> buffer{};
    std::cout << "std::array N    : " << buffer.size() << '\n';

    return 0;
}
