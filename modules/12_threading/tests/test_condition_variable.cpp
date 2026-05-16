// 模块 12：condition_variable + mutex 生产者—消费者同步（确定性、无 sleep）。

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>

#include <gtest/gtest.h>

TEST(ConditionVariable, ProducerConsumerDrainsExactlyN) {
    constexpr int kTotal = 200;

    std::mutex mtx;
    std::condition_variable cv;
    std::queue<int> queue;

    std::int64_t sum = 0;

    std::thread consumer([&sum, &mtx, &cv, &queue] {
        std::unique_lock<std::mutex> lock(mtx);
        int consumed = 0;
        while (consumed < kTotal) {
            cv.wait(lock, [&queue] { return !queue.empty(); });
            while (!queue.empty() && consumed < kTotal) {
                const int v = queue.front();
                queue.pop();
                lock.unlock();
                sum += v;
                ++consumed;
                lock.lock();
            }
        }
    });

    std::thread producer([&mtx, &cv, &queue] {
        for (int i = 1; i <= kTotal; ++i) {
            {
                std::scoped_lock lock(mtx);
                queue.push(i);
            }
            cv.notify_one();
        }
    });

    producer.join();
    consumer.join();

    const std::int64_t expected =
        static_cast<std::int64_t>(kTotal) * (kTotal + 1) / 2;
    EXPECT_EQ(sum, expected);
}
