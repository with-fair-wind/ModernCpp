// C++23 显式对象形参（deducing this）：用 this auto&& self 把 *this 的类型也参与模板推导。
//
// 无此特性时用 #if 给出说明，避免在非 C++23 环境硬编不过。

#include <iostream>
#include <type_traits>
#include <utility>

#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L

namespace {

class SideNote {
public:
    // `this auto&&` 等价于显式模板化 `this Self&&`，更紧凑。
    void bump(this auto&& self) {
        (void)self;
    }
};

class Tracked {
public:
    // 统一成员函数模板：左值实例推导为 Tracked&，右值实例推导为 Tracked&&。
    template <typename Self>
    void describe(this Self&& self) {
        static_cast<void>(std::forward<Self>(self));

        using Clean = std::remove_reference_t<Self>;
        if constexpr (std::is_const_v<Clean>) {
            std::cout << "Self 含 const —— ";
        } else {
            std::cout << "Self 非 const —— ";
        }

        if constexpr (std::is_lvalue_reference_v<Self>) {
            std::cout << "从左值调用\n";
        } else {
            std::cout << "从右值 / 将亡值调用\n";
        }

        // 仍可访问成员，self 即 *this 的别名。
    }
};

}  // namespace

int main() {
    SideNote{}.bump();

    Tracked t{};
    Tracked const ct{};

    std::cout << "左值非 const：\n";
    t.describe();

    std::cout << "const 左值：\n";
    ct.describe();

    std::cout << "临时右值：\n";
    Tracked{}.describe();

    std::cout << "static_cast<Tracked&&> 非 const（将亡值演示）：\n";
    static_cast<Tracked&&>(t).describe();

    return 0;
}

#else

int main() {
    std::cout
        << "当前翻译单元未检测到 __cpp_explicit_this_parameter。\n"
        << "请使用 C++23 且实现显式对象形参（例如 GCC 13+ / Clang 18+ / MSVC 支持度视版本）。\n";
    return 0;
}

#endif
