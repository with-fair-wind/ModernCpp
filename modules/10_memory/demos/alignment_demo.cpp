// alignas 控制对象对齐；演示“假共享”（false sharing）场景下用缓存行对齐隔离热点。

#include <atomic>
#include <cstddef>
#include <iostream>
#include <thread>

namespace {

// 常见 L1 缓存行长度；用 64 字节对齐把两个原子放到不同行，减少争用同一行的写回。
struct alignas(64) CacheLineAtomic {
    std::atomic<int> counter;
};

void spinIncrement(std::atomic<int>& a, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        a.fetch_add(1, std::memory_order_relaxed);
    }
}

void demoAlignQuery() {
    alignas(32) double values[4]{};
    std::cout << "alignof(double[4]) with alignas(32): " << alignof(decltype(values)) << '\n';
    std::cout << "地址 mod 32: "
              << (reinterpret_cast<std::uintptr_t>(values) %
                  32U)  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
              << '\n';
}

void demoFalseSharingMitigation() {
    // 两个计数器分别独占一条缓存行，避免两个线程反复修改同一行引发性能抖动。
    CacheLineAtomic a{};
    CacheLineAtomic b{};

    constexpr int kIters = 200'000;
    std::thread t1([&] { spinIncrement(a.counter, kIters); });
    std::thread t2([&] { spinIncrement(b.counter, kIters); });
    t1.join();
    t2.join();

    std::cout << "a=" << a.counter.load() << " b=" << b.counter.load() << '\n';
}

}  // namespace

int main() {
    demoAlignQuery();
    demoFalseSharingMitigation();
    return 0;
}
