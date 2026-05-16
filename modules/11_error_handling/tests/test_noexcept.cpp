// 模块 11：noexcept 运算符在编译期的结果。

#include <gtest/gtest.h>

namespace {

[[maybe_unused]] void allowedToThrow() {}

[[maybe_unused]] void neverThrows() noexcept {}

struct NoexceptStruct {
    void quiet() noexcept {}
    void loud() {}
};

}  // namespace

TEST(Noexcept, PlainFunctionVersusNoexcept) {
    // noexcept() 为不可求值语境，显式调用以保证定义被 ODR 使用
    allowedToThrow();
    neverThrows();
    EXPECT_FALSE(noexcept(allowedToThrow()));
    EXPECT_TRUE(noexcept(neverThrows()));
}

TEST(Noexcept, MemberCallReflectsSpecifier) {
    EXPECT_TRUE(noexcept(NoexceptStruct{}.quiet()));
    EXPECT_FALSE(noexcept(NoexceptStruct{}.loud()));
}

TEST(Noexcept, SimpleExpressionCanBeNoexcept) {
    int x = 0;
    EXPECT_TRUE(noexcept(++x));
    EXPECT_EQ(x, 0);  // noexcept 子表达式不在此处求值，x 保持为 0
}
