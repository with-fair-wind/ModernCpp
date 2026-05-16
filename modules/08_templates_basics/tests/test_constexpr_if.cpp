// 验证 `if constexpr` 在编译期对不同分支的决定性裁剪。

#include <array>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

namespace {

template <typename T>
constexpr std::size_t bytePolicy() {
    if constexpr (std::is_same_v<T, void>) {
        return 0;
    } else if constexpr (std::is_integral_v<T>) {
        return sizeof(T);
    } else {
        return sizeof(T) + 2;  // 测试用占位策略：非 void 亦非整型。
    }
}

}  // namespace

TEST(ConstexprIf, VoidHandledSeparately) {
    static_assert(bytePolicy<void>() == 0);
    EXPECT_EQ(bytePolicy<void>(), static_cast<std::size_t>(0));
}

TEST(ConstexprIf, IntegralUsesSizeof) {
    static_assert(bytePolicy<unsigned char>() == sizeof(unsigned char));
    static_assert(bytePolicy<long long>() == sizeof(long long));
    EXPECT_EQ(bytePolicy<int>(), sizeof(int));
}

TEST(ConstexprIf, OtherTypesFallThroughElse) {
    constexpr std::size_t kPolicy = bytePolicy<std::string>();
    static_assert(kPolicy == sizeof(std::string) + 2);

    constexpr std::array<int, bytePolicy<unsigned>()> kStorage{};
    EXPECT_EQ(kStorage.size(), sizeof(unsigned));
}

#if __cplusplus >= 202302L

namespace {

constexpr int branchMarker(int value) {
    if consteval {
        return value * 10;
    }
    return value;
}

}  // namespace

TEST(ConstexprIf, IfConstevalDistinguishesContexts) {
    static_assert(branchMarker(4) == 40);
    int runtime = 4;
    EXPECT_EQ(branchMarker(runtime), 4);
}

#endif
