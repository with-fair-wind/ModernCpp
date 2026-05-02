// Smoke test for module 12: jthread / stop_token cooperative cancellation.

#include <atomic>
#include <latch>
#include <stop_token>
#include <thread>

#include <gtest/gtest.h>

// These tests previously relied on `sleep_for(20ms)` to give the worker time
// to make progress before the main thread observed it. That's flaky on busy
// CI runners (heavy load can suspend the worker past the deadline). std::latch
// gives us a deterministic happens-before edge: the worker counts down once it
// has made an observable side effect, and main blocks on wait() until then.

TEST(JThread, RequestStopCausesCooperativeExit) {
    std::atomic<int> ticks{0};
    std::latch first_tick{1};
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) - canonical jthread signature
        std::jthread worker([&](std::stop_token st) {
            ticks.fetch_add(1, std::memory_order_relaxed);
            first_tick.count_down();
            while (!st.stop_requested()) {
                ticks.fetch_add(1, std::memory_order_relaxed);
                std::this_thread::yield();
            }
        });
        first_tick.wait();
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
    std::latch entered{1};
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) - canonical jthread signature
        std::jthread worker([&](std::stop_token st) {
            ran.store(true);
            entered.count_down();
            while (!st.stop_requested()) {
                std::this_thread::yield();
            }
        });
        entered.wait();
    }
    EXPECT_TRUE(ran.load());
}
