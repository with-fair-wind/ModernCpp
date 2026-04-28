// Smoke test for module 12: jthread / stop_token cooperative cancellation.

#include <atomic>
#include <chrono>
#include <stop_token>
#include <thread>

#include <gtest/gtest.h>

TEST(JThread, RequestStopCausesCooperativeExit) {
    std::atomic<int> ticks{0};
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) - canonical jthread signature
        std::jthread worker([&](std::stop_token st) {
            while (!st.stop_requested()) {
                ticks.fetch_add(1, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        worker.request_stop();
        // jthread dtor joins.
    }
    EXPECT_GT(ticks.load(), 0);
}

TEST(JThread, DestructorJoinsWithoutExplicitRequestStop) {
    // jthread's destructor calls request_stop() then join() — so a worker
    // that observes its stop_token is guaranteed to terminate even if the
    // caller forgets to ask.
    std::atomic<bool> ran{false};
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) - canonical jthread signature
        std::jthread worker([&](std::stop_token st) {
            ran.store(true);
            while (!st.stop_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(ran.load());
}
