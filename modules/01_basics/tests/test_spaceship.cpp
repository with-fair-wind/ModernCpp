// `operator<=> = default` 自动合成 <、<=、>、>=。结果的"序类别"
// 退化到最弱的成员：整数保持 strong，含 double 时因 NaN 而降级为
// std::partial_ordering。

#include <compare>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

namespace {

struct Point {
    int x;
    int y;
    auto operator<=>(Point const&) const = default;
};

struct Mass {
    double kg;
    auto operator<=>(Mass const&) const = default;
};

}  // namespace

TEST(Spaceship, DefaultsAllSixComparisons) {
    constexpr Point kA{.x = 1, .y = 2};
    constexpr Point kB{.x = 1, .y = 3};
    EXPECT_TRUE(kA < kB);
    EXPECT_TRUE(kA <= kB);
    EXPECT_FALSE(kA > kB);
    EXPECT_FALSE(kA == kB);
    EXPECT_TRUE(kA != kB);
}

TEST(Spaceship, IntegralComparisonsAreStrong) {
    static_assert(std::is_same_v<std::compare_three_way_result_t<Point>, std::strong_ordering>);
    SUCCEED();
}

TEST(Spaceship, FloatingPointComparisonsArePartial) {
    static_assert(std::is_same_v<std::compare_three_way_result_t<Mass>, std::partial_ordering>);

    Mass const nan{std::numeric_limits<double>::quiet_NaN()};
    Mass const one{1.0};
    auto const cat = nan <=> one;
    EXPECT_EQ(cat, std::partial_ordering::unordered);
    EXPECT_FALSE(nan < one);
    EXPECT_FALSE(nan == one);
}
