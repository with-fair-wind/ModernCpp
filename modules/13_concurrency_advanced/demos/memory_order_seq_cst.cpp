// 模块 13：顺序一致（memory_order_seq_cst）内存模型演示。
//
// 默认的原子操作使用 seq_cst；所有线程观测到的 seq_cst 读写必须符合全局一致的
// 单一总序（single total order）。下面用「交叉读写」的两个原子变量展示：任意时刻
// 读到的中间状态都与某一全局交错顺序相容。

#include <atomic>
#include <iostream>
#include <thread>

namespace {

// 演示 seq_cst：两线程交错读写的全局原子。
std::atomic<int> x{0};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<int> y{0};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void threadA() {
    x.store(1, std::memory_order_seq_cst);
    const int ry = y.load(std::memory_order_seq_cst);
    std::cout << "线程 A：写完 x 后读到 y = " << ry << '\n';
}

void threadB() {
    y.store(1, std::memory_order_seq_cst);
    const int rx = x.load(std::memory_order_seq_cst);
    std::cout << "线程 B：写完 y 后读到 x = " << rx << '\n';
}

}  // namespace

int main() {
    std::cout << "seq_cst：两个原子变量交叉读写（默认内存序即为 seq_cst）\n";
    std::thread ta(threadA);
    std::thread tb(threadB);
    ta.join();
    tb.join();
    std::cout << "结束：x=" << x.load() << " y=" << y.load()
              << "（四个 (rx,ry) 组合在 seq_cst 下皆可能出现；不会出现「违背总序」的幻读）\n";
    return 0;
}
