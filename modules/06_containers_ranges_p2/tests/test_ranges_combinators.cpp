// 多 range 组合：zip / cartesian_product / chunk / chunk_by / slide / adjacent / pairwise。

#include <functional>
#include <iterator>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace stdv = std::views;
namespace stdr = std::ranges;

TEST(RangesZip, MatchesShorterRange) {
    std::vector<int> a{1, 2, 3, 4};
    std::vector<std::string> b{"x", "y", "z"};  // 更短
    auto zipped = stdv::zip(a, b);
    EXPECT_EQ(stdr::distance(zipped), 3);

    std::vector<std::pair<int, std::string>> got;
    for (auto const& [x, y] : zipped) {
        got.emplace_back(x, y);
    }
    ASSERT_EQ(got.size(), 3U);
    EXPECT_EQ(got[0].first, 1);
    EXPECT_EQ(got[0].second, "x");
}

TEST(RangesZip, ReturnsTupleOfReferences) {
    std::vector<int> v{1, 2, 3};
    std::vector<int> w{10, 20, 30};
    for (auto [x, y] : stdv::zip(v, w)) {
        x += y;
    }
    EXPECT_EQ(v, (std::vector{11, 22, 33}));
    EXPECT_EQ(w, (std::vector{10, 20, 30}));  // w 未改
}

TEST(RangesZipTransform, FoldsToScalarPerStep) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{10, 20, 30};
    auto sums = stdv::zip_transform(std::plus<>{}, a, b) | stdr::to<std::vector<int>>();
    EXPECT_EQ(sums, (std::vector{11, 22, 33}));
}

TEST(RangesCartesianProduct, MatchesNxM) {
    std::vector<int> rows{1, 2, 3};
    std::vector<char> cols{'a', 'b'};
    auto prod = stdv::cartesian_product(rows, cols);
    EXPECT_EQ(stdr::distance(prod), 6);

    auto first = *prod.begin();
    EXPECT_EQ(std::get<0>(first), 1);
    EXPECT_EQ(std::get<1>(first), 'a');
}

TEST(RangesChunk, NonDivisibleHasShortLastBlock) {
    std::vector<int> v{1, 2, 3, 4, 5};
    auto chunks = v | stdv::chunk(2);
    std::vector<std::vector<int>> got;
    for (auto chunk_view : chunks) {
        got.emplace_back(chunk_view | stdr::to<std::vector<int>>());
    }
    ASSERT_EQ(got.size(), 3U);
    EXPECT_EQ(got[0], (std::vector{1, 2}));
    EXPECT_EQ(got[1], (std::vector{3, 4}));
    EXPECT_EQ(got[2], (std::vector{5}));  // 余数块
}

TEST(RangesChunkBy, BreaksWhenPredFails) {
    std::vector<int> v{1, 2, 5, 4, 3, 4};
    auto blocks = v | stdv::chunk_by(std::less<int>{});
    std::vector<std::vector<int>> got;
    for (auto block : blocks) {
        got.emplace_back(block | stdr::to<std::vector<int>>());
    }
    ASSERT_EQ(got.size(), 3U);
    EXPECT_EQ(got[0], (std::vector{1, 2, 5}));
    EXPECT_EQ(got[1], (std::vector{4}));
    EXPECT_EQ(got[2], (std::vector{3, 4}));
}

TEST(RangesSlide, WindowCountIsSizeMinusWidthPlusOne) {
    std::vector<int> v{1, 2, 3, 4, 5};
    auto windows = v | stdv::slide(3);
    EXPECT_EQ(stdr::distance(windows), 3);

    std::vector<std::vector<int>> got;
    for (auto const& win : windows) {
        got.emplace_back(win | stdr::to<std::vector<int>>());
    }
    EXPECT_EQ(got, (std::vector<std::vector<int>>{{1, 2, 3}, {2, 3, 4}, {3, 4, 5}}));
}

TEST(RangesAdjacent, ReturnsTuplesOfSize) {
    std::vector<int> v{1, 2, 3, 4};
    auto pairs = v | stdv::adjacent<2>;  // 等价于 pairwise
    int last_a = -1;
    int last_b = -1;
    int count = 0;
    for (auto const& [a, b] : pairs) {
        last_a = a;
        last_b = b;
        ++count;
    }
    EXPECT_EQ(count, 3);
    EXPECT_EQ(last_a, 3);
    EXPECT_EQ(last_b, 4);
}

TEST(RangesPairwise, ComputesAdjacentDifferences) {
    std::vector<int> v{1, 3, 6, 10, 15};
    std::vector<int> diffs;
    for (auto const& [prev, cur] : v | stdv::pairwise) {
        diffs.push_back(cur - prev);
    }
    EXPECT_EQ(diffs, (std::vector{2, 3, 4, 5}));
}
