// 模块 11：std::optional 构造、查询与 monadic 操作的测试。

#include <optional>
#include <string>

#include <gtest/gtest.h>

namespace {

std::optional<int> parseDigit(char c) {
    if (c < '0' || c > '9') {
        return std::nullopt;
    }
    return c - '0';
}

}  // namespace

TEST(Optional, DefaultIsEmpty) {
    std::optional<int> const empty{};
    EXPECT_FALSE(empty.has_value());
}

TEST(Optional, ValueCtorAndDeref) {
    std::optional<int> const full{42};
    ASSERT_TRUE(full.has_value());
    EXPECT_EQ(*full, 42);
}

TEST(Optional, ValueOrFallsBack) {
    std::optional<int> const empty{};
    EXPECT_EQ(empty.value_or(-1), -1);
    std::optional<int> const ok{9};
    EXPECT_EQ(ok.value_or(-1), 9);
}

TEST(Optional, ParseDigitHelper) {
    EXPECT_FALSE(parseDigit('m').has_value());
    auto const digit8 = parseDigit('8');
    ASSERT_TRUE(digit8.has_value());
    EXPECT_EQ(*digit8, 8);  // NOLINT(bugprone-unchecked-optional-access)
}

#if defined(__cpp_lib_optional) && __cpp_lib_optional >= 202110L

TEST(Optional, AndThenChainsOnValue) {
    auto const r = parseDigit('3').and_then([](int d) { return std::optional<int>{d + 1}; });
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, 4);  // NOLINT(bugprone-unchecked-optional-access)
}

TEST(Optional, TransformMapsValue) {
    auto const r = parseDigit('7').transform([](int d) { return d * 10; });
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, 70);  // NOLINT(bugprone-unchecked-optional-access)
}

TEST(Optional, OrElseRunsWhenEmpty) {
    auto const r = parseDigit('z').or_else([] { return std::optional<int>{99}; });
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, 99);  // NOLINT(bugprone-unchecked-optional-access)
}

#endif
