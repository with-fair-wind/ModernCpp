// 数值算法：iota / accumulate / partial_sum / inner_product / reduce / fold。

#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <vector>

#include <gtest/gtest.h>

namespace stdr = std::ranges;
namespace stdv = std::views;

TEST(AlgoNumeric, IotaFillsIncrement) {
    std::vector<int> v(5);
    stdr::iota(v, 10);
    EXPECT_EQ(v, (std::vector{10, 11, 12, 13, 14}));
}

TEST(AlgoNumeric, AccumulateInitTypeMatters) {
    std::vector<double> v{0.2, 0.3};
    // 初值是 int → 中间累加按 int，结果会被截断（这正是测试要核实的"陷阱"）
    // NOLINTNEXTLINE(bugprone-fold-init-type)
    int truncated = std::accumulate(v.begin(), v.end(), 0);
    EXPECT_EQ(truncated, 0);

    // 浮点初值才能保住小数
    double real = std::accumulate(v.begin(), v.end(), 0.0);
    EXPECT_DOUBLE_EQ(real, 0.5);
}

TEST(AlgoNumeric, AccumulateWithMultiplies) {
    std::vector<int> v{1, 2, 3, 4, 5};
    int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<>{});
    EXPECT_EQ(product, 120);
}

TEST(AlgoNumeric, PartialSum) {
    std::vector<int> v{1, 2, 3, 4, 5};
    std::vector<int> prefix(v.size());
    std::partial_sum(v.begin(), v.end(), prefix.begin());
    EXPECT_EQ(prefix, (std::vector{1, 3, 6, 10, 15}));
}

TEST(AlgoNumeric, AdjacentDifference) {
    std::vector<int> v{1, 3, 6, 10, 15};
    std::vector<int> diffs(v.size());
    std::adjacent_difference(v.begin(), v.end(), diffs.begin());
    // 第一个保留原值；之后是相邻差分
    EXPECT_EQ(diffs, (std::vector{1, 2, 3, 4, 5}));
}

TEST(AlgoNumeric, InnerProductDot) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    int dot = std::inner_product(a.begin(), a.end(), b.begin(), 0);
    EXPECT_EQ(dot, 4 + 10 + 18);
}

TEST(AlgoNumeric, ReduceMatchesAccumulateForCommutativeOps) {
    std::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(std::reduce(v.begin(), v.end(), 0), 15);
    EXPECT_EQ(std::reduce(v.begin(), v.end(), 1, std::multiplies<>{}), 120);
}

TEST(AlgoNumeric, InclusiveExclusiveScan) {
    std::vector<int> v{1, 2, 3, 4, 5};

    std::vector<int> incl(v.size());
    std::inclusive_scan(v.begin(), v.end(), incl.begin());
    EXPECT_EQ(incl, (std::vector{1, 3, 6, 10, 15}));

    std::vector<int> excl(v.size());
    std::exclusive_scan(v.begin(), v.end(), excl.begin(), 0);
    EXPECT_EQ(excl, (std::vector{0, 1, 3, 6, 10}));
}

TEST(AlgoNumeric, TransformReduce) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    int dot = std::transform_reduce(a.begin(), a.end(), b.begin(), 0);
    EXPECT_EQ(dot, 32);

    int sum_squares =
        std::transform_reduce(a.begin(), a.end(), 0, std::plus<>{}, [](int x) { return x * x; });
    EXPECT_EQ(sum_squares, 1 + 4 + 9);
}

TEST(AlgoNumeric, FoldLeftAndFirst) {
    auto sum = stdr::fold_left(stdv::iota(1, 11), 0, std::plus<>{});
    EXPECT_EQ(sum, 55);

    auto product = stdr::fold_left_first(std::vector{1, 2, 3, 4, 5}, std::multiplies<>{});
    ASSERT_TRUE(product.has_value());
    EXPECT_EQ(*product, 120);
}

TEST(AlgoNumeric, GcdLcmMidpoint) {
    EXPECT_EQ(std::gcd(12, 18), 6);
    EXPECT_EQ(std::gcd(0, 7), 7);
    EXPECT_EQ(std::gcd(0, 0), 0);

    EXPECT_EQ(std::lcm(4, 6), 12);
    EXPECT_EQ(std::lcm(0, 5), 0);

    EXPECT_EQ(std::midpoint(1, 9), 5);
    EXPECT_EQ(std::midpoint(2, 7), 4);  // 截断到 4，不是 4.5
}
