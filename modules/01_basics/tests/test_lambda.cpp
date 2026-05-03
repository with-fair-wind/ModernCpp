// Lambda 捕获语义：按值捕获在构造时即冻结一份副本；按引用捕获跟踪
// 外层变量；mutable 解除 operator() 上的隐式 const；泛型 lambda 的
// operator() 自身是模板。

#include <string>

#include <gtest/gtest.h>

TEST(Lambda, CaptureByValueFreezesAtConstruction) {
    int n = 1;
    auto get = [n] { return n; };
    n = 99;
    EXPECT_EQ(get(), 1);
    // 同时观察外层 n 已变 —— 用一下 n，绕过"值赋给后未读"的告警。
    EXPECT_EQ(n, 99);
}

TEST(Lambda, CaptureByReferenceTracksOuter) {
    int n = 1;
    auto get = [&n] { return n; };
    n = 99;
    EXPECT_EQ(get(), 99);
}

TEST(Lambda, MutableLambdaModifiesItsOwnCopy) {
    int n = 0;
    auto step = [n]() mutable { return ++n; };
    EXPECT_EQ(step(), 1);
    EXPECT_EQ(step(), 2);
    EXPECT_EQ(n, 0);
}

TEST(Lambda, GenericLambdaInstantiatesPerCallType) {
    auto twice = [](auto const& v) { return v + v; };
    EXPECT_EQ(twice(21), 42);
    EXPECT_EQ(twice(std::string{"ha"}), "haha");
}
