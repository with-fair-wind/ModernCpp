// 折叠表达式：一元左/右折叠与带初值的二元折叠应与手算结果一致。

#include <gtest/gtest.h>

namespace {

template <typename... Args>
[[nodiscard]] constexpr auto sumRightFold(Args... args) noexcept {
    return (... + args);
}

template <typename... Args>
[[nodiscard]] constexpr auto sumLeftFold(Args... args) noexcept {
    return (args + ...);
}

template <typename... Args>
[[nodiscard]] constexpr auto sumWithTail(Args... args) noexcept {
    return (args + ... + -1);
}

template <typename... Args>
[[nodiscard]] constexpr auto sumBinaryLeft(Args... args) noexcept {
    return (0 + ... + args);
}

template <typename... Args>
[[nodiscard]] constexpr auto productLeft(Args... args) noexcept {
    return (args * ... * 1);  // explicit identity
}

}  // namespace

TEST(FoldExpressions, UnaryRightAndLeftAgree) {
    constexpr int kRight = sumRightFold(1, 2, 3);
    constexpr int kLeft = sumLeftFold(1, 2, 3);

    EXPECT_EQ(kRight, kLeft);
    EXPECT_EQ(kRight, 6);
}

TEST(FoldExpressions, TailInitChangesNeutralElement) {
    EXPECT_EQ(sumWithTail(3, 7), 9);  // 3 + 7 + (-1)

    constexpr int kProd = productLeft(2, 3, 5);
    EXPECT_EQ(kProd, 30);
}

TEST(FoldExpressions, BinaryLeftFoldAddsIdentityPrefix) {
    constexpr int kSum = sumBinaryLeft(1, 2, 3);
    EXPECT_EQ(kSum, 6);
}
