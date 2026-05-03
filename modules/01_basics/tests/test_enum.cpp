// 有作用域枚举：显式底层类型、std::to_underlying（C++23），以及一个
// 极简的标志位 operator|，演示如何重新引入位运算。

#include <cstdint>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

namespace {

enum class Day : std::uint8_t { Mon = 1, Tue, Wed };

// 把所有 1~7 的位组合都列出来，避免 clang-analyzer 抱怨 `Read | Write`
// 落在 enumerator 范围之外。底层类型 uint8_t 已足够。
enum class Flag : std::uint8_t {
    None = 0,
    Read = 1U,
    Write = 2U,
    ReadWrite = Read | Write,
    Exec = 4U,
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

TEST(Enum, UnderlyingTypeIsAsDeclared) {
    static_assert(std::is_same_v<std::underlying_type_t<Day>, std::uint8_t>);
    EXPECT_EQ(std::to_underlying(Day::Wed), 3);
}

TEST(Enum, FlagsCompose) {
    constexpr Flag kRw = Flag::Read | Flag::Write;
    EXPECT_TRUE(has(kRw, Flag::Read));
    EXPECT_TRUE(has(kRw, Flag::Write));
    EXPECT_FALSE(has(kRw, Flag::Exec));
}
