// 模块 13 测试：std::atomic_flag 的 test_and_set / clear。

#include <atomic>
#include <latch>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(AtomicFlag, TestAndSetIsSerializing) {
    std::atomic_flag f = ATOMIC_FLAG_INIT;
    EXPECT_FALSE(f.test_and_set(std::memory_order_acquire));
    EXPECT_TRUE(f.test_and_set(std::memory_order_acquire));
    f.clear(std::memory_order_release);
    EXPECT_FALSE(f.test_and_set(std::memory_order_acquire));
}

TEST(AtomicFlag, SpinLockExcludesConcurrentCriticalSection) {
    std::atomic_flag f = ATOMIC_FLAG_INIT;
    std::atomic<int> depth{0};
    constexpr int kThreads = 4;
    constexpr int kIterations = 2'000;
    std::latch start{kThreads};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    auto worker = [&]() {
        start.arrive_and_wait();
        for (int round = 0; round < kIterations; ++round) {
            while (f.test_and_set(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            EXPECT_EQ(depth.fetch_add(1, std::memory_order_acq_rel), 0);
            EXPECT_EQ(depth.fetch_sub(1, std::memory_order_acq_rel), 1);
            f.clear(std::memory_order_release);
        }
    };

    while (threads.size() < static_cast<std::size_t>(kThreads)) {
        threads.emplace_back(worker);
    }

    for (auto& th : threads) {
        th.join();
    }
}

}  // namespace
