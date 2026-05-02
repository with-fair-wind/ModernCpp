// Smoke test for module 11: std::expected happy path / error path.

#include <expected>
#include <limits>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace {

std::expected<int, std::string> parse_positive(std::string_view s) {
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
    auto const r = parse_positive("42");
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, 42);
}

TEST(Expected, ErrCarriesReason) {
    auto const r = parse_positive("12a");
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), "non-digit");
}

TEST(Expected, ValueOrFallsBackOnError) {
    EXPECT_EQ(parse_positive("").value_or(-1), -1);
    EXPECT_EQ(parse_positive("9").value_or(-1), 9);
}

TEST(Expected, OverflowReturnsError) {
    // 19 digits — way past INT_MAX whatever its size; overflow must be caught
    // before the multiply/add runs into UB (otherwise UBSan fails the test).
    auto const r = parse_positive("9999999999999999999");
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), "overflow");
}

TEST(Expected, AcceptsIntMaxBoundary) {
    // INT_MAX itself must still parse — the guard rejects only true overflow.
    auto const r = parse_positive(std::to_string(std::numeric_limits<int>::max()));
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, std::numeric_limits<int>::max());
}
