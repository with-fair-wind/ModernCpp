// 模块 12 演示：mutex、lock_guard、unique_lock、scoped_lock（多锁同时获取）。

#include <iostream>
#include <mutex>

namespace {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::mutex log_mutex;

void printLine(const char* text) {
    // NOLINTNEXTLINE(modernize-use-scoped-lock) — demo 专门展示 lock_guard
    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << text << '\n';
}

}  // namespace

int main() {
    std::mutex m;

    // lock_guard：构造加锁、析构解锁（作用域锁，不可手动 unlock 再 lock）
    {
        // NOLINTNEXTLINE(modernize-use-scoped-lock) — demo 专门展示 lock_guard
        std::lock_guard<std::mutex> guard(m);
        printLine("lock_guard 持有互斥量");
    }

    // unique_lock：可 defer、可 adopt、可临时 unlock/relock，适配条件变量等
    {
        std::unique_lock<std::mutex> ulock(m);
        printLine("unique_lock 默认已加锁");
        ulock.unlock();
        printLine("unique_lock 已 unlock，可再做别的事…");
        ulock.lock();
        printLine("unique_lock 再次 lock");
    }

    // scoped_lock（C++17）：可变参、可一次获取多个锁，避免死锁（约定顺序）
    std::mutex m1;
    std::mutex m2;
    {
        std::scoped_lock locks(m1, m2);
        printLine("scoped_lock 同时持有 m1 与 m2");
    }

    printLine("演示结束");
    return 0;
}
