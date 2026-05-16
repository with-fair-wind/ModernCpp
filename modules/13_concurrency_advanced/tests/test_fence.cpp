// 模块 13 测试：atomic_thread_fence 与 relaxed 原子配合形成同步。

#include <atomic>
#include <gtest/gtest.h>
#include <latch>
#include <thread>

namespace {

TEST(Fence, ReleaseAcquirePairEstablishesHappensBefore) {
    std::atomic<bool> x{false};
    std::atomic<bool> y{false};
    std::atomic<int> z{0};
    std::latch second_ready{1};

    std::thread first([&]() {
        x.store(true, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
        y.store(true, std::memory_order_relaxed);
    });

    std::thread second([&]() {
        second_ready.count_down();
        while (!y.load(std::memory_order_relaxed)) {
            std::this_thread::yield();
        }
        std::atomic_thread_fence(std::memory_order_acquire);
        if (x.load(std::memory_order_relaxed)) {
            z.fetch_add(1, std::memory_order_relaxed);
        }
    });

    second_ready.wait();
    first.join();
    second.join();

    EXPECT_EQ(z.load(), 1);
}

} // namespace
