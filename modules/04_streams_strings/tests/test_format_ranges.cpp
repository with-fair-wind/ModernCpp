// C++23 std::formatter 的范围扩展：vector/tuple/pair 等组合类型。

#include <format>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#if defined(__cpp_lib_format_ranges) && (__cpp_lib_format_ranges >= 202207L)

TEST(stdFormatRanges,VectorOfInts) {
    std::vector<int> data{5, 6, 7};
    auto message = std::format("{}", data);
    EXPECT_NE(message.find('5'), std::string::npos);
    EXPECT_NE(message.find('6'), std::string::npos);
    EXPECT_NE(message.find('7'), std::string::npos);
    EXPECT_EQ(message.front(), '[');
    EXPECT_EQ(message.back(), ']');
}

TEST(stdFormatRanges,PairAndTupleNest) {
    std::pair<int, std::string> pair_value{42, std::string{"life"}};
    std::tuple<int, double, char> triple{1, 2.5, 'z'};

    auto pair_msg = std::format("{}", pair_value);
    auto tuple_msg = std::format("{}", triple);

    EXPECT_NE(pair_msg.find("life"), std::string::npos);
    EXPECT_NE(tuple_msg.find('z'), std::string::npos);
    EXPECT_NE(tuple_msg.find("2."), std::string::npos);
    EXPECT_NE(pair_msg.find('4'), std::string::npos);
    EXPECT_NE(tuple_msg.find('1'), std::string::npos);
}

#else

TEST(stdFormatRanges,FeatureNotAvailable) {
    GTEST_SKIP() << "当前标准库未定义 __cpp_lib_format_ranges";
}

#endif
