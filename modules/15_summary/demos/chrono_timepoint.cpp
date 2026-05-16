// std::chrono::time_point 与各时钟：system_clock / steady_clock /
// high_resolution_clock 以及 now()、耗时测量。

#include <chrono>
#include <ctime>
#include <format>
#include <iostream>
#include <thread>

namespace {

template <typename Clock>
void dumpClockHeadline() {
    std::cout << std::format("- {}\n", typeid(Clock).name());
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    using namespace std::chrono_literals;

    dumpClockHeadline<std::chrono::system_clock>();
    auto const sys_now = std::chrono::system_clock::now();
    std::time_t const tt = std::chrono::system_clock::to_time_t(sys_now);
    // std::ctime 依赖静态缓冲区，非线程安全；本 demo 仅在单线程中演示。
    std::cout << "  system_clock::now -> to_time_t (local ctime):\n    "
              << std::ctime(&tt);  // NOLINT(concurrency-mt-unsafe)
    std::cout << "  epoch 毫秒: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     sys_now.time_since_epoch())
                     .count()
              << '\n';

    dumpClockHeadline<std::chrono::steady_clock>();
    auto const t0 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(30ms);
    auto const t1 = std::chrono::steady_clock::now();
    std::cout << std::format(
        "  steady 间隔 {:>8} ns（单调，适合计时）\n",
        std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0)
            .count());

    dumpClockHeadline<std::chrono::high_resolution_clock>();
    auto const hires_now = std::chrono::high_resolution_clock::now();
    std::cout << std::format(
        "  high_resolution_clock tick 计数（epoch）: {}\n",
        hires_now.time_since_epoch().count());

    return 0;
}
