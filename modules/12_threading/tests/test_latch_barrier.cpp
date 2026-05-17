// 模块 12：std::latch 一次性栅栏与 std::barrier 可复用栅栏。

#include <atomic>
#include <barrier>
#include <latch>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

TEST(Latch, AllPartiesReleaseWaiters) {
    constexpr int kParties = 5;
    std::latch arrive{kParties};
    std::atomic<int> progressed{0};

    std::vector<std::thread> threads;
    threads.reserve(kParties);
    for (int i = 0; i < kParties; ++i) {
        threads.emplace_back([&arrive, &progressed] {
            arrive.arrive_and_wait();
            progressed.fetch_add(1);
        });
    }

    arrive.wait();
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(progressed.load(), kParties);
}

TEST(Barrier, CompletionRunsOncePerPhase) {
    constexpr int kParties = 3;
    std::atomic<int> completions{0};
    std::barrier sync(kParties, [&completions]() noexcept { completions.fetch_add(1); });

    auto two_phases = [&sync] {
        sync.arrive_and_wait();
        sync.arrive_and_wait();
    };

    std::thread a(two_phases);
    std::thread b(two_phases);
    two_phases();
    a.join();
    b.join();

    EXPECT_EQ(completions.load(), 2);
}
