// 把 IEEE-754 的怪异行为转化为可断言的现象。
//
// NaN 与任何值的所有有序比较（含 ==）都为假，而 != 为真 —— 这正是
// 经典的"判断 NaN"惯用法（`x != x`）。带符号零比较相等，但位模式
// 不同。超过约 2^24 后，float 的 ULP 已超过 1，加 1 直接被吞没。

#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

TEST(FloatingPoint, NaNFailsAllOrderingExceptInequality) {
    constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
    EXPECT_FALSE(kNaN == kNaN);
    EXPECT_FALSE(kNaN < 1.0);
    EXPECT_FALSE(kNaN > 1.0);
    EXPECT_TRUE(kNaN != kNaN);
    EXPECT_TRUE(std::isnan(kNaN));
}

TEST(FloatingPoint, SignedZerosCompareEqualButDifferInBits) {
    EXPECT_EQ(0.0F, -0.0F);
    EXPECT_EQ(std::bit_cast<std::uint32_t>(0.0F), 0U);
    EXPECT_EQ(std::bit_cast<std::uint32_t>(-0.0F), 0x8000'0000U);
}

TEST(FloatingPoint, AddingOneVanishesAtLargeMagnitude) {
    // 在 1e10 附近 float 的 ULP 大约是 2^11 = 2048，加 1 直接被舍掉。
    constexpr float kBig = 1.0e10F;
    EXPECT_EQ(kBig + 1.0F, kBig);
    // 但 1e6 处 ULP < 1，加法仍是可观测的。
    constexpr float kSmall = 1.0e6F;
    EXPECT_NE(kSmall + 1.0F, kSmall);
}
