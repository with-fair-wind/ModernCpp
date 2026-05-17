// move_iterator / make_move_iterator + copy：读取端表现为移动语义。

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(MoveIterators, MakeMoveIteratorEmptiesSourceStrings) {
    std::vector<std::string> src{"apple", "banana"};
    std::vector<std::string> dst;

    std::copy(std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()),
              std::back_inserter(dst));

    ASSERT_EQ(dst.size(), 2U);
    EXPECT_EQ(dst[0], "apple");
    EXPECT_EQ(dst[1], "banana");

    ASSERT_EQ(src.size(), 2U);
    EXPECT_TRUE(src[0].empty());
    EXPECT_TRUE(src[1].empty());
}

TEST(MoveIterators, MoveIteratorRangePreservesOrder) {
    std::vector<std::string> pool{"north", "east"};
    std::vector<std::string> sink(2);

    auto first = std::move_iterator(pool.begin());
    auto last = std::move_iterator(pool.end());
    std::copy(first, last, sink.begin());

    EXPECT_EQ(sink[0], "north");
    EXPECT_EQ(sink[1], "east");
    EXPECT_TRUE(pool[0].empty());
    EXPECT_TRUE(pool[1].empty());
}

}  // namespace
