// ODR（单一定义规则）基础演示。
// 展示头文件声明 + 源文件定义的标准多文件组织方式。
//
// 关键点：
//   - 函数在整个程序中只能定义一次（除非 inline）
//   - 类定义在每个 TU 中只能出现一次（头文件保护保证）
//   - 静态数据成员的定义放在源文件中

#include <iostream>

#include "odr_helper.h"  // 只包含声明，不会引入重复定义

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== ODR 基础演示 =====\n\n";

    // 调用在另一个 TU 中定义的自由函数
    std::cout << "add(3, 4) = " << add(3, 4) << '\n';
    std::cout << "greet(\"World\") = " << greet("World") << '\n';
    std::cout << '\n';

    // 使用在头文件中定义的类（成员函数定义在 odr_helper.cpp）
    Counter c1{10};
    c1.increment();
    c1.increment();
    std::cout << "Counter c1 初始值 10, increment 2 次: " << c1.value() << '\n';

    Counter c2{0};
    c2.decrement();
    std::cout << "Counter c2 初始值 0, decrement 1 次: " << c2.value() << '\n';

    // 默认参数演示 —— 默认参数写在声明（头文件）中，定义不重复
    std::cout << '\n';
    std::cout << "-- 默认参数 --\n";
    c1.incrementBy();  // 使用默认参数 step=1
    std::cout << "c1.incrementBy() 后: " << c1.value() << '\n';
    c1.incrementBy(5);  // 显式传参
    std::cout << "c1.incrementBy(5) 后: " << c1.value() << '\n';

    // 静态数据成员跨所有实例共享
    std::cout << '\n';
    std::cout << "Counter::instance_count = " << Counter::instance_count << " (应为 2)\n";

    return 0;
}
