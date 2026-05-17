// 用 decltype((expr)) 与 type_traits 核对表达式的值类别（lvalue / xvalue / prvalue）。

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

namespace {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage) — 需宏传入未求值的 decltype((expr))
#define MCPP_EXPECT_LVALUE(expr) std::is_lvalue_reference_v<decltype((expr))>
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MCPP_EXPECT_XVALUE(expr) \
    (!MCPP_EXPECT_LVALUE(expr) && std::is_rvalue_reference_v<decltype((expr))>)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MCPP_EXPECT_PRVALUE(expr) (!std::is_reference_v<decltype((expr))>)

}  // namespace

TEST(ValueCategories, NamedVariableIsLvalue) {
    int x = 0;
    (void)x;  // 仅在 decltype 中使用
    EXPECT_TRUE(MCPP_EXPECT_LVALUE(x));
    EXPECT_FALSE(MCPP_EXPECT_XVALUE(x));
    EXPECT_FALSE(MCPP_EXPECT_PRVALUE(x));
}

TEST(ValueCategories, MoveAppliesToLvalueProducesXvalue) {
    int x = 0;
    EXPECT_FALSE(MCPP_EXPECT_LVALUE(std::move(x)));
    EXPECT_TRUE(MCPP_EXPECT_XVALUE(std::move(x)));
    EXPECT_FALSE(MCPP_EXPECT_PRVALUE(std::move(x)));
}

TEST(ValueCategories, IntegerLiteralIsPrvalue) {
    EXPECT_FALSE(MCPP_EXPECT_LVALUE(42));
    EXPECT_FALSE(MCPP_EXPECT_XVALUE(42));
    EXPECT_TRUE(MCPP_EXPECT_PRVALUE(42));
}

TEST(ValueCategories, PreIncrementYieldsLvalue) {
    int n = 1;
    // decltype 内为未求值上下文，++n 不会实际执行。
    EXPECT_TRUE(MCPP_EXPECT_LVALUE(++n));
    EXPECT_FALSE(MCPP_EXPECT_PRVALUE(++n));
    ++n;  // 实际自增
    EXPECT_EQ(n, 2);
}

TEST(ValueCategories, StringLiteralIsLvalue) {
    // 字符串字面量是左值（类型为 char const[N]）。
    EXPECT_TRUE(MCPP_EXPECT_LVALUE("literal"));
    EXPECT_FALSE(MCPP_EXPECT_PRVALUE("literal"));
}

#undef MCPP_EXPECT_PRVALUE
#undef MCPP_EXPECT_XVALUE
#undef MCPP_EXPECT_LVALUE
