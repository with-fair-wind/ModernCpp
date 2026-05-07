// std::generator (C++23) 行为：input_range / 与 views 组合 / 一次性消费。

#include <generator>
#include <ranges>
#include <vector>

#include <gtest/gtest.h>

namespace stdv = std::views;
namespace stdr = std::ranges;

namespace {

std::generator<int> countUp(int from, int n) {
    for (int i = 0; i < n; ++i) {
        co_yield from + i;
    }
}

std::generator<int> emptyGen() {
    co_return;
}

std::generator<int> infiniteFrom(int start) {
    while (true) {
        co_yield start++;
    }
}

std::generator<int> interleave(int n) {
    for (int i = 0; i < n; ++i) {
        co_yield i;
        co_yield -i;
    }
}

}  // namespace

TEST(StdGenerator, BasicYield) {
    auto v = countUp(10, 5) | stdr::to<std::vector<int>>();
    EXPECT_EQ(v, (std::vector{10, 11, 12, 13, 14}));
}

TEST(StdGenerator, EmptyGenerator) {
    auto v = emptyGen() | stdr::to<std::vector<int>>();
    EXPECT_TRUE(v.empty());
}

TEST(StdGenerator, ComposesWithTake) {
    auto v = infiniteFrom(0) | stdv::take(5) | stdr::to<std::vector<int>>();
    EXPECT_EQ(v, (std::vector{0, 1, 2, 3, 4}));
}

TEST(StdGenerator, ComposesWithFilter) {
    auto v = infiniteFrom(1) | stdv::filter([](int x) { return x % 3 == 0; }) | stdv::take(4) |
             stdr::to<std::vector<int>>();
    EXPECT_EQ(v, (std::vector{3, 6, 9, 12}));
}

TEST(StdGenerator, ComposesWithTransform) {
    auto v =
        countUp(1, 5) | stdv::transform([](int x) { return x * x; }) | stdr::to<std::vector<int>>();
    EXPECT_EQ(v, (std::vector{1, 4, 9, 16, 25}));
}

TEST(StdGenerator, MultipleYieldsPerIteration) {
    auto v = interleave(3) | stdr::to<std::vector<int>>();
    EXPECT_EQ(v, (std::vector{0, 0, 1, -1, 2, -2}));
}

TEST(StdGenerator, IsInputRange) {
    static_assert(stdr::input_range<std::generator<int>>);
    static_assert(stdr::view<std::generator<int>>);
    // generator 不是 forward_range——只能遍历一次
    static_assert(!stdr::forward_range<std::generator<int>>);
}

TEST(StdGenerator, ManualBeginIncrementDeref) {
    auto g = countUp(100, 3);
    auto it = g.begin();
    ASSERT_NE(it, g.end());
    EXPECT_EQ(*it, 100);
    ++it;
    EXPECT_EQ(*it, 101);
    ++it;
    EXPECT_EQ(*it, 102);
    ++it;
    EXPECT_EQ(it, g.end());
}
