// constexpr 变量、函数与 constexpr lambda：编译期求值与现代 C++ 常量表达式。
//
// 展示 factorial / fibonacci 等可在编译期完成的工作，以及如何与运行时混用。

#include <array>
#include <iostream>
#include <type_traits>

namespace {

constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int fibonacci(int n) {
    return n <= 1 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

constexpr int kFib10 = fibonacci(10);
constexpr int kFact6 = factorial(6);

// C++17 起 lambda 可被 constexpr（隐式或显式），此处显式标明意图。
constexpr auto makeScale(int base) {
    return [base](int x) { return x * base; };
}

constexpr int scaledValue() {
    constexpr auto kScaler = makeScale(3);
    return kScaler(7);
}

}  // namespace

int main() {
    static_assert(factorial(5) == 120);
    static_assert(kFib10 == 55);
    static_assert(kFact6 == 720);
    static_assert(scaledValue() == 21);

    // 常量表达式上下文：结果在编译期已知。
    std::cout << "constexpr fibonacci(10) = " << kFib10 << '\n';
    std::cout << "constexpr factorial(6) = " << kFact6 << '\n';
    std::cout << "constexpr lambda 3 * 7 = " << scaledValue() << '\n';

    int n = 5;
    // 运行时实参仍可调用 constexpr 函数，只是不再是编译期常量。
    std::cout << "runtime factorial(" << n << ") = " << factorial(n) << '\n';

    constexpr std::array<int, factorial(4)> kTable{};
    std::cout << "array size (= factorial(4)) = " << kTable.size() << '\n';

    return 0;
}
