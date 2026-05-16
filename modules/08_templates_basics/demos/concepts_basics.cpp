// C++20 concepts：用 requires 子句表述约束，将“合法实参集合”说清楚。
//
// 下面演示自定义 concept 以及与 auto 占位符组合的简洁写法。

#include <concepts>
#include <iostream>
#include <string>
#include <utility>

#if __cpp_concepts >= 202002L

namespace {

template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
};

template <typename T>
concept Printable = requires(T value, std::ostream& os) {
    { os << value } -> std::same_as<std::ostream&>;
};

template <typename T>
void dump(T&& value)
    requires Addable<std::decay_t<T>> && Printable<std::decay_t<T>>
{
    auto sum = std::forward<T>(value) + std::forward<T>(value);
    std::cout << "doubled via concept constraints: " << sum << '\n';
}

template <Addable T>
T fusedAdd(T a, T b) {
    return a + b;
}

}  // namespace

int main() {
    dump(21);
    dump(std::string{"hi"});

    std::cout << "fusedAdd(3.5, 0.5) = " << fusedAdd(3.5, 0.5) << '\n';
    return 0;
}

#else

int main() {
    std::cout << "编译器未开启 concepts（需要 __cpp_concepts >= 202002L）。\n";
    return 0;
}

#endif
