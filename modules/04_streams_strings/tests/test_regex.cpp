// StdRegex：<regex> 与 regex_demo.cpp 对齐的冒烟测试。

#include <regex>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(StdRegex, RegexMatchWholeString) {
    std::regex const pattern{R"(^\d{3}$)", std::regex_constants::ECMAScript};
    EXPECT_TRUE(std::regex_match("042", pattern));
    EXPECT_FALSE(std::regex_match("42x", pattern));
}

TEST(StdRegex, RegexSearchProducesSubmatchWithCmatch) {
    std::regex const digits{R"(\d+)", std::regex_constants::ECMAScript};
    std::string const haystack{"abc 910 def"};
    std::cmatch m{};
    ASSERT_TRUE(
        std::regex_search(haystack.data(), haystack.data() + haystack.size(), m, digits));
    EXPECT_EQ(m.str(), "910");
}

TEST(StdRegex, RegexSearchExtractsCapturedGroupsWithSmatch) {
    std::regex const labeled{R"(name=(\w+);id=(\d+))"};
    std::string const line{"name=Ada;id=9"};
    std::smatch m{};
    ASSERT_TRUE(std::regex_search(line, m, labeled));
    EXPECT_EQ(m.str(), line);
    ASSERT_GE(m.size(), 3UL);
    EXPECT_EQ(m[1].str(), "Ada");
    EXPECT_EQ(m[2].str(), "9");
}

TEST(StdRegex, RegexReplaceNormalizesRunsOfWhitespace) {
    std::regex const whitespace{R"(\s+)", std::regex_constants::ECMAScript};
    std::string boxed{" alpha \t \n beta "};
    std::string const cleaned = std::regex_replace(boxed, whitespace, " ");
    EXPECT_EQ(cleaned, " alpha beta ");
}

TEST(StdRegex, SregexIteratorListsEveryMatch) {
    std::regex const token{R"(\d+)", std::regex_constants::ECMAScript};
    std::string const haystack{"no 11 or 222 here"};
    std::vector<std::string> pieces{};
    for (auto iter = std::sregex_iterator(haystack.begin(), haystack.end(), token);
         iter != std::sregex_iterator(); ++iter) {
        pieces.push_back((*iter).str());
    }
    ASSERT_EQ(pieces.size(), 2UL);
    EXPECT_EQ(pieces[0], "11");
    EXPECT_EQ(pieces[1], "222");
}

TEST(StdRegex, LazyQuantifierProducesShorterInteriorMatch) {
    std::regex const greedy{R"(<.*>)", std::regex_constants::ECMAScript};
    std::regex const lazy{R"(<.*?>)", std::regex_constants::ECMAScript};
    std::string const html{"<a><b>"};
    std::smatch g{};
    std::smatch el{};
    ASSERT_TRUE(std::regex_search(html, g, greedy));
    ASSERT_TRUE(std::regex_search(html, el, lazy));
    EXPECT_LT(el.str().size(), g.str().size());
    EXPECT_EQ(el.str(), "<a>");
}

TEST(StdRegex, IcaseAllowsCaseMismatch) {
    std::regex const phrase{R"(^OkToGo$)",
                            std::regex_constants::ECMAScript | std::regex_constants::icase};
    EXPECT_TRUE(std::regex_match("OKTOGO", phrase));
}
