// 模块 13：std::atomic 常见 API：load / store / exchange / compare_exchange / fetch_add。

#include <atomic>
#include <iostream>

namespace {

void demoLoadStore() {
    std::atomic<int> a{7};
    std::cout << "load: " << a.load() << '\n';
    a.store(42);
    std::cout << "after store(42): " << a.load() << '\n';
}

void demoExchange() {
    std::atomic<int> a{1};
    const int old = a.exchange(99);
    std::cout << "exchange: 旧值 " << old << " 当前 " << a.load() << '\n';
}

void demoCompareExchange() {
    std::atomic<int> a{5};
    int expected = 5;
    const bool ok = a.compare_exchange_strong(expected, 10);
    std::cout << "compare_exchange_strong: " << (ok ? "成功" : "失败")
              << " expected(后)=" << expected << " 现值=" << a.load() << '\n';

    expected = 999;  // 与当前值不符
    const bool fail = a.compare_exchange_weak(expected, 0);
    std::cout << "compare_exchange_weak(应失败): " << (fail ? "成功?" : "失败")
              << " expected(后)=" << expected << '\n';
}

void demoFetchAdd() {
    std::atomic<int> a{100};
    const int prev = a.fetch_add(7);
    std::cout << "fetch_add(7)：返回旧值 " << prev << " 新值 " << a.load() << '\n';
}

}  // namespace

int main() {
    demoLoadStore();
    demoExchange();
    demoCompareExchange();
    demoFetchAdd();
    return 0;
}
