// 模块 13 测试：内存序下的可见性（acquire-release 与 relaxed 计数）。

#include <atomic>
#include <gtest/gtest.h>
#include <latch>
#include <string>
#include <thread>
#include <vector>

namespace {

TEST(MemoryOrder, ReleaseAcquirePublishesPayload) {
    std::string payload;
    std::atomic<bool> ready{false};
    std::latch consumer_started{1};

    std::thread producer([&]() {
        payload = "ping";
        ready.store(true, std::memory_order_release);
    });

    std::thread cons([&]() {
        consumer_started.count_down();
        while (!ready.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        EXPECT_EQ(payload, "ping");
    });

    consumer_started.wait();
    cons.join();
    producer.join();
}

TEST(MemoryOrder, RelaxedFetchAddSum) {
    std::atomic<int> counter{0};
    constexpr int kThreads = 4;
    constexpr int kEach = 5'000;
    std::latch start{kThreads};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    auto worker = [&]() {
        start.arrive_and_wait();
        for (int n = 0; n < kEach; ++n) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    while (threads.size() < static_cast<std::size_t>(kThreads)) {
        threads.emplace_back(worker);
    }

    for (auto& th : threads) {
        th.join();
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), kThreads * kEach);
}

} // namespace
