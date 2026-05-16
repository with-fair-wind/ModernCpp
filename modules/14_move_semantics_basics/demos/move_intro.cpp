// 用大容量 vector 粗略对比「拷贝构造」与「移动构造」的耗时差异。
//
// 要点：移动通常只是窃取指针与尺寸元数据；拷贝要为每个元素调用拷贝语义，
// 对大块堆内存场景差距会非常夸张。

#include <chrono>
#include <iostream>
#include <utility>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

template <typename F>
double measureSeconds(F&& fn) {
    auto const t0 = Clock::now();
    std::forward<F>(fn)();
    auto const t1 = Clock::now();
    return std::chrono::duration<double>{t1 - t0}.count();
}

}  // namespace

int main() {
    constexpr std::size_t kSize = 5'000'000;

    std::vector<int> heavy(kSize, 42);

    double const copy_secs = measureSeconds([&] {
        // 故意整块拷贝以便计时对照；拷贝体未再用，仅度量开销。
        [[maybe_unused]] auto duplicate = heavy;  // NOLINT(performance-unnecessary-copy-initialization)
    });

    double const move_secs = measureSeconds([&] { [[maybe_unused]] auto relocated = std::move(heavy); });

    std::cout << "元素个数: " << kSize << '\n'
              << "拷贝构造耗时约: " << copy_secs << " s\n"
              << "移动构造耗时约: " << move_secs << " s\n";

    return 0;
}
