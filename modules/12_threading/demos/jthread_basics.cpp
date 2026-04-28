// Smoke demo for module 12: std::jthread + std::stop_token cooperative cancel.
//
// jthread joins on destruction (RAII) and threads its stop_token through to
// the worker, so the main thread can ask the worker to wind down cleanly
// without raw flags or detached threads.

#include <chrono>
#include <iostream>
#include <stop_token>
#include <thread>

int main() {
    // NOLINTNEXTLINE(performance-unnecessary-value-param) - canonical jthread signature
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
    // jthread dtor joins automatically.
    return 0;
}
