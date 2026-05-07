// stdr::to / range factory / subrange / fold_left / contains。

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace stdv = std::views;
namespace stdr = std::ranges;

TEST(RangesTo, MaterializesToVector) {
    auto v = stdv::iota(1, 6) | stdr::to<std::vector<int>>();
    EXPECT_EQ(v, (std::vector{1, 2, 3, 4, 5}));
}

TEST(RangesTo, MaterializesToMap) {
    auto m = stdv::iota(0, 3) | stdv::transform([](int i) {
                 return std::pair{i, std::string(static_cast<std::size_t>(i + 1), 'a')};
             }) |
             stdr::to<std::map<int, std::string>>();
    ASSERT_EQ(m.size(), 3U);
    EXPECT_EQ(m.at(0), "a");
    EXPECT_EQ(m.at(2), "aaa");
}

TEST(RangesFactory, SingleEmptyRepeat) {
    auto s = stdv::single(42) | stdr::to<std::vector<int>>();
    EXPECT_EQ(s, (std::vector{42}));

    auto e = stdv::empty<int>;
    EXPECT_EQ(stdr::distance(e), 0);

    auto rep = stdv::repeat('x', 4) | stdr::to<std::string>();
    EXPECT_EQ(rep, "xxxx");
}

TEST(RangesSubrange, SizeFromIterators) {
    std::vector<int> v{10, 20, 30, 40, 50};
    auto sub = stdr::subrange(v.begin() + 1, v.begin() + 4);
    EXPECT_EQ(sub.size(), 3U);
    EXPECT_EQ(*sub.begin(), 20);
    EXPECT_EQ(*(sub.end() - 1), 40);
}

TEST(RangesFold, FoldLeftSumsRange) {
    auto sum = stdr::fold_left(stdv::iota(1, 11), 0, std::plus<>{});
    EXPECT_EQ(sum, 55);
}

TEST(RangesFold, FoldLeftFirstReturnsOptional) {
    auto product = stdr::fold_left_first(std::vector{1, 2, 3, 4}, std::multiplies<>{});
    ASSERT_TRUE(product.has_value());
    EXPECT_EQ(*product, 24);

    auto empty_fold = stdr::fold_left_first(std::vector<int>{}, std::plus<>{});
    EXPECT_FALSE(empty_fold.has_value());
}

TEST(RangesAlgo, ContainsAndStartsEndsWith) {
    std::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_TRUE(stdr::contains(v, 3));
    EXPECT_FALSE(stdr::contains(v, 99));

    EXPECT_TRUE(stdr::starts_with(v, std::vector{1, 2}));
    EXPECT_FALSE(stdr::starts_with(v, std::vector{2, 3}));
    EXPECT_TRUE(stdr::ends_with(v, std::vector{4, 5}));
    EXPECT_FALSE(stdr::ends_with(v, std::vector{4, 6}));
}

TEST(RangesAlgo, ContainsSubrange) {
    std::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_TRUE(stdr::contains_subrange(v, std::vector{2, 3, 4}));
    EXPECT_FALSE(stdr::contains_subrange(v, std::vector{2, 4}));  // 不连续
}
