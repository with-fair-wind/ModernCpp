// std::bitset：固定长度位串的运算与互转。

#include <bitset>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

TEST(Bitset, SizeAndIndexing) {
    std::bitset<8> bs{0b1010'0110U};
    EXPECT_EQ(bs.size(), 8U);
    EXPECT_TRUE(bs[1]);
    EXPECT_FALSE(bs[0]);
    EXPECT_TRUE(bs[7]);
}

TEST(Bitset, SetResetFlip) {
    std::bitset<4> bs;
    bs.set();  // 全 1
    EXPECT_TRUE(bs.all());
    bs.reset(2);  // 位 2 → 0
    EXPECT_FALSE(bs[2]);
    bs.flip();  // 全部翻转
    EXPECT_TRUE(bs[2]);
    EXPECT_FALSE(bs[0]);
}

TEST(Bitset, CountAnyAllNone) {
    std::bitset<8> bs{0b0010'0010U};
    EXPECT_EQ(bs.count(), 2U);
    EXPECT_TRUE(bs.any());
    EXPECT_FALSE(bs.all());
    EXPECT_FALSE(bs.none());

    std::bitset<8> zero;
    EXPECT_TRUE(zero.none());
}

TEST(Bitset, BitwiseOps) {
    std::bitset<4> a{0b1100U};
    std::bitset<4> b{0b1010U};
    EXPECT_EQ((a & b).to_ulong(), 0b1000U);
    EXPECT_EQ((a | b).to_ulong(), 0b1110U);
    EXPECT_EQ((a ^ b).to_ulong(), 0b0110U);
    EXPECT_EQ((~a).to_ulong(), 0b0011U);
    EXPECT_EQ((a << 1).to_ulong(), 0b1000U);  // 高位被截掉
    EXPECT_EQ((b >> 1).to_ulong(), 0b0101U);
}

TEST(Bitset, ConvertToString) {
    std::bitset<5> bs{0b1'0110U};
    EXPECT_EQ(bs.to_string(), "10110");
    EXPECT_EQ(bs.to_string('o', 'x'), "xoxxo");
}

TEST(Bitset, TestThrowsOutOfRange) {
    std::bitset<4> bs;
    EXPECT_THROW(static_cast<void>(bs.test(99)), std::out_of_range);
}

TEST(Bitset, ConstructFromString) {
    std::bitset<8> bs{std::string{"00001111"}};
    EXPECT_EQ(bs.to_ulong(), 0b0000'1111U);
}
