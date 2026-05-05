// std::span：连续视图的不变量。

#include <array>
#include <cstddef>
#include <numeric>
#include <span>
#include <vector>

#include <gtest/gtest.h>

TEST(Span, ConstructFromVector) {
    std::vector<int> v{1, 2, 3, 4, 5};
    std::span<int> s{v};
    EXPECT_EQ(s.size(), 5U);
    EXPECT_EQ(s.front(), 1);
    EXPECT_EQ(s.back(), 5);
    EXPECT_EQ(s.data(), v.data());
}

TEST(Span, FirstLastSubspan) {
    std::array<int, 6> a{10, 20, 30, 40, 50, 60};
    std::span<int> s{a};

    auto head2 = s.first(2);
    EXPECT_EQ(head2.size(), 2U);
    EXPECT_EQ(head2.front(), 10);
    EXPECT_EQ(head2.back(), 20);

    auto tail3 = s.last(3);
    EXPECT_EQ(tail3.size(), 3U);
    EXPECT_EQ(tail3.front(), 40);

    auto mid = s.subspan(1, 3);
    EXPECT_EQ(mid.size(), 3U);
    EXPECT_EQ(std::accumulate(mid.begin(), mid.end(), 0), 20 + 30 + 40);
}

TEST(Span, IsNonOwningView) {
    std::vector<int> v{1, 2, 3};
    std::span<int> s{v};
    s[0] = 999;  // 通过 span 写入 → 写穿到原容器
    EXPECT_EQ(v[0], 999);
}

TEST(Span, ConstSpanForReadOnly) {
    std::vector<int> v{1, 2, 3};
    std::span<const int> ro{v};
    EXPECT_EQ(ro.size(), 3U);
    // ro[0] = 0;          // 不会编译：const T*
    EXPECT_EQ(ro.front(), 1);
}

TEST(Span, StaticVsDynamicExtent) {
    std::array<int, 4> a{1, 2, 3, 4};

    // 由 std::array 隐式得到固定 extent 的 span
    std::span<int, 4> sa{a};
    static_assert(decltype(sa)::extent == 4);

    // 由 vector 得到的 span 是动态 extent
    std::vector<int> v{1, 2};
    std::span<int> sd{v};
    static_assert(decltype(sd)::extent == std::dynamic_extent);
    EXPECT_EQ(sd.size(), 2U);
}

TEST(Span, AsBytesSizeMatches) {
    std::array<std::int32_t, 3> a{1, 2, 3};
    auto bytes = std::as_bytes(std::span{a});
    EXPECT_EQ(bytes.size_bytes(), sizeof(std::int32_t) * 3);
}
