// 模板显式实例化测试：验证源文件中显式实例化的函数模板和类模板。
// 对照文档「模板显式实例化」章节。

#include <cstddef>
#include <string>

#include <gtest/gtest.h>

#include "tmpl.h"

// ========== 函数模板 tmplAdd ==========

TEST(ExplicitInstantiation, TmplAddInt) {
    EXPECT_EQ(tmplAdd(3, 4), 7);
    EXPECT_EQ(tmplAdd(0, 0), 0);
    EXPECT_EQ(tmplAdd(-5, 3), -2);
}

TEST(ExplicitInstantiation, TmplAddDouble) {
    EXPECT_DOUBLE_EQ(tmplAdd(1.5, 2.5), 4.0);
    EXPECT_DOUBLE_EQ(tmplAdd(-1.0, 1.0), 0.0);
}

TEST(ExplicitInstantiation, TmplAddString) {
    EXPECT_EQ(tmplAdd(std::string{"hello"}, std::string{" world"}),
              "hello world");
    EXPECT_EQ(tmplAdd(std::string{}, std::string{"abc"}), "abc");
}

// ========== 函数模板 tmplToString ==========

TEST(ExplicitInstantiation, TmplToStringInt) {
    EXPECT_EQ(tmplToString(42), "42");
    EXPECT_EQ(tmplToString(-7), "-7");
    EXPECT_EQ(tmplToString(0), "0");
}

TEST(ExplicitInstantiation, TmplToStringDouble) {
    // std::to_string(3.14) 的精确格式取决于实现，只验证非空
    std::string result = tmplToString(3.14);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("3.14"), std::string::npos);
}

// ========== 类模板 Stack<int> ==========

TEST(ExplicitInstantiation, StackIntPushPopSize) {
    Stack<int> s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0U);

    s.push(10);
    s.push(20);
    s.push(30);
    EXPECT_EQ(s.size(), 3U);
    EXPECT_FALSE(s.empty());

    EXPECT_EQ(s.pop(), 30);
    EXPECT_EQ(s.pop(), 20);
    EXPECT_EQ(s.pop(), 10);
    EXPECT_TRUE(s.empty());
}

TEST(ExplicitInstantiation, StackIntPopEmptyReturnsDefault) {
    // 空栈 pop 返回 T{} 即 0
    Stack<int> s;
    EXPECT_EQ(s.pop(), 0);
}

// ========== 类模板 Stack<std::string> ==========

TEST(ExplicitInstantiation, StackStringPushPop) {
    Stack<std::string> s;
    s.push("alpha");
    s.push("beta");
    EXPECT_EQ(s.size(), 2U);

    EXPECT_EQ(s.pop(), "beta");
    EXPECT_EQ(s.pop(), "alpha");
    EXPECT_TRUE(s.empty());
}

TEST(ExplicitInstantiation, StackStringPopEmptyReturnsDefault) {
    Stack<std::string> s;
    EXPECT_EQ(s.pop(), "");
}

// ========== 类模板 Stack<double> ==========

TEST(ExplicitInstantiation, StackDoublePushPop) {
    Stack<double> s;
    s.push(1.5);
    s.push(2.5);
    EXPECT_DOUBLE_EQ(s.pop(), 2.5);
    EXPECT_DOUBLE_EQ(s.pop(), 1.5);
}
