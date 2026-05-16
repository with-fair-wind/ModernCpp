// 模块 13：memory_order_relaxed 演示——仅保证原子性，不参与 synchronizes-with。
//
// 多线程对一个原子计数器做 relaxed 递增，总和仍正确（无数据竞争）；但 relaxed
// 不与「保护其他非原子数据」混用——本 demo 只统计计数，不当作发布/获取屏障。

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

namespace {

constexpr int kThreads = 4;
constexpr int kPerThread = 100'000;

void worker(std::atomic<int>& counter) {
    for (int i = 0; i < kPerThread; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

} // namespace

int main() {
    std::atomic<int> counter{0};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back(worker, std::ref(counter));
    }
    for (auto& t : threads) {
        t.join();
    }
    const int expected = kThreads * kPerThread;
    std::cout << "relaxed fetch_add 合计：" << counter.load(std::memory_order_relaxed)
              << "（期望 " << expected << "）\n";
    return 0;
}
