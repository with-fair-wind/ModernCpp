// std::unordered_map / set：负载因子、桶接口、自定义哈希。

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <gtest/gtest.h>

namespace {

struct Point {
    int x;
    int y;
    bool operator==(const Point&) const = default;
};

struct PointHash {
    std::size_t operator()(Point const& p) const noexcept {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1U);
    }
};

}  // namespace

TEST(UnorderedMap, BasicLookup) {
    std::unordered_map<std::string, int> m;
    m["a"] = 1;
    m["b"] = 2;
    EXPECT_EQ(m.at("a"), 1);
    EXPECT_TRUE(m.contains("b"));
    EXPECT_FALSE(m.contains("missing"));
}

TEST(UnorderedMap, LoadFactorAndReserve) {
    std::unordered_map<int, int> m;
    EXPECT_EQ(m.load_factor(), 0.0F);
    EXPECT_GT(m.max_load_factor(), 0.0F);

    m.reserve(1000);
    EXPECT_GE(static_cast<float>(m.bucket_count()) * m.max_load_factor(), 1000.0F);

    for (int i = 0; i < 64; ++i) {
        m.emplace(i, i * 10);
    }
    EXPECT_LE(m.load_factor(), m.max_load_factor());
}

TEST(UnorderedMap, BucketInterface) {
    std::unordered_map<int, int> m;
    for (int i = 0; i < 32; ++i) {
        m.emplace(i, i);
    }
    auto bidx = m.bucket(5);
    EXPECT_LT(bidx, m.bucket_count());

    // bucket_size = std::distance(begin(idx), end(idx))
    auto count = std::distance(m.begin(bidx), m.end(bidx));
    EXPECT_EQ(static_cast<std::size_t>(count), m.bucket_size(bidx));
}

TEST(UnorderedSet, CustomHashFunctor) {
    std::unordered_set<Point, PointHash> s;
    s.insert({.x = 1, .y = 2});
    s.insert({.x = 3, .y = 4});
    s.insert({.x = 1, .y = 2});  // 重复 → 不插入
    EXPECT_EQ(s.size(), 2U);
    EXPECT_TRUE(s.contains(Point{.x = 1, .y = 2}));
}

TEST(UnorderedMultimap, EqualityIgnoresOrderWithinKey) {
    std::unordered_multimap<int, int> a;
    a.emplace(1, 10);
    a.emplace(1, 20);
    a.emplace(2, 30);

    std::unordered_multimap<int, int> b;
    b.emplace(2, 30);
    b.emplace(1, 20);
    b.emplace(1, 10);

    // 与 multimap 不同：unordered_multimap 的 == 只要每个键上的值集合互为排列即可。
    EXPECT_EQ(a, b);
}

TEST(UnorderedMap, EraseByKeyAndIterator) {
    std::unordered_map<int, int> m{{1, 10}, {2, 20}, {3, 30}};
    EXPECT_EQ(m.erase(2), 1U);
    EXPECT_EQ(m.size(), 2U);

    auto it = m.find(1);
    ASSERT_NE(it, m.end());
    m.erase(it);
    EXPECT_EQ(m.size(), 1U);
}
