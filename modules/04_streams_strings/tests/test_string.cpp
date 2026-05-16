// std::string 高频 API（查找、starts_with、contains、substr）与 SSO 现象的粗粒度观测。

#include <cstddef>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace {

[[nodiscard]] bool byteInObjectStorage(std::string const& string_value) noexcept {
    const auto *const object_begin = reinterpret_cast<unsigned char const*>(&string_value);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto *const object_end = object_begin + sizeof(string_value);  // NOLINT(bugprone-sizeof-container)
    const auto *const data_ptr = reinterpret_cast<unsigned char const*>(string_value.data());  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    return data_ptr >= object_begin && data_ptr < object_end;
}

}  // namespace

TEST(stdStringApis,FindAndSubstr) {
    std::string const hay{"abracadabra"};
    EXPECT_EQ(hay.find("bra"), 1U);
    EXPECT_EQ(hay.rfind('a'), hay.size() - 1);
    EXPECT_EQ(hay.substr(4, 3), "cad");
}

#ifdef __cpp_lib_constexpr_string

TEST(stdStringApis,PrefixesSuffixesMembership) {
    std::string const text{"streaming"};
    EXPECT_TRUE(text.starts_with("stream"));
    EXPECT_TRUE(text.ends_with("ing"));
    EXPECT_FALSE(text.ends_with('X'));
}

TEST(stdStringApis,ContainsSubsequence) {
    std::string const blob{"needle in hay"};
    EXPECT_TRUE(blob.contains("needle"));
    EXPECT_FALSE(blob.contains("thread"));
}

#endif

TEST(stdStringApis,SsoRoughCheck) {
    std::string const short_literal{"yo"};
    std::string long_string(96, '#');
    EXPECT_TRUE(short_literal.size() <= short_literal.capacity());
    EXPECT_LT(short_literal.capacity(), long_string.capacity());

    EXPECT_TRUE(byteInObjectStorage(short_literal));
}

TEST(stdStringApis,ConcatAndAssignments) {
    std::string acc;
    acc += "a";
    acc.append(std::string_view{"bc"});
    acc.push_back('d');
    EXPECT_EQ(acc.size(), 4U);
    EXPECT_EQ(acc, "abcd");
    EXPECT_EQ(acc.front(), 'a');
    EXPECT_EQ(acc.back(), 'd');
}
