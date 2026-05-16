// 模块 12：信号量 try_acquire / acquire / release（无 sleep，完全确定性）。

#include <latch>
#include <semaphore>
#include <thread>

#include <gtest/gtest.h>

TEST(Semaphore, TryAcquireUntilExhausted) {
    std::counting_semaphore<8> sem{2};
    EXPECT_TRUE(sem.try_acquire());
    EXPECT_TRUE(sem.try_acquire());
    EXPECT_FALSE(sem.try_acquire());
    sem.release();
    EXPECT_TRUE(sem.try_acquire());
}

TEST(Semaphore, BlockingAcquireWakesOnRelease) {
    std::binary_semaphore gate{0};
    std::latch thread_started{1};
    std::latch acquired{1};
    std::thread waiter([&gate, &thread_started, &acquired] {
        thread_started.count_down();
        gate.acquire();
        acquired.count_down();
    });
    thread_started.wait();
    gate.release();
    acquired.wait();
    waiter.join();
}
