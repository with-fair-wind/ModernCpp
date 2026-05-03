// 有作用域枚举（C++11）、显式底层类型、std::to_underlying（C++23）、
// using enum（C++20），以及标志位风格的运算符重载。
//
// 有作用域枚举解决了 C 风格枚举的两大痛点：名字污染、向 int 的隐式转换。
// 下面的位标志模式说明了它的"代价"：你必须自己重新提供位运算。

#include <cstdint>
#include <iostream>
#include <type_traits>
#include <utility>

namespace {

enum class Day : std::uint8_t { Mon = 1, Tue, Wed, Thu, Fri, Sat, Sun };

// 位标志枚举：所有 1~7 的组合都列在 enumerator 中，避免 clang-analyzer
// 抱怨 `Read | Write` 的结果（=3）不在枚举的合法值集合内。底层类型选
// std::uint8_t —— 当前只用 3 个位，1 字节足够。
enum class Flag : std::uint8_t {
    None = 0,
    Read = 1U << 0U,
    Write = 1U << 1U,
    ReadWrite = Read | Write,
    Exec = 1U << 2U,
    ReadExec = Read | Exec,
    WriteExec = Write | Exec,
    All = Read | Write | Exec,
};

constexpr Flag operator|(Flag a, Flag b) noexcept {
    using T = std::underlying_type_t<Flag>;
    return Flag{static_cast<T>(a) | static_cast<T>(b)};
}

constexpr bool has(Flag set, Flag bit) noexcept {
    using T = std::underlying_type_t<Flag>;
    return (static_cast<T>(set) & static_cast<T>(bit)) != 0;
}

}  // namespace

int main() {
    constexpr Day kToday = Day::Wed;
    static_assert(std::is_same_v<std::underlying_type_t<Day>, std::uint8_t>);

    // std::to_underlying 等价于 static_cast<U>(value)，但更能表达意图，
    // 即便后续修改了底层类型也仍然正确。
    std::cout << "today underlying = " << +std::to_underlying(kToday) << '\n';

    switch (kToday) {
        using enum Day;
        case Mon:
        case Tue:
        case Wed:
        case Thu:
        case Fri:
            std::cout << "weekday\n";
            break;
        case Sat:
        case Sun:
            std::cout << "weekend\n";
            break;
    }

    constexpr Flag kPerms = Flag::Read | Flag::Write;
    std::cout << std::boolalpha << "has Read  = " << has(kPerms, Flag::Read) << '\n'
              << "has Write = " << has(kPerms, Flag::Write) << '\n'
              << "has Exec  = " << has(kPerms, Flag::Exec) << '\n';

    return 0;
}
