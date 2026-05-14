// ODR 测试：验证跨 TU 的函数调用、类成员函数链接、静态成员共享。

#include <string>

#include <gtest/gtest.h>

#include "odr_helper.h"

TEST(ODR, FreeFunctionLinksAcrossTU) {
    // add() 定义在 test_odr_helper.cpp，本 TU 通过头文件声明调用
    EXPECT_EQ(add(1, 2), 3);
    EXPECT_EQ(add(-5, 5), 0);
    EXPECT_EQ(add(0, 0), 0);
}

TEST(ODR, FreeFunctionReturnsString) {
    EXPECT_EQ(greet("Alice"), "Hello, Alice!");
    EXPECT_EQ(greet(""), "Hello, !");
}

TEST(ODR, ClassMemberFunctionLinksAcrossTU) {
    Counter c{10};
    EXPECT_EQ(c.value(), 10);

    c.increment();
    c.increment();
    EXPECT_EQ(c.value(), 12);

    c.decrement();
    EXPECT_EQ(c.value(), 11);
}

TEST(ODR, DefaultArgumentInDeclaration) {
    // 默认参数写在声明（头文件）中，定义不重复
    Counter c{0};
    c.incrementBy();  // 使用默认参数 step=1
    EXPECT_EQ(c.value(), 1);
    c.incrementBy(10);  // 显式传参
    EXPECT_EQ(c.value(), 11);
}

TEST(ODR, StaticMemberSharedAcrossInstances) {
    // 重置计数（之前的测试可能已经创建过实例）
    Counter::instance_count = 0;

    Counter c1{0};
    EXPECT_EQ(Counter::instance_count, 1);

    Counter c2{5};
    EXPECT_EQ(Counter::instance_count, 2);

    // 同一静态成员 —— 两个实例共享
    Counter c3{100};
    EXPECT_EQ(Counter::instance_count, 3);
}
