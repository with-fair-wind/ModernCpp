// 模块 13：acquire-release 成对同步演示（生产者 / 消费者）。
//
// release 写把「此前对数据非原子内存的写入」释放给 acquire 读；消费者在
// acquire 看到标志为真之后，可以安全读取共享数据。

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

namespace {

// 生产者/消费者演示用共享数据。
std::string shared_payload;      // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<bool> ready{false};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void producer() {
    shared_payload = "payload-from-producer";
    // release：在此之前对 shared_payload 的写入对消费者 acquire 可见
    ready.store(true, std::memory_order_release);
    std::cout << "生产者：数据写好并发 release ready\n";
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    std::cout << "消费者：acquire 看到 ready，读到 \"" << shared_payload << "\"\n";
}

}  // namespace

int main() {
    std::thread prod(producer);
    std::thread cons(consumer);
    prod.join();
    cons.join();
    return 0;
}
