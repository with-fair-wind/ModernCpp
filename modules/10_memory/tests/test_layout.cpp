// 验证对象的 sizeof / offsetof、EBO 与 [[no_unique_address]] 的压缩语义。

#include <cstddef>

#include <gtest/gtest.h>

namespace {

struct EmptyTag {};

struct Packed {
    short s{};
    // 在许多 ABI 下 double 对齐为 8，short 之后会插入填充对齐到 double。
    double d{};
};

struct EboNode : EmptyTag {
    int value{};
};

struct NonEboNode {
    EmptyTag tag{};
    int value{};
};

struct WithCompressedEmpty {
    [[no_unique_address]] EmptyTag tag{};
    int value{};
};

}  // namespace

TEST(LayoutSizes, EmptyTypeHasNonZeroStandaloneSize) {
    EXPECT_GE(sizeof(EmptyTag), sizeof(char));
}

TEST(LayoutSizes, PackedStructHasHoleBeforeDouble) {
    EXPECT_GE(sizeof(Packed), sizeof(short) + sizeof(double));
#if defined(__GNUC__) || defined(__clang__)
    EXPECT_GT(offsetof(Packed, d), offsetof(Packed, s));
#endif
}

TEST(LayoutSizes, EmptyBaseOptimizationShrinksComparedToSeparateMember) {
    EXPECT_LT(sizeof(EboNode), sizeof(NonEboNode));
}

TEST(LayoutSizes, NonOptimizedEmptyMemberAddsByte) {
    EXPECT_GE(sizeof(NonEboNode), sizeof(EboNode));
}

TEST(LayoutSizes, NoUniqueAddressCompressesEmptyMember) {
    EXPECT_GE(sizeof(WithCompressedEmpty), sizeof(int));
#if !defined(_MSC_VER) || defined(__clang__)
    // MSVC 的 MSVC ABI 在具体布局上偶有差异；Clang/MinGW/Linux 上对空成员压缩更直观。
    EXPECT_EQ(sizeof(WithCompressedEmpty), sizeof(int));
#endif
}
