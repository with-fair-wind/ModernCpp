// 万能引用（转发引用）与 std::forward：在模板中保留值类别，实现完美转发。
//
// 形如 `T&&` 且 T 由模板推导时，才是万能引用；显式指定 T 时则退化为右值引用。

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace {

template <typename T>
void logValueCategory(T&& value) {
    std::cout << "parameter is ";
    decltype(auto) forwarded = std::forward<T>(value);
    if constexpr (std::is_lvalue_reference_v<decltype(forwarded)>) {
        std::cout << "lvalue ref\n";
    } else {
        std::cout << "rvalue ref\n";
    }
}

template <typename T, typename U>
void relay(T&& slot, U&& payload) {
    std::forward<T>(slot)(std::forward<U>(payload));
}

struct Printer {
    void operator()(std::string const& text) const {
        std::cout << "lvalue string: " << text << '\n';
    }
    // 演示：命中右值 string 重载；此处不要求消耗参数（ostream 按 const 引用接收）。
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    void operator()(std::string&& text) const {
        std::cout << "rvalue string: " << text << '\n';
    }
};

}  // namespace

int main() {
    int n = 1;
    logValueCategory(n);
    logValueCategory(2);

    Printer p;
    std::string base{"hello"};
    relay(p, base);                // 以左值转发，期望命中 const& 重载。
    relay(p, std::string{"tmp"});  // 以右值转发，期望命中 && 重载。

    return 0;
}
