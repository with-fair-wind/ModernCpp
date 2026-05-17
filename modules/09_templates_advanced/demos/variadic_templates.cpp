// 可变参数模板：用「形参包」表达 0..N 个类型或函数实参，配合递归或列表展开处理。

#include <array>
#include <iostream>
#include <ostream>
#include <type_traits>

namespace {

void printArgs(std::ostream& /*unused*/) {}

template <typename Head, typename... Tail>
void printArgs(std::ostream& out, Head const& head, Tail const&... tail) {
    out << head;
    if constexpr (sizeof...(tail) > 0) {
        out << ' ';
    }
    printArgs(out, tail...);
}

template <typename... Args>
[[nodiscard]] constexpr std::size_t countArgs() {
    return sizeof...(Args);
}

template <typename... Args>
[[nodiscard]] constexpr std::size_t countValues(Args&&... /*args*/) noexcept {
    return sizeof...(Args);
}

template <typename... Args>
[[nodiscard]] constexpr auto expandToArray(Args... values) {
    using Common = std::common_type_t<Args...>;
    return std::array<Common, sizeof...(Args)>{static_cast<Common>(values)...};
}

}  // namespace

int main() {
    constexpr std::size_t kTypeArity = countArgs<int, double, char>();
    constexpr std::size_t kValueArity = countValues(1, 2, 3, 4);
    std::cout << "类型包数量: " << kTypeArity << "\n值包数量: " << kValueArity << '\n';

    std::cout << "递归展开: ";
    printArgs(std::cout, "hello", 2026, 3.14);
    std::cout << '\n';

    constexpr auto kPacked = expandToArray(1, 2U, 3L);
    std::cout << "初始化列表展开成 std::array: ";
    for (auto element : kPacked) {
        std::cout << element << ' ';
    }
    std::cout << '\n';

    return 0;
}
