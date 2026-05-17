// 模块 13：std::atomic_thread_fence —— 与原子操作配合建立 happens-before。
//
// fence(memory_order_release) 可与另一个线程上的 fence(acquire) + relaxed load/store
// 组合成同步关系（细节依赖配对模式）。此处演示 release fence + acquire fence 配对。

#include <atomic>
#include <iostream>
#include <thread>

namespace {

// 演示用全局原子：线程间可见的同步演示状态。
std::atomic<bool> x{false};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<bool> y{false};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<int> z{0};       // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void thread1() {
    x.store(true, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
    y.store(true, std::memory_order_relaxed);
}

void thread2() {
    while (!y.load(std::memory_order_relaxed)) {
        std::this_thread::yield();
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    if (x.load(std::memory_order_relaxed)) {
        z.fetch_add(1, std::memory_order_relaxed);
    }
}

}  // namespace

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    t1.join();
    t2.join();
    std::cout << "fence 配对后 z = " << z.load() << "（若同步成立应为 1）\n";
    return 0;
}
