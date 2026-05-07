// 模块 11 的烟雾测试：std::expected 的成功路径与错误路径。

#include <expected>
#include <limits>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace {

std::expected<int, std::string> parsePositive(std::string_view s) {
    if (s.empty()) {
        return std::unexpected("empty");
    }
    constexpr int kMax = std::numeric_limits<int>::max();
    int value = 0;
    for (char const c : s) {
        if (c < '0' || c > '9') {
            return std::unexpected("non-digit");
        }
        int const digit = c - '0';
        if (value > (kMax - digit) / 10) {
            return std::unexpected("overflow");
        }
        value = (value * 10) + digit;
    }
    if (value == 0) {
        return std::unexpected("zero");
    }
    return value;
}

}  // namespace

TEST(Expected, OkCarriesValue) {
    auto const r = parsePositive("42");
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, 42);
}

TEST(Expected, ErrCarriesReason) {
    auto const r = parsePositive("12a");
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), "non-digit");
}

TEST(Expected, ValueOrFallsBackOnError) {
    EXPECT_EQ(parsePositive("").value_or(-1), -1);
    EXPECT_EQ(parsePositive("9").value_or(-1), 9);
}

TEST(Expected, OverflowReturnsError) {
    // 19 位数 —— 不论 INT_MAX 多大都铁定溢出；必须在乘加进入 UB 之前
    // 拦下（否则 UBSan 会让测试挂掉）。
    auto const r = parsePositive("9999999999999999999");
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), "overflow");
}

TEST(Expected, AcceptsIntMaxBoundary) {
    // INT_MAX 本身仍应能成功解析 —— 防护逻辑只拒绝真正的溢出。
    auto const r = parsePositive(std::to_string(std::numeric_limits<int>::max()));
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, std::numeric_limits<int>::max());
}
