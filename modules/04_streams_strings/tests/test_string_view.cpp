// std::string_view 视图构造、切片以及与 npos 协同的语义。

#include <cstring>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

TEST(stdStringView,ConstructionAndLength) {
    std::string const owned{"view me"};
    std::string_view from_string{owned};
    std::string_view literal{"peek"};
    char stack[] = "stack";
    std::string_view from_array{stack, std::strlen(stack)};

    EXPECT_EQ(from_string.size(), 7U);
    EXPECT_EQ(literal.front(), 'p');
    EXPECT_EQ(from_array.back(), 'k');
}

TEST(stdStringView,SubstringsShareStorage) {
    std::string const base{"abcdefghi"};
    std::string_view full{base};
    auto prefix = full.substr(0, 4);
    auto suffix = full.substr(6);
    EXPECT_EQ(prefix, "abcd");
    EXPECT_EQ(suffix, "ghi");

    prefix.remove_prefix(1);
    EXPECT_EQ(prefix, "bcd");
}

TEST(stdStringView,NposMeansNotFound) {
    std::string_view text{"abc"};
    EXPECT_EQ(text.find('z'), std::string_view::npos);
    EXPECT_EQ(text.rfind("zzz"), std::string_view::npos);

    constexpr std::string_view kLiteral{"zzz"};
    static_assert(kLiteral.contains('z'));
}

TEST(stdStringView,ComparePreservesLexicalOrder) {
    std::string_view a{"baa"};
    std::string_view b{"baa!"};
    EXPECT_LT(a.compare(b), 0);
    EXPECT_EQ(a.compare("baa"), 0);
    EXPECT_GT(b.compare(a), 0);
    EXPECT_EQ(a.substr(0, 2), "ba");
}
