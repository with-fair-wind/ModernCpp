// 惰性实例化：仅在 ODR 使用成员时才实例化；NoAdd + MaybeMath 为测试端独立复现。

#include <gtest/gtest.h>

namespace {

struct NoAdd {
    int tag{};
};

template <typename T>
struct MaybeMath {
    [[nodiscard]] static constexpr int sizeOf() noexcept {
        return static_cast<int>(sizeof(T));
    }

    template <typename U = T>
    [[nodiscard]] static auto add(U const& lhs, U const& rhs) -> decltype(lhs + rhs) {
        return lhs + rhs;
    }
};

}  // namespace

TEST(LazyInstantiation, SizeOfForIntAndAddSemantics) {
    static_assert(MaybeMath<int>::sizeOf() == static_cast<int>(sizeof(int)));
    constexpr int kSz = MaybeMath<int>::sizeOf();
    EXPECT_EQ(kSz, static_cast<int>(sizeof(int)));

    EXPECT_EQ(MaybeMath<int>::add(2, 3), 5);
}

TEST(LazyInstantiation, NoAddSizeOfWithoutInstantiatingAdd) {
    constexpr int kPlain = MaybeMath<NoAdd>::sizeOf();
    EXPECT_EQ(kPlain, static_cast<int>(sizeof(NoAdd)));

    constexpr int kCheck = MaybeMath<NoAdd>::sizeOf();
    EXPECT_EQ(kCheck, kPlain);
}
