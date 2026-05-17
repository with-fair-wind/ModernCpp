// std::ranges::move 与 std::move_backward 的可观测结果。

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(MoveAlgorithms, RangesMoveCopiesToDestination) {
    std::vector<std::string> src{"x", "yy", "zzz"};
    std::vector<std::string> dst(3);

    auto const result = std::ranges::move(src, dst.begin());

    ASSERT_EQ(result.out, dst.end());
    EXPECT_EQ(dst[0], "x");
    EXPECT_EQ(dst[1], "yy");
    EXPECT_EQ(dst[2], "zzz");

    ASSERT_FALSE(src.empty());
    EXPECT_TRUE(src[0].empty());
    EXPECT_TRUE(src[1].empty());
    EXPECT_TRUE(src[2].empty());
}

TEST(MoveAlgorithms, MoveBackwardOverlappingRanges) {
    std::vector<int> buf{10, 20, 30, 40, 50, -1, -1, -1};
    auto const first = buf.begin();
    auto const mid = buf.begin() + 3;
    auto const dest_end = buf.end();

    std::move_backward(first, mid, dest_end);

    ASSERT_GE(buf.size(), 8U);
    EXPECT_EQ(buf[5], 10);
    EXPECT_EQ(buf[6], 20);
    EXPECT_EQ(buf[7], 30);
}

}  // namespace
