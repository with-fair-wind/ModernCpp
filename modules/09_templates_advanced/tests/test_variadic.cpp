// 可变参数模板：`sizeof...`、二元右折叠加法。

#include <cstddef>

#include <gtest/gtest.h>

namespace {

template <typename... Ts>
constexpr std::size_t kTypePackSize = sizeof...(Ts);

template <typename... Ts>
constexpr std::size_t valuePackArity(Ts... /*values*/) noexcept {
    return sizeof...(Ts);
}

template <typename Head, typename... Tail>
constexpr auto sumFold(Head head, Tail... tail) noexcept {
    return head + (... + tail);
}

}  // namespace

TEST(Variadic, TypeAndValuePackSizing) {
    EXPECT_EQ((kTypePackSize<int, double, char>), 3U);
    EXPECT_EQ(valuePackArity(1U, 2U, 3U, 4U), 4U);
}

TEST(Variadic, FoldSumMatchesExpectation) {
    EXPECT_EQ(sumFold(1, 2, 3, 4), 10);
}
