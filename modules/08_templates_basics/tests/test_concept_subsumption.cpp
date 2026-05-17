// concept 子蕴含：受限更严的 overload 胜出。

#if __cpp_concepts >= 202002L

#include <concepts>

#include <gtest/gtest.h>

namespace {

template <typename T>
    requires std::integral<T>
int tagOverload(T /* value */) noexcept {
    return 1;
}

template <typename T>
    requires std::signed_integral<T>
int tagOverload(T /* value */) noexcept {
    return 2;
}

}  // namespace

TEST(ConceptSubsumption, PreferMoreConstrainedOverload) {
    EXPECT_EQ(tagOverload(42U), 1);  // unsigned-only 可走 integral。
    EXPECT_EQ(tagOverload(-5), 2);   // signed 同时可行，应选择 signed_integral。
}

#else

#include <gtest/gtest.h>

TEST(ConceptSubsumptionFallback, RequiresCpp20ConceptMacros) {
    GTEST_SKIP() << "跳过：未检测到 __cpp_concepts >= 202002L（工具链不支持 concepts）";
}

#endif
