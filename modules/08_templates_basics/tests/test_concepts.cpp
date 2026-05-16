// concepts：验证约束是否按预期允许 / 拒绝给定类型。

#if __cpp_concepts >= 202002L

#include <concepts>
#include <cstddef>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

namespace {

template <typename T>
concept AddableRequiresSame = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
};

template <typename T>
concept NonPointerScalar = requires {
    requires !std::is_pointer_v<std::decay_t<T>>;
    requires sizeof(std::decay_t<T>) <= sizeof(void*);
};

template <AddableRequiresSame T>
constexpr T doubleByAdd(const T& base) noexcept {
    return base + base;
}

// `+` 的返回类型与操作数不一致，用来否定 `same_as<T>` 约束。
struct MismatchedSummand {};

constexpr int operator+(MismatchedSummand /* lhs */, MismatchedSummand /* rhs */) noexcept {
    return 0;
}

}  // namespace

TEST(Concepts, AllowsAddableIntsAndStrings) {
    static_assert(AddableRequiresSame<int>);
    static_assert(AddableRequiresSame<std::string>);

    EXPECT_EQ(doubleByAdd(21), 42);
    EXPECT_EQ(doubleByAdd(std::string{"cpp"}), "cppcpp");

    constexpr bool kIntsOkay = NonPointerScalar<int>;
    constexpr bool kStructsOkay = NonPointerScalar<std::byte>;

    EXPECT_TRUE(kIntsOkay);
    EXPECT_TRUE(kStructsOkay);
    EXPECT_FALSE((NonPointerScalar<int*>));
}

TEST(Concepts, RejectsMismatchedAddResult) {
    // 强制 ODR 使用，避免 “unneeded internal declaration” 与概念检查无关的告警。
    volatile int sink = MismatchedSummand{} + MismatchedSummand{};
    (void)sink;
    ASSERT_FALSE((AddableRequiresSame<MismatchedSummand>));
}

#else

#include <gtest/gtest.h>

TEST(ConceptsFallback, RequiresCpp20ConceptMacros) {
    GTEST_SKIP() << "跳过：未检测到 __cpp_concepts >= 202002L（工具链不支持 concepts）";
}

#endif
