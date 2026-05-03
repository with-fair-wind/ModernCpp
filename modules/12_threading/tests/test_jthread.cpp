// 模块 12 的烟雾测试：jthread / stop_token 的协作式取消。

#include <atomic>
#include <latch>
#include <stop_token>
#include <thread>

#include <gtest/gtest.h>

// 这些用例此前依赖 `sleep_for(20ms)` 给工作线程时间做出可观测进展，
// 但在繁忙的 CI runner 上会偶发失败（高负载可能让工作线程被挂起
// 超过截止时间）。改用 std::latch 提供确定性的 happens-before 边：
// 工作线程一旦产生可观测副作用就 count_down，主线程在 wait() 上阻塞，
// 直到看到那一下信号。

TEST(JThread, RequestStopCausesCooperativeExit) {
    std::atomic<int> ticks{0};
    std::latch first_tick{1};
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) - jthread 的标准签名要求按值传递
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
        // jthread 析构时会 join。
    }
    EXPECT_GT(ticks.load(), 0);
}

TEST(JThread, DestructorJoinsWithoutExplicitRequestStop) {
    // jthread 的析构会先调用 request_stop()，再 join() —— 因此只要
    // 工作线程检视了 stop_token，就算调用方忘了显式请求，线程也一定
    // 会终止。
    std::atomic<bool> ran{false};
    std::latch entered{1};
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) - jthread 的标准签名要求按值传递
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
