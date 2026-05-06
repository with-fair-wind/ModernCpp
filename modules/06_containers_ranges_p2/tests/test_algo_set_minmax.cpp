// 集合操作 / minmax / clamp / 排列。

#include <algorithm>
#include <iterator>
#include <vector>

#include <gtest/gtest.h>

namespace stdr = std::ranges;

TEST(AlgoSet, Includes) {
    std::vector<int> a{1, 2, 4, 5, 8};
    EXPECT_TRUE(stdr::includes(a, std::vector{2, 5, 8}));
    EXPECT_FALSE(stdr::includes(a, std::vector{1, 2, 3}));
}

TEST(AlgoSet, Intersection) {
    std::vector<int> a{1, 2, 4, 5, 8};
    std::vector<int> b{2, 3, 4, 6, 8};
    std::vector<int> dst;
    stdr::set_intersection(a, b, std::back_inserter(dst));
    EXPECT_EQ(dst, (std::vector{2, 4, 8}));
}

TEST(AlgoSet, Union) {
    std::vector<int> a{1, 2, 4};
    std::vector<int> b{2, 3, 4, 5};
    std::vector<int> dst;
    stdr::set_union(a, b, std::back_inserter(dst));
    EXPECT_EQ(dst, (std::vector{1, 2, 3, 4, 5}));
}

TEST(AlgoSet, Difference) {
    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<int> b{2, 4};
    std::vector<int> dst;
    stdr::set_difference(a, b, std::back_inserter(dst));
    EXPECT_EQ(dst, (std::vector{1, 3, 5}));
}

TEST(AlgoSet, SymmetricDifference) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{2, 3, 4};
    std::vector<int> dst;
    stdr::set_symmetric_difference(a, b, std::back_inserter(dst));
    EXPECT_EQ(dst, (std::vector{1, 4}));
}

TEST(AlgoMinMax, MinMaxOnRange) {
    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};
    auto mm = stdr::minmax(v);
    EXPECT_EQ(mm.min, 1);
    EXPECT_EQ(mm.max, 9);
}

TEST(AlgoMinMax, MinMaxElement) {
    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};
    auto mm = stdr::minmax_element(v);
    EXPECT_EQ(*mm.min, 1);
    EXPECT_EQ(*mm.max, 9);
}

TEST(AlgoClamp, ClampsValueToRange) {
    EXPECT_EQ(std::clamp(5, 0, 10), 5);
    EXPECT_EQ(std::clamp(-3, 0, 10), 0);
    EXPECT_EQ(std::clamp(99, 0, 10), 10);
}

TEST(AlgoCount, AllAnyNone) {
    std::vector<int> v{2, 4, 6, 8};
    EXPECT_TRUE(stdr::all_of(v, [](int x) { return x % 2 == 0; }));
    EXPECT_TRUE(stdr::any_of(v, [](int x) { return x > 5; }));
    EXPECT_TRUE(stdr::none_of(v, [](int x) { return x < 0; }));

    EXPECT_EQ(stdr::count(v, 4), 1);
    EXPECT_EQ(stdr::count_if(v, [](int x) { return x > 4; }), 2);
}

TEST(AlgoPermutation, IsPermutation) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{2, 1, 3};
    std::vector<int> c{1, 2, 4};
    EXPECT_TRUE(stdr::is_permutation(a, b));
    EXPECT_FALSE(stdr::is_permutation(a, c));
}

TEST(AlgoPermutation, NextPrevPermutation) {
    std::vector<int> v{1, 2, 3};
    auto next1 = stdr::next_permutation(v);
    EXPECT_TRUE(next1.found);
    EXPECT_EQ(v, (std::vector{1, 3, 2}));

    auto next2 = stdr::next_permutation(v);
    EXPECT_TRUE(next2.found);
    EXPECT_EQ(v, (std::vector{2, 1, 3}));

    // 反向回到上一个
    auto prev = stdr::prev_permutation(v);
    EXPECT_TRUE(prev.found);
    EXPECT_EQ(v, (std::vector{1, 3, 2}));
}

TEST(AlgoPermutation, NextPermutationCountsAll) {
    std::vector<int> v{1, 2, 3};
    int count = 1;  // 包含初始排列
    while (stdr::next_permutation(v).found) {
        ++count;
    }
    EXPECT_EQ(count, 6);  // 3! = 6
}
