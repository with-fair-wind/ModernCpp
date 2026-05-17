// 验证整数 / C++20 类类型 NTTP 产生互不等价的类型别名。

#include <type_traits>

#include <gtest/gtest.h>

namespace {

template <int Identity>
struct IntSlot {
    static constexpr int kSlot = Identity;
};

#if __cplusplus >= 202002L
struct Box {
    int payload{};
};

template <Box Value>
constexpr int unwrap() noexcept {
    return Value.payload;
}

inline constexpr Box kEight{.payload = 8};
inline constexpr Box kNine{.payload = 9};
#endif

}  // namespace

TEST(Nttp, IntegerValuesYieldDistinctTypes) {
    static_assert(!std::is_same_v<IntSlot<1>, IntSlot<3>>);

    constexpr int kLhs = IntSlot<1>::kSlot;
    constexpr int kRhs = IntSlot<3>::kSlot;

    EXPECT_NE(kLhs, kRhs);
}

TEST(Nttp, ConstexprFunctionsDependOnDistinctClassNttps) {
#if __cplusplus >= 202002L
    EXPECT_EQ(unwrap<kEight>(), unwrap<Box{.payload = 8}>());
    EXPECT_EQ(unwrap<kNine>(), 9);
#else
    GTEST_SKIP() << "requires C++20 class-type NTTP";
#endif
}
