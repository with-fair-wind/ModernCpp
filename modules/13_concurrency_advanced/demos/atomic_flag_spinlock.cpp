// 模块 13：用 std::atomic_flag 实现轻量自旋锁（spinlock）。
//
// test_and_set(memory_order_acquire) 获取锁；clear(memory_order_release) 释放锁，
// 与 mutex 相比更适合极短临界区（否则浪费 CPU）。

#include <atomic>
#include <iostream>
#include <thread>

namespace {

class Spinlock {
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_{ATOMIC_FLAG_INIT};
};

// 演示用：多线程共享自旋锁与计数器。
std::atomic<int> guarded_counter{0};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
Spinlock g_spin;                      // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void incrementMany(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        g_spin.lock();
        guarded_counter.fetch_add(1, std::memory_order_relaxed);
        g_spin.unlock();
    }
}

}  // namespace

int main() {
    constexpr int kPerThread = 50'000;
    std::thread t1(incrementMany, kPerThread);
    std::thread t2(incrementMany, kPerThread);
    t1.join();
    t2.join();
    std::cout << "Spinlock 保护下的计数：" << guarded_counter.load() << "（期望 "
              << (2 * kPerThread) << "）\n";
    return 0;
}
