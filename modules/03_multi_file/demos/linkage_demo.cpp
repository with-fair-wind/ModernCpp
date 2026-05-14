// 链接属性综合演示：外部链接、内部链接（static / 匿名命名空间）、
// const 变量的默认内部链接、extern const。
//
// 关键点：
//   - 函数和非 const 变量默认外部链接
//   - static 函数/变量具有内部链接（仅当前 TU 可见）
//   - 匿名命名空间中的实体具有内部链接（C++ 推荐方式）
//   - const 全局变量在 C++ 中默认内部链接
//   - 需要跨 TU 共享 const 变量时用 extern const

#include <iostream>

#include "linkage_api.h"

// 本 TU 自己的 static 函数 —— 与 linkage_internal.cpp 里同名但互不干扰
static int internalHelper(int x) {
    return x - 1;
}

// 本 TU 自己的 const 变量 —— 默认内部链接，不影响其他 TU
const int kLocalConst = 999;

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== 链接属性演示 =====\n\n";

    // 1. 外部链接函数
    std::cout << "-- 外部链接 --\n";
    std::cout << "publicAdd(10, 20) = " << publicAdd(10, 20) << '\n';
    std::cout << "publicMultiply(3, 7) = " << publicMultiply(3, 7) << '\n';
    std::cout << '\n';

    // 2. extern const 跨 TU 共享
    std::cout << "-- extern const --\n";
    std::cout << "kSharedConstant = " << kSharedConstant << " (定义在 linkage_internal.cpp)\n";
    std::cout << "kLocalConst = " << kLocalConst << " (本 TU 内部链接，不冲突)\n";
    std::cout << '\n';

    // 3. 内部链接函数 —— 本 TU 的 internalHelper 与另一个 TU 的同名函数不同
    std::cout << "-- 内部链接 --\n";
    std::cout << "本 TU internalHelper(5) = " << internalHelper(5) << " (x-1 = 4)\n";
    std::cout << "另 TU internalHelper(5) = " << callInternalHelper() << " (x*x+1 = 26)\n";
    std::cout << "另 TU anonHelper(7) = " << callAnonNsHelper() << " (x*3-2 = 19)\n";

    return 0;
}
