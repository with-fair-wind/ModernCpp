// 模块 12 演示：std::condition_variable + std::mutex 实现生产者—消费者。

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::mutex queue_mutex;
std::condition_variable cv;
// std::queue 默认构造：部分检查器认为可能抛异常（静态初始化风险），此处为教学用共享状态。
std::queue<int> task_queue;  // NOLINT(bugprone-throwing-static-initialization)
bool done = false;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

void producer() {
    for (int i = 1; i <= 5; ++i) {
        {
            std::scoped_lock lock(queue_mutex);
            task_queue.push(i);
            std::cout << "producer 放入 " << i << '\n';
        }
        cv.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    {
        std::scoped_lock lock(queue_mutex);
        done = true;
    }
    cv.notify_one();
}

void consumer() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    while (!done || !task_queue.empty()) {
        cv.wait(lock, [] { return done || !task_queue.empty(); });
        while (!task_queue.empty()) {
            const int v = task_queue.front();
            task_queue.pop();
            lock.unlock();
            std::cout << "consumer 取出 " << v << '\n';
            lock.lock();
        }
    }
}

}  // namespace

int main() {
    std::thread p(producer);
    std::thread c(consumer);
    p.join();
    c.join();
    return 0;
}
