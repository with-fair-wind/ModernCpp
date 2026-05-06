// std::less<> / std::equal_to<> / 自定义 transparent hash 的异构查找。

#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <gtest/gtest.h>

namespace {

struct StringHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
    std::size_t operator()(const std::string& s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
    std::size_t operator()(const char* s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
};

}  // namespace

TEST(TransparentLess, SetAcceptsHeterogeneousKey) {
    std::set<std::string, std::less<>> s{"alpha", "beta", "gamma"};

    EXPECT_TRUE(s.contains("alpha"));                            // 字面量 char[]
    EXPECT_TRUE(s.contains(std::string_view{"beta"}));           // string_view
    EXPECT_TRUE(s.contains(std::string{"gamma"}));               // 同型
    EXPECT_FALSE(s.contains("delta"));
}

TEST(TransparentLess, MapLowerBoundOnHeterogeneousKey) {
    std::map<std::string, int, std::less<>> m{{"a", 1}, {"c", 3}, {"e", 5}};

    auto lb = m.lower_bound(std::string_view{"b"});
    ASSERT_NE(lb, m.end());
    EXPECT_EQ(lb->first, "c");

    auto ub = m.upper_bound("c");
    ASSERT_NE(ub, m.end());
    EXPECT_EQ(ub->first, "e");
}

TEST(TransparentHash, UnorderedSetWithCustomHash) {
    std::unordered_set<std::string, StringHash, std::equal_to<>> s{"alpha", "beta", "gamma"};

    EXPECT_TRUE(s.contains("alpha"));
    EXPECT_TRUE(s.contains(std::string_view{"beta"}));
    EXPECT_FALSE(s.contains("delta"));
}

TEST(TransparentHash, UnorderedMapWithCustomHash) {
    std::unordered_map<std::string, int, StringHash, std::equal_to<>> m{
        {"alpha", 1}, {"beta", 2}, {"gamma", 3}};

    EXPECT_EQ(m.find("alpha")->second, 1);
    EXPECT_EQ(m.find(std::string_view{"beta"})->second, 2);
    EXPECT_EQ(m.find(std::string{"gamma"})->second, 3);
    EXPECT_TRUE(m.contains("alpha"));
    EXPECT_FALSE(m.contains("missing"));
}

TEST(TransparentHash, EquivalentKeysHashTheSame) {
    StringHash h;
    auto h_cstr = h("hello");
    auto h_sv = h(std::string_view{"hello"});
    auto h_str = h(std::string{"hello"});
    EXPECT_EQ(h_cstr, h_sv);
    EXPECT_EQ(h_sv, h_str);
}
