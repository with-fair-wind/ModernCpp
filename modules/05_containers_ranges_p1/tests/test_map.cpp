// std::map / set / multimap / multiset 与节点提取。

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include <gtest/gtest.h>

TEST(Map, OperatorBracketInsertsDefault) {
    std::map<std::string, int> m;
    int& v = m["new"];  // 默认插入 0
    EXPECT_EQ(v, 0);
    EXPECT_EQ(m.size(), 1U);
    v = 7;
    EXPECT_EQ(m["new"], 7);
}

TEST(Map, AtThrowsWhenMissing) {
    std::map<std::string, int> m;
    m["a"] = 1;
    EXPECT_EQ(m.at("a"), 1);
    EXPECT_THROW(static_cast<void>(m.at("missing")), std::out_of_range);
}

TEST(Map, FindAndContains) {
    std::map<int, int> m{{1, 100}, {2, 200}};
    auto it = m.find(2);
    ASSERT_NE(it, m.end());
    EXPECT_EQ(it->second, 200);

    EXPECT_TRUE(m.contains(1));
    EXPECT_FALSE(m.contains(99));
    EXPECT_EQ(m.count(1), 1U);
    EXPECT_EQ(m.count(99), 0U);
}

TEST(Map, InsertVsInsertOrAssignVsTryEmplace) {
    std::map<std::string, int> m{{"a", 1}};

    auto [it1, ok1] = m.insert({"a", 999});
    EXPECT_FALSE(ok1);
    EXPECT_EQ(it1->second, 1);  // 失败时保留旧值

    auto [it2, ok2] = m.insert_or_assign("a", 999);
    EXPECT_FALSE(ok2);  // 表示是 assign 而非 insert
    EXPECT_EQ(it2->second, 999);

    auto [it3, ok3] = m.try_emplace("b", 42);
    EXPECT_TRUE(ok3);
    EXPECT_EQ(it3->second, 42);
}

TEST(Map, LowerUpperBoundEqualRange) {
    std::map<int, int> m{{1, 10}, {3, 30}, {5, 50}, {7, 70}};
    EXPECT_EQ(m.lower_bound(3)->first, 3);
    EXPECT_EQ(m.upper_bound(3)->first, 5);
    EXPECT_EQ(m.lower_bound(4)->first, 5);  // 不存在 → 下一个更大键

    auto [lo, hi] = m.equal_range(3);
    EXPECT_EQ(lo->first, 3);
    EXPECT_EQ(hi->first, 5);

    auto [lo2, hi2] = m.equal_range(4);
    EXPECT_EQ(lo2, hi2);  // 不存在的键：返回空区间
}

TEST(Map, ExtractAllowsKeyRename) {
    std::map<std::string, int> m{{"alice", 1}, {"bob", 2}};
    auto node = m.extract("alice");
    ASSERT_FALSE(node.empty());
    node.key() = "alex";
    auto result = m.insert(std::move(node));
    EXPECT_TRUE(result.inserted);
    EXPECT_EQ(m.count("alice"), 0U);
    EXPECT_EQ(m.at("alex"), 1);
}

TEST(Map, MergeOnlyMovesNonExisting) {
    std::map<int, std::string> dst{{1, "old1"}, {2, "old2"}};
    std::map<int, std::string> src{{2, "new2"}, {3, "new3"}};
    dst.merge(src);
    EXPECT_EQ(dst.size(), 3U);
    EXPECT_EQ(dst.at(1), "old1");
    EXPECT_EQ(dst.at(2), "old2");  // 已存在的键保留旧值
    EXPECT_EQ(dst.at(3), "new3");
    EXPECT_EQ(src.size(), 1U);  // src 中"无法移走"的留下
    EXPECT_EQ(src.at(2), "new2");
}

TEST(Multimap, EqualRangeYieldsAllMatches) {
    std::multimap<int, char> m{{1, 'a'}, {2, 'b'}, {2, 'c'}, {2, 'd'}, {3, 'e'}};
    EXPECT_EQ(m.count(2), 3U);

    auto [lo, hi] = m.equal_range(2);
    int n = 0;
    for (auto it = lo; it != hi; ++it, ++n) {
        EXPECT_EQ(it->first, 2);
    }
    EXPECT_EQ(n, 3);
}

TEST(Set, BasicOps) {
    std::set<int> s{3, 1, 4, 1, 5, 9, 2, 6};  // 自动去重 + 排序
    EXPECT_EQ(s.size(), 7U);
    EXPECT_TRUE(s.contains(1));
    EXPECT_FALSE(s.contains(7));

    // 顺序遍历得到升序
    int prev = *s.begin();
    for (auto it = std::next(s.begin()); it != s.end(); ++it) {
        EXPECT_LT(prev, *it);
        prev = *it;
    }
}

TEST(Set, NodeExchangeWithMultiset) {
    std::multiset<int> ms{1, 2, 2, 3};
    std::set<int> s;
    auto node = ms.extract(2);  // 取一个 2
    ASSERT_FALSE(node.empty());
    s.insert(std::move(node));  // 节点跨容器移动
    EXPECT_EQ(s.size(), 1U);
    EXPECT_TRUE(s.contains(2));
}
