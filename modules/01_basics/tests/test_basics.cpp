// 覆盖 01_basics 模块各 demo 的可观测行为：安全整数比较
// （std::cmp_xxx / std::in_range）以及 <bit> 辅助函数。

#include <bit>
#include <cstdint>
#include <limits>
#include <utility>

#include <gtest/gtest.h>

TEST(IntCmp, NegativeIsLessThanUnsigned) {
    EXPECT_TRUE(std::cmp_less(-1, 1U));
    EXPECT_FALSE(std::cmp_greater_equal(-1, 0U));

    // std::cmp_* 存在的意义：朴素 `>` 会把 -1 静默提升为无符号，
    // 绕回到 UINT_MAX，结果反转为 "true" —— 这正是 std::cmp_less
    // 帮你回避的陷阱。
    // NOLINTNEXTLINE(clang-diagnostic-sign-conversion) - 故意为之，演示陷阱
    EXPECT_TRUE(-1 > 0U);
    EXPECT_EQ(static_cast<unsigned>(-1), std::numeric_limits<unsigned>::max());
}

TEST(IntCmp, InRangeChecksRepresentability) {
    EXPECT_TRUE(std::in_range<std::int8_t>(100));
    EXPECT_FALSE(std::in_range<std::int8_t>(200));
    EXPECT_FALSE(std::in_range<std::uint8_t>(-1));
}

TEST(BitOps, PowerOfTwoHelpers) {
    EXPECT_TRUE(std::has_single_bit(8U));
    EXPECT_FALSE(std::has_single_bit(6U));
    EXPECT_EQ(std::bit_ceil(5U), 8U);
    EXPECT_EQ(std::bit_floor(5U), 4U);
    EXPECT_EQ(std::bit_width(5U), 3);
}

TEST(BitOps, PopcountAndLeadingZeros) {
    EXPECT_EQ(std::popcount(0b1011U), 3);

    constexpr std::uint8_t kV = 0b0000'1100U;
    EXPECT_EQ(std::countl_zero(kV), 4);
    EXPECT_EQ(std::countr_zero(kV), 2);
}

TEST(BitCast, FloatSignBit) {
    EXPECT_EQ(std::bit_cast<std::uint32_t>(0.0F), 0U);
    EXPECT_EQ(std::bit_cast<std::uint32_t>(-0.0F), 0x8000'0000U);
}
