// Covers the observable behaviour of the 01_basics demos: safe integer
// comparison (std::cmp_xxx / std::in_range) and the <bit> helpers.

#include <bit>
#include <cstdint>
#include <utility>

#include <gtest/gtest.h>

TEST(IntCmp, NegativeIsLessThanUnsigned) {
    EXPECT_TRUE(std::cmp_less(-1, 1u));
    EXPECT_FALSE(std::cmp_greater_equal(-1, 0u));
    // The naive comparison would give the opposite answer:
    EXPECT_TRUE(-1 > 0u);
}

TEST(IntCmp, InRangeChecksRepresentability) {
    EXPECT_TRUE(std::in_range<std::int8_t>(100));
    EXPECT_FALSE(std::in_range<std::int8_t>(200));
    EXPECT_FALSE(std::in_range<std::uint8_t>(-1));
}

TEST(BitOps, PowerOfTwoHelpers) {
    EXPECT_TRUE(std::has_single_bit(8u));
    EXPECT_FALSE(std::has_single_bit(6u));
    EXPECT_EQ(std::bit_ceil(5u), 8u);
    EXPECT_EQ(std::bit_floor(5u), 4u);
    EXPECT_EQ(std::bit_width(5u), 3);
}

TEST(BitOps, PopcountAndLeadingZeros) {
    EXPECT_EQ(std::popcount(0b1011u), 3);

    constexpr std::uint8_t v = 0b0000'1100u;
    EXPECT_EQ(std::countl_zero(v), 4);
    EXPECT_EQ(std::countr_zero(v), 2);
}

TEST(BitCast, FloatSignBit) {
    EXPECT_EQ(std::bit_cast<std::uint32_t>(0.0f), 0u);
    EXPECT_EQ(std::bit_cast<std::uint32_t>(-0.0f), 0x8000'0000u);
}
