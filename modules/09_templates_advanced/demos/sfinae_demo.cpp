// SFINAE：`std::enable_if` 在返回类型/默认模板实参中剔除非法重载；
// `std::void_t` 用于检测某类型表达式是否良构（detection idiom）。

#include <iostream>
#include <type_traits>
#include <vector>

namespace {

// 返回类型 SFINAE：仅当 T 为整型时才参与重载集。
template <typename T>
// NOLINTNEXTLINE(modernize-use-constraints) — 本节演示 SFINAE 与 enable_if
auto asDouble(T value) -> std::enable_if_t<std::is_integral_v<T>, double> {
    return static_cast<double>(value);
}

template <typename T>
// NOLINTNEXTLINE(modernize-use-constraints) — 本节演示 SFINAE 与 enable_if
auto asDouble(T value) -> std::enable_if_t<std::is_floating_point_v<T>, double> {
    return static_cast<double>(value);
}

template <typename, typename = void>
struct HasValueType : std::false_type {};

template <typename T>
struct HasValueType<T, std::void_t<typename T::value_type>> : std::true_type {};

template <typename Container>
void describe([[maybe_unused]] Container const& container) {
    if constexpr (HasValueType<Container>::value) {
        std::cout << "拥有 value_type 的容器\n";
    } else {
        std::cout << "非标准容器\n";
    }
}

}  // namespace

int main() {
    std::cout << "整型 asDouble: " << asDouble(3) << '\n';
    std::cout << "浮点 asDouble: " << asDouble(2.5F) << '\n';

    std::vector<int> vec;
    describe(vec);

    double raw = 4.2;
    describe(raw);

    static_assert(HasValueType<std::vector<float>>::value);
    static_assert(!HasValueType<double>::value);

    return 0;
}
