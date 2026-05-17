// 模块 12：mutex 保护共享状态，验证多线程递增的正确性。

#include <mutex>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

TEST(Mutex, ConcurrentIncrementsPreserveTotal) {
    constexpr int kThreads = 6;
    constexpr int kIterationsPerThread = 10'000;

    int counter = 0;
    std::mutex mtx;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&counter, &mtx] {
            for (int j = 0; j < kIterationsPerThread; ++j) {
                std::scoped_lock lock(mtx);
                ++counter;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter, kThreads * kIterationsPerThread);
}
