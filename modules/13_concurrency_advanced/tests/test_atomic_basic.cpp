// 模块 13 测试：atomic 基本 load / store / exchange / fetch_add。

#include <atomic>
#include <gtest/gtest.h>

namespace {

TEST(AtomicBasic, LoadStoreRoundTrip) {
    std::atomic<int> v{0};
    v.store(7);
    EXPECT_EQ(v.load(), 7);
}

TEST(AtomicBasic, ExchangeReturnsOldAndSetsNew) {
    std::atomic<int> a{1};
    const int old = a.exchange(42);
    EXPECT_EQ(old, 1);
    EXPECT_EQ(a.load(), 42);
}

TEST(AtomicBasic, FetchAddAccumulator) {
    std::atomic<int> sum{0};
    EXPECT_EQ(sum.fetch_add(5), 0);
    EXPECT_EQ(sum.fetch_add(3), 5);
    EXPECT_EQ(sum.load(), 8);
}

TEST(AtomicBasic, CompareExchangeStrongSuccess) {
    std::atomic<int> a{100};
    int expected = 100;
    EXPECT_TRUE(a.compare_exchange_strong(expected, 200));
    EXPECT_EQ(expected, 100);
    EXPECT_EQ(a.load(), 200);
}

TEST(AtomicBasic, CompareExchangeStrongFailureUpdatesExpected) {
    std::atomic<int> a{5};
    int expected = 99;
    EXPECT_FALSE(a.compare_exchange_strong(expected, 0));
    EXPECT_EQ(expected, 5);
    EXPECT_EQ(a.load(), 5);
}

} // namespace
