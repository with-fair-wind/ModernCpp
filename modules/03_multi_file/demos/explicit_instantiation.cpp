// 模板显式实例化演示。
// 模板声明在 tmpl.h，定义在 tmpl_impl.cpp（非常规做法）。
// tmpl_impl.cpp 中显式实例化了 int、double、std::string 三种类型。
//
// 关键点：
//   - 模板通常需要定义在头文件（让使用者的 TU 自行实例化）
//   - 显式实例化允许把定义放源文件，但限制了可用类型
//   - 适用于：减少编译时间 / 控制 ABI / 限制用户只用特定类型

#include <iostream>
#include <string>

#include "tmpl.h"

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== 模板显式实例化演示 =====\n\n";

    // 函数模板 —— 已被显式实例化的类型
    std::cout << "-- 函数模板 tmplAdd --\n";
    std::cout << "tmplAdd(3, 4) = " << tmplAdd(3, 4) << '\n';
    std::cout << "tmplAdd(1.5, 2.5) = " << tmplAdd(1.5, 2.5) << '\n';
    std::cout << R"(tmplAdd("hello", " world") = )"
              << tmplAdd(std::string{"hello"}, std::string{" world"}) << '\n';
    std::cout << '\n';

    // 函数模板 —— tmplToString
    std::cout << "-- 函数模板 tmplToString --\n";
    std::cout << R"(tmplToString(42) = ")" << tmplToString(42) << "\"\n";
    std::cout << R"(tmplToString(3.14) = ")" << tmplToString(3.14) << "\"\n";
    std::cout << '\n';

    // 类模板 Stack
    std::cout << "-- 类模板 Stack<int> --\n";
    Stack<int> int_stack;
    int_stack.push(10);
    int_stack.push(20);
    int_stack.push(30);
    std::cout << "push 10, 20, 30; size = " << int_stack.size() << '\n';
    std::cout << "pop = " << int_stack.pop() << '\n';
    std::cout << "pop = " << int_stack.pop() << '\n';
    std::cout << "empty? " << (int_stack.empty() ? "是" : "否") << '\n';
    std::cout << '\n';

    // 类模板 Stack<std::string>
    std::cout << "-- 类模板 Stack<std::string> --\n";
    Stack<std::string> str_stack;
    str_stack.push("alpha");
    str_stack.push("beta");
    std::cout << R"(push "alpha", "beta"; size = )" << str_stack.size() << '\n';
    std::cout << R"(pop = ")" << str_stack.pop() << "\"\n";

    return 0;
}
