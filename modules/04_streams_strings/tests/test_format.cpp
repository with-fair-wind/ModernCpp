// std::format 的常见格式说明符合并行为。

#include <format>
#include <string>

#include <gtest/gtest.h>

TEST(stdFormat,IntegerRepresentations) {
    int const value = -15;
    auto dec = std::format("{:+d}", value);
    auto hex_upper = std::format("{:#X}", 255);
    auto oct = std::format("{:#o}", 64);

    EXPECT_EQ(dec, "-15");
    EXPECT_EQ(hex_upper, "0XFF");
    EXPECT_EQ(oct, "0100");
}

TEST(stdFormat,AlignmentAndPadding) {
    EXPECT_EQ(std::format("{:_>10}", "ok"), "________ok");
    EXPECT_EQ(std::format("{:^6}", 42), "  42  ");
}

TEST(stdFormat,FloatingFormatting) {
    double const sample = 1.5;  // 二进制浮点可精确表示，避免四舍五入差异
    auto fixed_three = std::format("{:.3f}", sample);
    auto scientific = std::format("{:.2e}", 1500.25);
    auto width_prec = std::format("{:10.2f}", sample);
    EXPECT_EQ(fixed_three, "1.500");
    EXPECT_NE(scientific.find('e'), std::string::npos);
    EXPECT_EQ(width_prec.size(), 10U);
    EXPECT_NE(width_prec.find("1.50"), std::string::npos);
}

TEST(stdFormat,BooleanFormatting) {
    EXPECT_EQ(std::format("{}", false), "false");
    EXPECT_EQ(std::format("{:s}", true), "true");

    EXPECT_EQ(std::format("{:^6s}", std::string{"yo"}), "  yo  ");
}
