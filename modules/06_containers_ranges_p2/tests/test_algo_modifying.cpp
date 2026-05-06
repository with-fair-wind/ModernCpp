// 修改型算法：remove / unique / replace / fill / generate / rotate / reverse / copy。

#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

#include <gtest/gtest.h>

namespace stdr = std::ranges;

TEST(AlgoModify, RemoveDoesNotShrink) {
    std::vector<int> v{1, 2, 2, 3, 2};
    auto tail = stdr::remove(v, 2);
    // remove 不改变 size
    EXPECT_EQ(v.size(), 5U);
    // [begin, tail.begin()) 是要保留的部分
    EXPECT_EQ(stdr::distance(v.begin(), tail.begin()), 2);
    v.erase(tail.begin(), tail.end());
    EXPECT_EQ(v, (std::vector{1, 3}));
}

TEST(AlgoModify, EraseIfRemovesAndShrinks) {
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    auto removed = std::erase_if(v, [](int x) { return x % 2 == 0; });
    EXPECT_EQ(removed, 3U);
    EXPECT_EQ(v, (std::vector{1, 3, 5}));
}

TEST(AlgoModify, UniqueOnlyAdjacent) {
    std::vector<int> v{1, 1, 2, 1, 3, 3, 4};
    auto tail = stdr::unique(v);
    v.erase(tail.begin(), tail.end());
    // 注意 1 出现两次：分别在不同段，unique 不会跨段去重
    EXPECT_EQ(v, (std::vector{1, 2, 1, 3, 4}));
}

TEST(AlgoModify, Replace) {
    std::vector<int> v{1, 2, 1, 2, 1};
    stdr::replace(v, 1, 99);
    EXPECT_EQ(v, (std::vector{99, 2, 99, 2, 99}));

    stdr::replace_if(v, [](int x) { return x > 50; }, -1);
    EXPECT_EQ(v, (std::vector{-1, 2, -1, 2, -1}));
}

TEST(AlgoModify, Fill) {
    std::vector<int> v(5);
    stdr::fill(v, 7);
    EXPECT_EQ(v, (std::vector{7, 7, 7, 7, 7}));
}

TEST(AlgoModify, Generate) {
    std::vector<int> v(5);
    int n = 0;
    stdr::generate(v, [&n] { return ++n; });
    EXPECT_EQ(v, (std::vector{1, 2, 3, 4, 5}));
}

TEST(AlgoModify, Rotate) {
    std::vector<int> v{1, 2, 3, 4, 5};
    stdr::rotate(v, v.begin() + 2);
    EXPECT_EQ(v, (std::vector{3, 4, 5, 1, 2}));
}

TEST(AlgoModify, Reverse) {
    std::vector<int> v{1, 2, 3, 4, 5};
    stdr::reverse(v);
    EXPECT_EQ(v, (std::vector{5, 4, 3, 2, 1}));
}

TEST(AlgoModify, ShiftLeftLosesElements) {
    std::vector<int> v{1, 2, 3, 4, 5};
    auto valid = stdr::shift_left(v, 2);
    // valid 是子 range：原 [begin, end-2)，元素已被左移到 [begin, begin+3)
    ASSERT_EQ(stdr::distance(valid), 3);
    std::vector<int> got{valid.begin(), valid.end()};
    EXPECT_EQ(got, (std::vector{3, 4, 5}));
}

TEST(AlgoModify, CopyAndCopyIf) {
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    std::vector<int> dst(v.size());
    stdr::copy(v, dst.begin());
    EXPECT_EQ(dst, v);

    std::vector<int> evens;
    stdr::copy_if(v, std::back_inserter(evens), [](int x) { return x % 2 == 0; });
    EXPECT_EQ(evens, (std::vector{2, 4, 6}));
}

TEST(AlgoModify, IterSwap) {
    std::vector<int> v{10, 20, 30, 40};
    stdr::iter_swap(v.begin(), v.begin() + 3);
    EXPECT_EQ(v, (std::vector{40, 20, 30, 10}));
}
