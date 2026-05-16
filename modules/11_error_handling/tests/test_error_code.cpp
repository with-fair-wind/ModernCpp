// 模块 11：std::error_code / error_condition / std::system_error 基本行为。

#include <string>
#include <system_error>

#include <gtest/gtest.h>

TEST(ErrorCode, DefaultConstructedIsSuccess) {
    std::error_code ec;
    EXPECT_FALSE(static_cast<bool>(ec));
    EXPECT_EQ(ec.value(), 0);
}

TEST(ErrorCode, MakeErrorCodeFromErrcIsNonZero) {
    auto ec = std::make_error_code(std::errc::no_such_file_or_directory);
    EXPECT_TRUE(static_cast<bool>(ec));
    EXPECT_NE(ec.value(), 0);
}

TEST(ErrorCode, ComparesWithErrorCondition) {
    auto ec = std::make_error_code(std::errc::no_such_file_or_directory);
    std::error_condition cond = std::make_error_condition(std::errc::no_such_file_or_directory);
    EXPECT_EQ(ec, cond);
}

TEST(ErrorCode, MessageIsNonEmpty) {
    auto ec = std::make_error_code(std::errc::no_such_file_or_directory);
    EXPECT_FALSE(ec.message().empty());
}

TEST(ErrorCode, CategoryNameLooksMeaningful) {
    EXPECT_FALSE(std::string{std::generic_category().name()}.empty());
    EXPECT_FALSE(std::string{std::system_category().name()}.empty());
}

TEST(ErrorCode, SystemErrorWrapsCode) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    try {
        throw std::system_error{ec, "ctx"};
        FAIL() << "未抛出";
    } catch (std::system_error const& ex) {
        EXPECT_EQ(ex.code(), ec);
    }
}
