// extern "C" 测试：验证 C ABI 函数能正确链接和调用。

#include <string>

#include <gtest/gtest.h>

#include "c_api.h"

TEST(ExternC, ArithmeticAdd) {
    EXPECT_EQ(c_api_add(3, 4), 7);
    EXPECT_EQ(c_api_add(-1, 1), 0);
    EXPECT_EQ(c_api_add(0, 0), 0);
}

TEST(ExternC, ArithmeticSubtract) {
    EXPECT_EQ(c_api_subtract(10, 3), 7);
    EXPECT_EQ(c_api_subtract(3, 10), -7);
}

TEST(ExternC, ArithmeticMultiply) {
    EXPECT_EQ(c_api_multiply(6, 7), 42);
    EXPECT_EQ(c_api_multiply(-2, 3), -6);
    EXPECT_EQ(c_api_multiply(0, 100), 0);
}

TEST(ExternC, FormatInt) {
    // c_api_format_int 内部用 C++ std::to_string 实现
    EXPECT_STREQ(c_api_format_int(42), "int(42)");
    EXPECT_STREQ(c_api_format_int(-7), "int(-7)");
    EXPECT_STREQ(c_api_format_int(0), "int(0)");
}

TEST(ExternC, Accumulator) {
    c_api_accumulator_reset();
    EXPECT_EQ(c_api_accumulator_get(), 0);

    c_api_accumulator_add(10);
    c_api_accumulator_add(20);
    c_api_accumulator_add(30);
    EXPECT_EQ(c_api_accumulator_get(), 60);

    c_api_accumulator_reset();
    EXPECT_EQ(c_api_accumulator_get(), 0);
}

TEST(ExternC, AccumulatorNegativeValues) {
    c_api_accumulator_reset();
    c_api_accumulator_add(100);
    c_api_accumulator_add(-30);
    c_api_accumulator_add(-20);
    EXPECT_EQ(c_api_accumulator_get(), 50);
}
