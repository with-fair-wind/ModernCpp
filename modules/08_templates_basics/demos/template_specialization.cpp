// 类模板主模板、偏特化与全特化：同一族模板在不同形参下的不同实现选择。

#include <iostream>
#include <type_traits>

namespace {

template <typename T>
struct TypeLabel {
    static constexpr char const* text() {
        return "primary";
    }
};

// 偏特化：指针类型共享另一份实现。
template <typename T>
struct TypeLabel<T*> {
    static constexpr char const* text() {
        return "pointer partial";
    }
};

// 全特化：对 bool 提供完全定制。
template <>
struct TypeLabel<bool> {
    static constexpr char const* text() {
        return "bool full";
    }
};

}  // namespace

int main() {
    std::cout << "int        -> " << TypeLabel<int>::text() << '\n';
    std::cout << "int*       -> " << TypeLabel<int*>::text() << '\n';
    std::cout << "bool       -> " << TypeLabel<bool>::text() << '\n';
    std::cout << "bool*      -> " << TypeLabel<bool*>::text() << '\n';

    static_assert(std::is_same_v<decltype(&TypeLabel<int>::text), char const* (*)()>);
    return 0;
}
