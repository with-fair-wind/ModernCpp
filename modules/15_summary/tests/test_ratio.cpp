// std::ratio 编译期算术：static_assert 搭配 googletest 期望值。

#include <ratio>
#include <type_traits>

#include <gtest/gtest.h>

TEST(RatioArithmetic, AdditionMatchesExpectation) {
    using quarter = std::ratio<1, 4>;
    using eighth = std::ratio<1, 8>;
    using sum = std::ratio_add<quarter, eighth>;
    static_assert(sum::num == 3 && sum::den == 8);
    EXPECT_EQ(sum::num, 3);
    EXPECT_EQ(sum::den, 8);
}

TEST(RatioArithmetic, MultiplyAndDivideReduce) {
    using step = std::ratio_multiply<std::milli, std::kilo>;
    static_assert(std::is_same_v<step, std::ratio<1>>);

    using third = std::ratio_divide<std::ratio<1>, std::ratio<3>>;
    using scaled = std::ratio_multiply<third, std::ratio<9>>;
    EXPECT_EQ(scaled::num, 3);
    EXPECT_EQ(scaled::den, 1);
}

TEST(RatioArithmetic, SubtractProducesReducedFraction) {
    using diff = std::ratio_subtract<std::ratio<7, 12>, std::ratio<1, 4>>;
    static_assert(diff::num == 1 && diff::den == 3);
    EXPECT_EQ(diff::num, 1);
    EXPECT_EQ(diff::den, 3);
}
