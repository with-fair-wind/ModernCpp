// 模块 12 演示：std::shared_mutex —— shared_lock 并发读，unique_lock 独占写。

#include <chrono>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::shared_mutex rw_mutex;
int shared_data = 0;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

void reader(int id) {
    std::shared_lock<std::shared_mutex> lk(rw_mutex);
    std::cout << "reader " << id << " 看到 shared_data=" << shared_data << '\n';
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void writer(int value) {
    std::unique_lock<std::shared_mutex> lk(rw_mutex);
    shared_data = value;
    std::cout << "writer 写入 shared_data=" << value << '\n';
}

} // namespace

int main() {
    std::thread r1(reader, 1);
    std::thread r2(reader, 2);
    std::thread w1([] { writer(100); });
    r1.join();
    r2.join();
    w1.join();

    std::thread r3(reader, 3);
    r3.join();

    return 0;
}
