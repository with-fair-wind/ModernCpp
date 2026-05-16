// 模块 12 演示：std::counting_semaphore / std::binary_semaphore（计数信号量）。

#include <chrono>
#include <iostream>
#include <semaphore>
#include <thread>

namespace {

constexpr int kTickets = 3;

}  // namespace

int main() {
    // counting_semaphore：允许多次 acquire，直到计数耗尽。
    std::counting_semaphore tickets{kTickets};
    std::cout << "初始计数=" << kTickets << "：连续 try_acquire\n";
    for (int i = 0; i < kTickets; ++i) {
        std::cout << "  try_acquire[" << i << "]=" << std::boolalpha << tickets.try_acquire()
                  << '\n';
    }
    std::cout << "耗尽后再 try_acquire=" << std::boolalpha << tickets.try_acquire() << '\n';

    tickets.release();
    std::cout << "release 一次后再 try_acquire=" << std::boolalpha << tickets.try_acquire() << '\n';

    // binary_semaphore：等价于 counting_semaphore<1> —— 互斥风格的二元许可。
    std::binary_semaphore gate{1};
    std::cout << "\nbinary_semaphore 初始占有许可 try_acquire=" << std::boolalpha
              << gate.try_acquire() << '\n';
    std::cout << "第二次 try_acquire=" << std::boolalpha << gate.try_acquire() << '\n';

    std::thread worker([&gate] {
        std::cout << "worker：阻塞 acquire...\n";
        gate.acquire();
        std::cout << "worker：拿到许可\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        gate.release();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << "main：release 唤醒 worker\n";
    gate.release();
    worker.join();

    return 0;
}
