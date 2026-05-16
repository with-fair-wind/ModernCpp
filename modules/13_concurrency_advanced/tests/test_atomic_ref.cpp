// 模块 13 测试：std::atomic_ref 并发递增的确定性验证。

#include <atomic>
#include <gtest/gtest.h>
#include <latch>
#include <thread>
#include <vector>

#if defined(__cpp_lib_atomic_ref) && __cpp_lib_atomic_ref >= 201806L

namespace {

TEST(AtomicRef, ConcurrentIncrementsKeepTotalOrder) {
    int shared = 0;
    constexpr int kThreads = 4;
    constexpr int kPerThread = 10'000;
    std::latch start{kThreads};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    auto worker = [&]() {
        start.arrive_and_wait();
        std::atomic_ref<int> ref{shared};
        for (int i = 0; i < kPerThread; ++i) {
            ref.fetch_add(1, std::memory_order_relaxed);
        }
    };

    while (threads.size() < static_cast<std::size_t>(kThreads)) {
        threads.emplace_back(worker);
    }

    for (auto& th : threads) {
        th.join();
    }

    EXPECT_EQ(shared, kThreads * kPerThread);
}

} // namespace

#else

TEST(AtomicRef, SkippedWhenLibMissing) {
    GTEST_SKIP() << "当前工具链未提供 std::atomic_ref";
}

#endif
