// 模块 12 演示：std::future / std::promise / std::packaged_task / std::async。

#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace {

int computeSum(int a, int b) {
    return a + b;
}

}  // namespace

int main() {
    // promise / future：手动交付结果或异常。
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    std::thread setter([&prom] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        prom.set_value(40 + 2);
    });
    std::cout << "promise/future get() = " << fut.get() << '\n';
    setter.join();

    // packaged_task：把可调用对象打包成异步结果通道。
    std::packaged_task<int(int, int)> task(computeSum);
    std::future<int> ft = task.get_future();
    std::thread runner([&task] { task(10, 20); });
    std::cout << "packaged_task get() = " << ft.get() << '\n';
    runner.join();

    // async：由实现选择在新线程或惰性求值；此处演示 launch::async。
    std::future<int> af = std::async(std::launch::async, computeSum, 5, 6);
    std::cout << "async get() = " << af.get() << '\n';

    std::promise<void> bad_prom;
    std::future<void> bad_fut = bad_prom.get_future();
    bad_prom.set_exception(std::make_exception_ptr(std::runtime_error("演示异常")));
    try {
        bad_fut.get();
    } catch (const std::runtime_error& ex) {
        std::cout << "捕获来自 promise 的异常: " << ex.what() << '\n';
    }

    return 0;
}
