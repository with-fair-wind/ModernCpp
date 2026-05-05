// std::vector：容量管理、迭代器/引用失效边界、erase 返回值、vector<bool> 代理。

#include <iterator>
#include <vector>

#include <gtest/gtest.h>

TEST(Vector, ReserveDoesNotChangeSize) {
    std::vector<int> v;
    v.reserve(100);
    EXPECT_EQ(v.size(), 0U);
    EXPECT_GE(v.capacity(), 100U);
}

TEST(Vector, ClearKeepsCapacity) {
    std::vector<int> v(50, 0);
    auto cap_before = v.capacity();
    v.clear();
    EXPECT_EQ(v.size(), 0U);
    EXPECT_EQ(v.capacity(), cap_before);  // 容量保留
}

TEST(Vector, ShrinkToFitDropsExtra) {
    std::vector<int> v;
    v.reserve(1024);
    v.push_back(1);
    EXPECT_GE(v.capacity(), 1024U);
    v.shrink_to_fit();
    // 主流实现都会真正收缩，但标准只规定为非绑定请求 → 用 LE 校验
    EXPECT_LE(v.capacity(), 1024U);
}

TEST(Vector, IteratorsStableWhenReserveSufficient) {
    std::vector<int> v;
    v.reserve(8);
    v.push_back(1);
    int* p = &v.front();
    v.push_back(2);
    v.push_back(3);
    EXPECT_EQ(p, &v.front());  // 没有重分配，引用仍有效
}

TEST(Vector, EraseReturnsNextValid) {
    std::vector<int> v{1, 2, 3, 4, 5};
    auto it = v.erase(v.begin() + 1);  // 删 2
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(v, (std::vector{1, 3, 4, 5}));
}

TEST(Vector, InsertReturnsPositionOfFirstInserted) {
    std::vector<int> v{1, 4};
    auto it = v.insert(v.begin() + 1, {2, 3});
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(v, (std::vector{1, 2, 3, 4}));
}

TEST(Vector, EmplaceBackReturnsReference) {
    std::vector<int> v;
    int& r = v.emplace_back(42);
    EXPECT_EQ(&r, &v.back());
    EXPECT_EQ(r, 42);
}

TEST(Vector, EraseAndEraseIfFreeFunctions) {
    std::vector<int> v{1, 2, 3, 2, 1, 2};
    auto removed = std::erase(v, 2);
    EXPECT_EQ(removed, 3U);
    EXPECT_EQ(v, (std::vector{1, 3, 1}));

    std::vector<int> w{1, 2, 3, 4, 5};
    auto removed_if = std::erase_if(w, [](int x) { return x % 2 == 0; });
    EXPECT_EQ(removed_if, 2U);
    EXPECT_EQ(w, (std::vector{1, 3, 5}));
}

TEST(Vector, AssignReplacesContent) {
    std::vector<int> v{1, 2, 3};
    v.assign(4, 7);
    EXPECT_EQ(v, (std::vector{7, 7, 7, 7}));
}

TEST(VectorBool, ProxyAssignmentWritesThrough) {
    std::vector<bool> vb(3, false);
    auto bit = vb[1];  // 代理对象
    bit = true;
    EXPECT_TRUE(vb[1]);
    EXPECT_FALSE(vb[0]);

    vb.flip();
    EXPECT_TRUE(vb[0]);
    EXPECT_FALSE(vb[1]);
}

TEST(VectorBool, ProxyTypeIsNotBoolReference) {
    std::vector<bool> vb(1, false);
    using R = decltype(vb[0]);
    static_assert(!std::is_same_v<R, bool&>);
}
