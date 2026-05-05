// std::tuple、std::pair 与结构化绑定。

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

TEST(Tuple, BasicAccess) {
    std::tuple<int, double, std::string> t{1, 2.5, "three"};
    EXPECT_EQ(std::get<0>(t), 1);
    EXPECT_DOUBLE_EQ(std::get<1>(t), 2.5);
    EXPECT_EQ(std::get<2>(t), "three");

    // 类型唯一时，可按类型取
    EXPECT_EQ(std::get<int>(t), 1);
    EXPECT_EQ(std::get<std::string>(t), "three");

    static_assert(std::tuple_size_v<decltype(t)> == 3);
    static_assert(std::is_same_v<std::tuple_element_t<1, decltype(t)>, double>);
}

TEST(Tuple, StructuredBinding) {
    auto t = std::tuple{1, 2.5, std::string{"x"}};
    auto& [a, b, c] = t;
    a = 10;
    EXPECT_EQ(std::get<0>(t), 10);
    EXPECT_DOUBLE_EQ(b, 2.5);
    EXPECT_EQ(c, "x");
}

TEST(Tuple, TieAssignment) {
    std::tuple<int, int, int> t{1, 2, 3};
    int x = 0;
    int z = 0;
    std::tie(x, std::ignore, z) = t;
    EXPECT_EQ(x, 1);
    EXPECT_EQ(z, 3);
}

TEST(Tuple, TupleCat) {
    auto a = std::tuple{1, 'a'};
    auto b = std::tuple{2.5, std::string{"x"}};
    auto c = std::tuple_cat(a, b);
    static_assert(std::tuple_size_v<decltype(c)> == 4);
    EXPECT_EQ(std::get<0>(c), 1);
    EXPECT_EQ(std::get<3>(c), "x");
}

TEST(Pair, BehavesLikeTwoTuple) {
    std::pair<int, std::string> p{42, "x"};
    auto& [n, s] = p;
    EXPECT_EQ(n, 42);
    EXPECT_EQ(s, "x");
    EXPECT_EQ(std::get<0>(p), 42);
}

TEST(Tuple, LexicographicComparison) {
    std::tuple<int, int> a{1, 2};
    std::tuple<int, int> b{1, 3};
    EXPECT_LT(a, b);
    EXPECT_NE(a, b);
    EXPECT_EQ(a, (std::tuple{1, 2}));
}

TEST(Tuple, AggregateStructuredBinding) {
    struct Point {
        int x;
        int y;
    };
    Point p{3, 4};
    auto& [x, y] = p;
    x = 30;
    EXPECT_EQ(p.x, 30);
    EXPECT_EQ(y, 4);
}
