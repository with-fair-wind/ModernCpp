// Ranges 基础视图：iota / filter / take / drop / take_while / drop_while
// reverse / transform / stride / keys / values / elements / as_const。

#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

namespace stdv = std::views;
namespace stdr = std::ranges;

TEST(RangesViews, IotaFiniteAndInfinite) {
    auto finite = stdv::iota(1, 5);
    EXPECT_EQ(stdr::distance(finite), 4);
    EXPECT_EQ(*finite.begin(), 1);

    auto first10 = stdv::iota(0) | stdv::take(10);
    EXPECT_EQ(stdr::distance(first10), 10);
    EXPECT_EQ(*first10.begin(), 0);
}

TEST(RangesViews, FilterAndTake) {
    auto odd_first3 =
        stdv::iota(1, 100) | stdv::filter([](int x) { return x % 2 == 1; }) | stdv::take(3);
    auto out = odd_first3 | stdr::to<std::vector<int>>();
    EXPECT_EQ(out, (std::vector{1, 3, 5}));
}

TEST(RangesViews, TakeWhileAndDropWhile) {
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    auto tw = v | stdv::take_while([](int x) { return x < 4; });
    auto dw = v | stdv::drop_while([](int x) { return x < 4; });
    EXPECT_EQ((tw | stdr::to<std::vector<int>>()), (std::vector{1, 2, 3}));
    EXPECT_EQ((dw | stdr::to<std::vector<int>>()), (std::vector{4, 5, 6}));
}

TEST(RangesViews, ReverseRequiresBidirectional) {
    std::vector<int> v{1, 2, 3, 4};
    auto rv = v | stdv::reverse | stdr::to<std::vector<int>>();
    EXPECT_EQ(rv, (std::vector{4, 3, 2, 1}));
}

TEST(RangesViews, TransformIsLazy) {
    int call_count = 0;
    auto twice = std::vector{1, 2, 3} | stdv::transform([&](int x) {
                     ++call_count;
                     return x * 2;
                 });
    // 仅当迭代时才会调用 transform——迭代前 call_count == 0
    EXPECT_EQ(call_count, 0);
    auto out = twice | stdr::to<std::vector<int>>();
    EXPECT_EQ(out, (std::vector{2, 4, 6}));
    EXPECT_EQ(call_count, 3);
}

TEST(RangesViews, FilterWritesThroughToContainer) {
    std::vector<int> v{1, 2, 3, 4, 5};
    for (int& x : v | stdv::filter([](int x) { return x % 2 == 0; })) {
        x *= 10;
    }
    EXPECT_EQ(v, (std::vector{1, 20, 3, 40, 5}));
}

TEST(RangesViews, StrideMatchesPythonRangeStep) {
    auto strided = stdv::iota(1, 10) | stdv::stride(3) | stdr::to<std::vector<int>>();
    EXPECT_EQ(strided, (std::vector{1, 4, 7}));
}

TEST(RangesViews, KeysAndValuesOnMap) {
    std::map<int, std::string> m{{1, "a"}, {2, "b"}, {3, "c"}};
    auto ks = m | stdv::keys | stdr::to<std::vector<int>>();
    auto vs = m | stdv::values | stdr::to<std::vector<std::string>>();
    EXPECT_EQ(ks, (std::vector{1, 2, 3}));
    EXPECT_EQ(vs, (std::vector<std::string>{"a", "b", "c"}));
}

TEST(RangesViews, ValuesIsWritable) {
    std::map<int, int> m{{1, 10}, {2, 20}};
    for (int& v : m | stdv::values) {
        v += 1;
    }
    EXPECT_EQ(m.at(1), 11);
    EXPECT_EQ(m.at(2), 21);
}

TEST(RangesViews, ElementsExtractsTupleColumn) {
    std::vector<std::tuple<int, std::string, double>> rows{
        {1, "x", 1.5}, {2, "y", 2.5}, {3, "z", 3.5}};
    auto ids = rows | stdv::elements<0> | stdr::to<std::vector<int>>();
    auto vals = rows | stdv::elements<2> | stdr::to<std::vector<double>>();
    EXPECT_EQ(ids, (std::vector{1, 2, 3}));
    EXPECT_EQ(vals, (std::vector{1.5, 2.5, 3.5}));
}

TEST(RangesViews, AsConstBlocksWriting) {
    std::vector<int> v{1, 2, 3};
    using ElemT = std::remove_reference_t<decltype(*(v | stdv::as_const).begin())>;
    static_assert(std::is_const_v<ElemT>);
}

TEST(RangesViews, CommonViewMakesIteratorAndSentinelSameType) {
    auto rng = stdv::iota(1, 6) | stdv::common;
    static_assert(std::is_same_v<decltype(rng.begin()), decltype(rng.end())>);
    EXPECT_EQ(stdr::distance(rng), 5);
}
