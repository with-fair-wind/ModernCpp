// 顺序容器（除 vector / array）：deque、list、forward_list 的关键 API。

#include <deque>
#include <forward_list>
#include <list>

#include <gtest/gtest.h>

TEST(Deque, PushFrontAndBack) {
    std::deque<int> d;
    d.push_back(2);
    d.push_back(3);
    d.push_front(1);
    d.push_front(0);
    EXPECT_EQ(d.front(), 0);
    EXPECT_EQ(d.back(), 3);
    EXPECT_EQ(d.size(), 4U);
    EXPECT_EQ(d[2], 2);  // 随机访问
}

TEST(Deque, ReferenceStableAcrossEndPushes) {
    std::deque<int> d{1, 2, 3};
    int& back_ref = d.back();
    int* back_addr = &back_ref;
    d.push_front(0);
    d.push_front(-1);
    // 端点 push 不会重定位已存在的元素（块本身不变）
    EXPECT_EQ(back_addr, &d.back());
    EXPECT_EQ(*back_addr, 3);
}

TEST(List, SpliceMovesNodes) {
    std::list<int> a{1, 2};
    std::list<int> b{10, 20, 30};
    a.splice(a.end(), b);
    EXPECT_EQ(a.size(), 5U);
    EXPECT_EQ(b.size(), 0U);  // 节点已被移走
    EXPECT_EQ(a, (std::list{1, 2, 10, 20, 30}));
}

TEST(List, SortIsStable) {
    struct Item {
        int key;
        int idx;
        bool operator==(const Item&) const = default;
    };
    std::list<Item> items{
        {.key = 2, .idx = 0}, {.key = 1, .idx = 1}, {.key = 2, .idx = 2}, {.key = 1, .idx = 3}};
    items.sort([](Item const& l, Item const& r) { return l.key < r.key; });

    auto it = items.begin();
    EXPECT_EQ(it->idx, 1);  // key=1, idx=1
    ++it;
    EXPECT_EQ(it->idx, 3);  // key=1, idx=3 → 在 idx=1 之后（稳定）
    ++it;
    EXPECT_EQ(it->idx, 0);
    ++it;
    EXPECT_EQ(it->idx, 2);
}

TEST(List, UniqueAndRemoveIfReturnCount) {
    std::list<int> nums{1, 1, 2, 3, 3, 3, 4};
    auto removed = nums.unique();
    EXPECT_EQ(removed, 3U);  // 去掉 3 个相邻重复
    EXPECT_EQ(nums, (std::list{1, 2, 3, 4}));

    auto removed_if = nums.remove_if([](int x) { return x % 2 == 0; });
    EXPECT_EQ(removed_if, 2U);
    EXPECT_EQ(nums, (std::list{1, 3}));
}

TEST(List, MergeOnSortedLists) {
    std::list<int> a{1, 3, 5};
    std::list<int> b{2, 4, 6};
    a.merge(b);
    EXPECT_EQ(a, (std::list{1, 2, 3, 4, 5, 6}));
    EXPECT_EQ(b.size(), 0U);
}

TEST(ForwardList, BeforeBeginAndInsertAfter) {
    std::forward_list<int> fl{2, 3, 4};
    // 在头部之前插入：用 before_begin
    fl.insert_after(fl.before_begin(), 1);
    auto it = fl.begin();
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 2);
}

TEST(ForwardList, EraseAfter) {
    std::forward_list<int> fl{1, 2, 3, 4};
    auto it = fl.before_begin();
    fl.erase_after(it);  // 删 1
    EXPECT_EQ(fl.front(), 2);
}

TEST(ForwardList, SpliceAfter) {
    std::forward_list<int> a{1, 2};
    std::forward_list<int> b{10, 20};
    auto a_first = a.begin();    // 指向 1
    a.splice_after(a_first, b);  // 在 1 之后接入 b 的全部
    auto it = a.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 10);
    EXPECT_EQ(*it++, 20);
    EXPECT_EQ(*it++, 2);
    EXPECT_TRUE(b.empty());
}
