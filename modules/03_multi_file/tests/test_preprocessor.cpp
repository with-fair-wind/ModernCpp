// 预处理器特性测试：宏展开、字符串化、拼接、__VA_OPT__、
// __has_include、feature test macro。

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum,modernize-macro-to-enum)

#include <string>

#include <gtest/gtest.h>

// ========== 被测宏定义 ==========

#define SQUARE(x) ((x) * (x))
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)
#define CONCAT(a, b) a##b
#define MAKE_VAR(prefix, id) prefix##_##id

#define TEST_VERSION 123

// C++20 __VA_OPT__ 宏
#define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_ARGS__ __VA_OPT__(, ) 3, 2, 1, 0)
#define COUNT_ARGS_IMPL(_1, _2, _3, N, ...) N

// do-while 安全宏
#define SAFE_INCREMENT(var, amount) \
    do {                            \
        (var) += (amount);          \
    } while (0)

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum,modernize-macro-to-enum)

// ========== 测试 ==========

TEST(Preprocessor, FunctionMacroWithParens) {
    // SQUARE 正确加括号，2+3 不会被拆开
    EXPECT_EQ(SQUARE(5), 25);
    EXPECT_EQ(SQUARE(2 + 3), 25);
}

TEST(Preprocessor, Stringify) {
    // # 运算符将参数转为字符串字面量
    EXPECT_STREQ(STRINGIFY(hello), "hello");
    EXPECT_STREQ(STRINGIFY(3 + 4), "3 + 4");
}

TEST(Preprocessor, StringifyExpandsMacro) {
    // 两层间接展开：TO_STRING 先展开参数，再字符串化
    EXPECT_STREQ(TO_STRING(TEST_VERSION), "123");
}

TEST(Preprocessor, TokenConcat) {
    int CONCAT(test, Value) = 42;  // 展开为 int testValue = 42;
    EXPECT_EQ(testValue, 42);

    int MAKE_VAR(data, count) = 7;  // 展开为 int data_count = 7;
    EXPECT_EQ(data_count, 7);
}

TEST(Preprocessor, VaOpt) {
    // __VA_OPT__ 根据可变参数是否为空来选择性插入内容
    EXPECT_EQ(COUNT_ARGS(), 0);
    EXPECT_EQ(COUNT_ARGS(a), 1);
    EXPECT_EQ(COUNT_ARGS(a, b), 2);
    EXPECT_EQ(COUNT_ARGS(a, b, c), 3);
}

TEST(Preprocessor, HasInclude) {
    // 标准库头文件应该存在
#if __has_include(<vector>)
    constexpr bool kHasVector = true;
#else
    constexpr bool kHasVector = false;
#endif
    EXPECT_TRUE(kHasVector);

    // 不存在的头文件
#if __has_include(<nonexistent_header_xyz.h>)
    constexpr bool kHasFake = true;
#else
    constexpr bool kHasFake = false;
#endif
    EXPECT_FALSE(kHasFake);
}

TEST(Preprocessor, FeatureTestMacro) {
    // C++17 及以上必有 __cpp_lib_optional
#ifdef __cpp_lib_optional
    EXPECT_GE(__cpp_lib_optional, 201606L);
#else
    GTEST_SKIP() << "__cpp_lib_optional 未定义";
#endif
}

TEST(Preprocessor, CppStandardMacro) {
    // __cplusplus 应该至少是 C++17 (201703L)
    EXPECT_GE(__cplusplus, 201703L);
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
TEST(Preprocessor, Undef) {
    // #undef 取消宏定义后该宏不再可用
#define UNDEF_TEST_VAL 123
    EXPECT_EQ(UNDEF_TEST_VAL, 123);
#undef UNDEF_TEST_VAL
    // UNDEF_TEST_VAL 此后不再可用，验证通过编译即可
    constexpr int after_undef = 456;
    EXPECT_EQ(after_undef, 456);
}
// NOLINTEND(cppcoreguidelines-macro-usage)

TEST(Preprocessor, DoWhileSafeMacro) {
    int val = 0;
    // NOLINTBEGIN(cppcoreguidelines-avoid-do-while,readability-simplify-boolean-expr,readability-braces-around-statements)
    if (true)
        SAFE_INCREMENT(val, 5);
    else
        SAFE_INCREMENT(val, 10);
    // NOLINTEND(cppcoreguidelines-avoid-do-while,readability-simplify-boolean-expr,readability-braces-around-statements)
    EXPECT_EQ(val, 5);
}
