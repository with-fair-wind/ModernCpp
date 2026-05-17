// 模块 12 演示：std::latch（一次性栅栏）与 std::barrier（可复用栅栏）。

#include <barrier>
#include <chrono>
#include <iostream>
#include <latch>
#include <thread>

namespace {

constexpr int kParties = 3;

}  // namespace

int main() {
    std::cout << "--- std::latch：主线程等待 " << kParties << " 个子线程就绪 ---\n";
    std::latch arrive{kParties};
    for (int i = 0; i < kParties; ++i) {
        std::thread([i, &arrive] {
            std::cout << "worker " << i << " 到达 latch 前\n";
            arrive.arrive_and_wait();
            std::cout << "worker " << i << " 通过 latch\n";
        }).detach();
    }
    arrive.wait();
    std::cout << "全部到达，继续主流程\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::cout << "\n--- std::barrier：两轮同步 ---\n";
    int round = 0;
    // main + 两条工作线程共三方，每一轮都必须 arrive_and_wait 三次。
    std::barrier sync_point(kParties, [&round] noexcept {
        ++round;
        std::cout << "[完成阶段回调] round=" << round << '\n';
    });

    std::thread a([&sync_point] {
        std::cout << "A 进入 barrier 第一轮\n";
        sync_point.arrive_and_wait();
        std::cout << "A 通过第一轮\n";
        sync_point.arrive_and_wait();
        std::cout << "A 通过第二轮\n";
    });
    std::thread b([&sync_point] {
        std::cout << "B 进入 barrier 第一轮\n";
        sync_point.arrive_and_wait();
        std::cout << "B 通过第一轮\n";
        sync_point.arrive_and_wait();
        std::cout << "B 通过第二轮\n";
    });

    std::cout << "main 进入 barrier 第一轮\n";
    sync_point.arrive_and_wait();
    std::cout << "main 通过第一轮\n";
    sync_point.arrive_and_wait();
    std::cout << "main 通过第二轮\n";

    a.join();
    b.join();

    return 0;
}
