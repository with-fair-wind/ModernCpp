// Covers the observable behaviour of the 01_basics demos: safe integer
// comparison (std::cmp_xxx / std::in_range) and the <bit> helpers.

#include <bit>
#include <cstdint>
#include <limits>
#include <utility>

#include <gtest/gtest.h>

TEST(IntCmp, NegativeIsLessThanUnsigned) {
    EXPECT_TRUE(std::cmp_less(-1, 1U));
    EXPECT_FALSE(std::cmp_greater_equal(-1, 0U));

    // Why std::cmp_* exists: the naive `>` silently promotes -1 to unsigned,
    // wrapping it to UINT_MAX, so the answer flips to "true" — the trap that
    // std::cmp_less protects you from.
    // NOLINTNEXTLINE(clang-diagnostic-sign-conversion) - intentional, demonstrates the trap
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
