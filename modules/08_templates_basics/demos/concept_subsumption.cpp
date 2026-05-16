// concept 子蕴含（subsumption）：更收紧的约束在重载胜出时优于更宽松的约束。
//
// signed_integral 比 integral “更受限”，两者同时可行时优先前者。

#include <concepts>
#include <iostream>

#if __cpp_concepts >= 202002L

namespace {

template <std::integral T>
int pick(T /* value */) {
    return 1;
}

template <std::signed_integral T>
int pick(T /* value */) {
    return 2;
}

}  // namespace

int main() {
    std::cout << "unsigned -> expect integral branch, got " << pick(42U) << '\n';
    std::cout << "signed -> expect signed_integral branch, got " << pick(-8) << '\n';

    return 0;
}

#else

int main() {
    std::cout << "编译器未开启 concepts（需要 __cpp_concepts >= 202002L）。\n";
    return 0;
}

#endif
