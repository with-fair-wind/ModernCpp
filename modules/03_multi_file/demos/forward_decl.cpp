// 前置声明演示：两个类互相持有对方的指针。
// 若直接 #include 对方头文件会造成循环包含；
// 用前置声明（class NodeB;）即可在不包含完整定义的情况下声明指针成员。
//
// 关键点：
//   - 不完整类型只能用于指针、引用、函数参数声明
//   - 需要访问成员时，才在 .cpp 中 #include 完整定义
//   - 减少头文件依赖可显著降低重编译范围

#include <iostream>

#include "forward_a.h"
#include "forward_b.h"

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== 前置声明演示 =====\n\n";

    NodeA a{"Alice"};
    NodeB b{"Bob"};

    std::cout << "初始状态:\n";
    std::cout << "  " << a.describe() << '\n';
    std::cout << "  " << b.describe() << '\n';
    std::cout << '\n';

    // 互相设置伙伴指针
    a.setPartner(&b);
    b.setPartner(&a);

    std::cout << "设置 partner 后:\n";
    std::cout << "  " << a.describe() << '\n';
    std::cout << "  " << b.describe() << '\n';

    return 0;
}
