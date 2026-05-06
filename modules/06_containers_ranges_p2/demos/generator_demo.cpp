// std::generator (C++23)：用协程实现的惰性输入序列。
//
// 关键点：
//   - generator 是 view，且只是 input_range——只能从前向后遍历一次。
//   - 函数体里出现 co_yield / co_return 即变协程；返回类型 std::generator<T> 由编译器
//     自动管理"挂起 / 恢复"的状态机。
//   - .begin() 才会真正启动协程；解引用 *it 拿到刚 co_yield 的值，++it 推进协程到
//     下一次 yield 或 co_return。
//   - 一个 generator 只能被遍历一次：要再用一次，必须重新构造一个 generator。
//   - 与 ranges 完美组合：可以接 take / filter / transform 等 view，把"无限序列"裁剪
//     成有限的窗口。

#include <generator>
#include <iostream>
#include <ranges>

namespace stdv = std::views;

namespace {

// 1) 经典 Fibonacci：前 n 项；状态保存在协程帧里
std::generator<int> fib(int n) {
    int a = 0;
    int b = 1;
    for (int i = 0; i < n; ++i) {
        co_yield a;
        int next = a + b;
        a = b;
        b = next;
    }
}

// 2) 无限自增：通过 take 截断才不会真的运行无穷
std::generator<int> infiniteIota(int start) {
    while (true) {
        co_yield start++;
    }
}

// 3) 用协程做 BFS 风格的递归展开：把嵌套结构压平
std::generator<int> flattenPairs(int low, int high) {
    for (int x = low; x <= high; ++x) {
        co_yield x;        // 主轴
        co_yield x * 10;   // 次轴
    }
}

}  // namespace

int main() {
    // 1) Fibonacci 前 10 项
    std::cout << "fib(10)              : ";
    for (int x : fib(10)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 2) 无限 iota 配 take(5)
    std::cout << "infiniteIota|take(5) : ";
    for (int x : infiniteIota(100) | stdv::take(5)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 3) 与 filter + transform 组合：第一个能被 7 整除的平方数（取前 3 个）
    std::cout << "first 3 squares-of-7m: ";
    for (int x : infiniteIota(1) | stdv::filter([](int n) { return n % 7 == 0; })
                     | stdv::transform([](int n) { return n * n; }) | stdv::take(3)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 4) flattenPairs：每次循环 yield 两次
    std::cout << "flattenPairs(1,3)    : ";
    for (int x : flattenPairs(1, 3)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 5) generator 只能用一次：第二次遍历需要重新构造
    auto once = fib(5);
    int sum = 0;
    for (int x : once) {
        sum += x;
    }
    std::cout << "fib(5) sum (one-shot): " << sum << '\n';
    // for (int x : once) { ... }  // ✗ generator 已耗尽

    return 0;
}
