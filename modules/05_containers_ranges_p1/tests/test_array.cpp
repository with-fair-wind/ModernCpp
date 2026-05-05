// std::array：栈上固定大小数组的安全包装。

#include <array>
#include <stdexcept>

#include <gtest/gtest.h>

TEST(StdArray, SizeAndAccess) {
    std::array<int, 4> a{10, 20, 30, 40};
    EXPECT_EQ(a.size(), 4U);
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(a.front(), 10);
    EXPECT_EQ(a.back(), 40);
    EXPECT_EQ(a[2], 30);
}

TEST(StdArray, AtChecksBounds) {
    std::array<int, 3> a{1, 2, 3};
    EXPECT_EQ(a.at(2), 3);
    EXPECT_THROW(static_cast<void>(a.at(99)), std::out_of_range);
}

TEST(StdArray, FillAndSwap) {
    std::array<int, 3> a{};
    a.fill(7);
    EXPECT_EQ(a, (std::array{7, 7, 7}));

    std::array<int, 3> b{1, 2, 3};
    a.swap(b);
    EXPECT_EQ(a, (std::array{1, 2, 3}));
    EXPECT_EQ(b, (std::array{7, 7, 7}));
}

TEST(StdArray, ComparisonIsLexicographic) {
    std::array<int, 3> a{1, 2, 3};
    std::array<int, 3> b{1, 2, 4};
    EXPECT_LT(a, b);
    EXPECT_NE(a, b);
    EXPECT_EQ(a, (std::array{1, 2, 3}));
}

TEST(StdArray, ToArrayDeducesElementType) {
    auto a = std::to_array({1, 2, 3, 4});  // C++20
    static_assert(std::is_same_v<decltype(a), std::array<int, 4>>);
    EXPECT_EQ(a.size(), 4U);
    EXPECT_EQ(a[0], 1);
}

TEST(StdArray, StructuredBinding) {
    std::array<int, 3> a{11, 22, 33};
    auto& [x, y, z] = a;
    x = 100;
    EXPECT_EQ(a[0], 100);
    EXPECT_EQ(y, 22);
    EXPECT_EQ(z, 33);
}
