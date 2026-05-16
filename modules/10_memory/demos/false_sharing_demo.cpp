// 伪共享：两颗逻辑核写入落在同一缓存行上的不同原子，缓存行在核间反复失效。
//
// `std::hardware_destructive_interference_size` 给出「独立热点应相隔」的距离；
// `std::hardware_constructive_interference_size` 表示可以放在一起的友好粒度。
//
// Apple Clang 等环境可能不提供标准常量，外层以 `__cpp_lib_hardware_interference_size`
// 做条件编译，回退时用常见的 64 字节假定仅作教学对比。

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <thread>

#ifdef __cpp_lib_hardware_interference_size
#include <new>
#endif

namespace {

constexpr int kIterations = 800'000;

#ifdef __cpp_lib_hardware_interference_size
constexpr std::size_t kLine = std::hardware_destructive_interference_size;
#else
constexpr std::size_t kLine = 64;
#endif

struct PackedCounters {
    std::atomic<std::uint64_t> lhs;
    std::atomic<std::uint64_t> rhs;
};

struct LineSplitCounters {
    alignas(kLine) std::atomic<std::uint64_t> lhs;
    alignas(kLine) std::atomic<std::uint64_t> rhs;
};

template <typename Layout>
double runTwoAccumulators(Layout& layout) {
    using clock = std::chrono::steady_clock;
    auto const t0 = clock::now();

    std::thread left([&]() {
        for (int i = 0; i < kIterations; ++i) {
            layout.lhs.fetch_add(1ULL, std::memory_order_relaxed);
        }
    });
    std::thread right([&]() {
        for (int i = 0; i < kIterations; ++i) {
            layout.rhs.fetch_add(1ULL, std::memory_order_relaxed);
        }
    });
    left.join();
    right.join();

    return std::chrono::duration<double>{clock::now() - t0}.count();
}

}  // namespace

int main() {
#ifdef __cpp_lib_hardware_interference_size
    std::cout << "hardware_destructive_interference_size : "
              << std::hardware_destructive_interference_size << '\n';
    std::cout << "hardware_constructive_interference_size: "
              << std::hardware_constructive_interference_size << '\n';
#else
    std::cout << "__cpp_lib_hardware_interference_size 不可用；演示使用假定缓存行宽度 " << kLine
              << " 字节。\n";
#endif

    PackedCounters packed{};
    LineSplitCounters split{};
    double const packed_sec = runTwoAccumulators(packed);
    double const split_sec = runTwoAccumulators(split);

    std::cout << "紧凑布局 " << sizeof(PackedCounters) << " 字节耗时: " << packed_sec << " s\n";
    std::cout << "`alignas(缓存线)` 拆分后 " << sizeof(LineSplitCounters) << " 字节耗时: "
              << split_sec << " s\n";
    std::cout << "两线程递增各 " << kIterations << " 次；结果 lhs/rhs == " << kIterations << "。"
              << " 具体快慢随 CPU/OS 而异，关键是对比「同行缓存线」vs「刻意隔离」。\n";
    std::cout << "packed lhs=" << packed.lhs.load() << " rhs=" << packed.rhs.load() << '\n';
    std::cout << "split  lhs=" << split.lhs.load() << " rhs=" << split.rhs.load() << '\n';

    return 0;
}
