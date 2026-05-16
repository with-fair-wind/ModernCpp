// 验证 constexpr 函数在编译期与运行期语境下的一致结果。

#include <gtest/gtest.h>

namespace {

constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int fibonacci(int n) {
    return n <= 1 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

}  // namespace

TEST(ConstexprFunc, CompileTimeValues) {
    static_assert(factorial(0) == 1);
    static_assert(factorial(5) == 120);
    static_assert(fibonacci(10) == 55);

    EXPECT_EQ(factorial(5), 120);
    EXPECT_EQ(fibonacci(10), 55);
}

TEST(ConstexprFunc, RuntimeMatchesConstexprSemantics) {
    int n = 6;
    EXPECT_EQ(factorial(n), 720);

    int k = 8;
    EXPECT_EQ(fibonacci(k), 21);
}
