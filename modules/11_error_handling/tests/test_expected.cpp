// Smoke test for module 11: std::expected happy path / error path.

#include <expected>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace {

std::expected<int, std::string> parse_positive(std::string_view s) {
    if (s.empty()) {
        return std::unexpected("empty");
    }
    int value = 0;
    for (char const c : s) {
        if (c < '0' || c > '9') {
            return std::unexpected("non-digit");
        }
        value = (value * 10) + (c - '0');
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
