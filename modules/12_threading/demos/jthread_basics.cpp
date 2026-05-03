// 模块 12 的小演示：std::jthread + std::stop_token 协作式取消。
//
// jthread 在析构时自动 join（RAII），并把 stop_token 透传给工作线程；
// 这样主线程就可以请求工作线程优雅退出，而无需自行管理裸标志或
// 使用 detached 线程。

#include <chrono>
#include <iostream>
#include <stop_token>
#include <thread>

int main() {
    // NOLINTNEXTLINE(performance-unnecessary-value-param) - jthread 的标准签名要求按值传递
    std::jthread worker([](std::stop_token st) {
        int ticks = 0;
        while (!st.stop_requested()) {
            ++ticks;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "worker stopped after " << ticks << " ticks\n";
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    worker.request_stop();
    // jthread 析构时会自动 join。
    return 0;
}
