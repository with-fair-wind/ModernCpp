// 排序与划分：sort / stable_sort / partial_sort / nth_element / partition / merge / heap。

#include <algorithm>
#include <functional>
#include <ranges>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace stdr = std::ranges;

TEST(AlgoSort, SortsAscendingByDefault) {
    std::vector<int> v{5, 2, 8, 1, 9, 3};
    stdr::sort(v);
    EXPECT_EQ(v, (std::vector{1, 2, 3, 5, 8, 9}));
    EXPECT_TRUE(stdr::is_sorted(v));
}

TEST(AlgoSort, SortsWithComparator) {
    std::vector<int> v{5, 2, 8, 1, 9, 3};
    stdr::sort(v, std::greater<>{});
    EXPECT_EQ(v, (std::vector{9, 8, 5, 3, 2, 1}));
}

TEST(AlgoSort, ProjectionPicksField) {
    struct Person {
        std::string name;
        int age;
    };
    std::vector<Person> v{
        {.name = "alice", .age = 30}, {.name = "bob", .age = 25}, {.name = "carol", .age = 40}};
    stdr::sort(v, std::less<>{}, &Person::age);
    EXPECT_EQ(v[0].age, 25);
    EXPECT_EQ(v[1].age, 30);
    EXPECT_EQ(v[2].age, 40);
}

TEST(AlgoSort, StableSortKeepsEqualOrder) {
    struct Person {
        std::string name;
        int age;
    };
    std::vector<Person> v{{.name = "first30", .age = 30},
                          {.name = "second30", .age = 30},
                          {.name = "carol", .age = 25}};
    stdr::stable_sort(v, std::less<>{}, &Person::age);
    EXPECT_EQ(v[0].name, "carol");
    EXPECT_EQ(v[1].name, "first30");
    EXPECT_EQ(v[2].name, "second30");  // 与原顺序一致
}

TEST(AlgoSort, PartialSortFirstK) {
    std::vector<int> v{9, 1, 8, 2, 7, 3, 6, 4, 5};
    stdr::partial_sort(v, v.begin() + 3);
    // 前 3 个一定是 1,2,3，但顺序已排
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(AlgoSort, NthElementPlacesValue) {
    std::vector<int> v{9, 1, 8, 2, 7, 3, 6, 4, 5};
    stdr::nth_element(v, v.begin() + 4);
    // v[4] 是第 5 小（索引 4 的值）；左侧都 ≤ 它，右侧都 ≥ 它
    EXPECT_EQ(v[4], 5);
    for (int i = 0; i < 4; ++i) {
        EXPECT_LE(v[i], v[4]);
    }
    for (int i = 5; i < static_cast<int>(v.size()); ++i) {
        EXPECT_GE(v[i], v[4]);
    }
}

TEST(AlgoPartition, SeparatesByPredicate) {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7};
    auto pivot = stdr::partition(v, [](int x) { return x % 2 == 0; });
    auto first_false = pivot.begin();
    // [begin, first_false) 全为偶数；[first_false, end) 全为奇数
    for (auto it = v.begin(); it != first_false; ++it) {
        EXPECT_EQ(*it % 2, 0);
    }
    for (auto it = first_false; it != v.end(); ++it) {
        EXPECT_EQ(*it % 2, 1);
    }
}

TEST(AlgoPartition, StablePartitionPreservesOrder) {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7};
    stdr::stable_partition(v, [](int x) { return x % 2 == 0; });
    // 偶数段保持原相对顺序：2, 4, 6
    EXPECT_EQ(v[0], 2);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 6);
    // 奇数段也保持原相对顺序：1, 3, 5, 7
    EXPECT_EQ(v[3], 1);
    EXPECT_EQ(v[4], 3);
    EXPECT_EQ(v[5], 5);
    EXPECT_EQ(v[6], 7);
}

TEST(AlgoMerge, MergesSortedInputs) {
    std::vector<int> a{1, 3, 5};
    std::vector<int> b{2, 4, 6};
    std::vector<int> dst(a.size() + b.size());
    stdr::merge(a, b, dst.begin());
    EXPECT_EQ(dst, (std::vector{1, 2, 3, 4, 5, 6}));
}

TEST(AlgoHeap, MakeHeapAndPushPop) {
    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};
    stdr::make_heap(v);
    EXPECT_TRUE(stdr::is_heap(v));
    EXPECT_EQ(v.front(), 9);  // 默认最大堆

    v.push_back(100);
    stdr::push_heap(v);
    EXPECT_EQ(v.front(), 100);

    stdr::pop_heap(v);  // 把堆顶交换到末尾
    EXPECT_EQ(v.back(), 100);
}

TEST(AlgoHeap, SortHeapTurnsHeapIntoSorted) {
    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};
    stdr::make_heap(v);
    stdr::sort_heap(v);
    EXPECT_TRUE(stdr::is_sorted(v));
}
