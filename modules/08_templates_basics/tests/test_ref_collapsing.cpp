// 引用折叠：`&` / `&&` 两两组合后与左值引用/右值引用的等价关系。

#include <type_traits>

#include <gtest/gtest.h>

namespace {

template <typename T>
using LRef = T&;

template <typename T>
using RRef = T&&;

}  // namespace

TEST(RefCollapsing, LRefLRef) {
    static_assert(std::is_same_v<LRef<int>&, int&>);
    EXPECT_TRUE((std::is_same_v<LRef<int>&, int&>));
}

TEST(RefCollapsing, LRefRRef) {
    static_assert(std::is_same_v<LRef<int>&&, int&>);
    EXPECT_TRUE((std::is_same_v<LRef<int>&&, int&>));
}

TEST(RefCollapsing, RRefLRef) {
    static_assert(std::is_same_v<RRef<int>&, int&>);
    EXPECT_TRUE((std::is_same_v<RRef<int>&, int&>));
}

TEST(RefCollapsing, RRefRRef) {
    static_assert(std::is_same_v<RRef<int>&&, int&&>);
    EXPECT_TRUE((std::is_same_v<RRef<int>&&, int&&>));
}
