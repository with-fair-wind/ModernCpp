// 模块 12 演示：std::thread 的基本用法 —— 多种构造方式、join/detach、线程标识。

#include <chrono>
#include <iostream>
#include <thread>

namespace {

void threadFunc(int value) {
    std::cout << "threadFunc: value=" << value << ", id=" << std::this_thread::get_id()
              << '\n';
}

struct Worker {
    static void runMember(int offset) {
        std::cout << "Worker::runMember: offset=" << offset << ", id=" << std::this_thread::get_id()
                  << '\n';
    }
};

} // namespace

int main() {
    std::cout << "main thread id: " << std::this_thread::get_id() << '\n';

    // 1) 绑定自由函数
    std::thread t1(threadFunc, 42);
    std::cout << "t1 joinable before join: " << std::boolalpha << t1.joinable() << '\n';
    t1.join();
    std::cout << "t1 joinable after join: " << std::boolalpha << t1.joinable() << '\n';

    // 2) lambda
    std::thread t2([] {
        std::cout << "lambda thread id: " << std::this_thread::get_id() << '\n';
    });
    if (t2.joinable()) {
        t2.join();
    }

    // 3) 静态成员函数指针（无 this，用法与自由函数类似）
    std::thread t3(&Worker::runMember, 7);
    t3.join();

    // 4) detach：线程与 std::thread 句柄分离，在后台继续运行直到函数返回
    std::thread t4([] {
        std::cout << "detached thread runs briefly...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        std::cout << "detached thread exiting\n";
    });
    t4.detach();
    std::cout << "t4 joinable after detach: " << std::boolalpha << t4.joinable() << '\n';

    // 给 detached 线程一点时间打印（演示用途；测试代码不应依赖 sleep）
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::cout << "hardware_concurrency: " << std::thread::hardware_concurrency() << '\n';
    return 0;
}
