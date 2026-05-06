// 搜索算法：find / find_if / find_first_of / search / adjacent_find / lower_bound / equal_range。

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace stdr = std::ranges;

TEST(AlgoFind, FindReturnsEndWhenMissing) {
    std::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(stdr::find(v, 3) - v.begin(), 2);
    EXPECT_EQ(stdr::find(v, 99), v.end());
}

TEST(AlgoFind, FindIfWithPredicate) {
    std::vector<int> v{1, 3, 5, 8, 9};
    auto it = stdr::find_if(v, [](int x) { return x % 2 == 0; });
    ASSERT_NE(it, v.end());
    EXPECT_EQ(*it, 8);
}

TEST(AlgoFind, FindFirstOfPicksFromSet) {
    std::vector<int> haystack{10, 20, 30, 40, 50};
    std::vector<int> needles{30, 40};
    auto it = stdr::find_first_of(haystack, needles);
    ASSERT_NE(it, haystack.end());
    EXPECT_EQ(*it, 30);
}

TEST(AlgoFind, AdjacentFind) {
    std::vector<int> v{1, 2, 3, 3, 4};
    auto it = stdr::adjacent_find(v);
    ASSERT_NE(it, v.end());
    EXPECT_EQ(it - v.begin(), 2);  // 第一个 3
}

TEST(AlgoSearch, FindsSubsequence) {
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    std::vector<int> pat{3, 4, 5};
    auto it = stdr::search(v, pat);
    ASSERT_NE(it.begin(), v.end());
    EXPECT_EQ(it.begin() - v.begin(), 2);
}

TEST(AlgoSearch, FindEndPicksLastOccurrence) {
    std::vector<int> v{1, 2, 3, 1, 2, 3, 1};
    std::vector<int> pat{1, 2};
    auto it = stdr::find_end(v, pat);
    ASSERT_NE(it.begin(), v.end());
    EXPECT_EQ(it.begin() - v.begin(), 3);
}

TEST(AlgoSearch, SearchN) {
    std::vector<int> v{1, 0, 0, 0, 2, 0, 0};
    auto it = stdr::search_n(v, 3, 0);
    ASSERT_NE(it.begin(), v.end());
    EXPECT_EQ(it.begin() - v.begin(), 1);
}

TEST(AlgoBinary, LowerUpperBoundEqualRange) {
    std::vector<int> sorted{1, 2, 3, 3, 3, 4, 5};

    auto lb = stdr::lower_bound(sorted, 3);
    auto ub = stdr::upper_bound(sorted, 3);
    EXPECT_EQ(lb - sorted.begin(), 2);
    EXPECT_EQ(ub - sorted.begin(), 5);
    EXPECT_TRUE(stdr::binary_search(sorted, 3));
    EXPECT_FALSE(stdr::binary_search(sorted, 99));

    auto er = stdr::equal_range(sorted, 3);
    EXPECT_EQ(stdr::distance(er.begin(), er.end()), 3);
}

TEST(AlgoBinary, BinarySearchUsesProjection) {
    struct Item {
        int id;
        std::string name;
    };
    std::vector<Item> items{
        {.id = 1, .name = "a"}, {.id = 3, .name = "b"}, {.id = 5, .name = "c"}};
    EXPECT_TRUE(stdr::binary_search(items, 3, std::less<>{}, &Item::id));
    EXPECT_FALSE(stdr::binary_search(items, 4, std::less<>{}, &Item::id));
}

TEST(AlgoSearch, BoyerMoore) {
    std::string text{"the quick brown fox jumps over the lazy dog"};
    std::string pat{"fox"};
    auto it = std::search(text.begin(), text.end(),
                          std::boyer_moore_searcher(pat.begin(), pat.end()));
    ASSERT_NE(it, text.end());
    EXPECT_EQ(static_cast<std::size_t>(it - text.begin()), text.find(pat));
}
