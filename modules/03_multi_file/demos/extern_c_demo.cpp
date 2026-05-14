// extern "C" 语言链接演示。
// 展示如何将 C++ 实现封装为 C ABI 接口，使其可被 C 链接器/其他语言调用。
//
// 关键点：
//   - extern "C" 禁止名字改编（name mangling），符号名即函数名
//   - 通过 __cplusplus 保护宏让同一头文件兼容 C 和 C++ 编译器
//   - 内部实现可以自由使用 C++ 特性，只要接口保持 C 兼容

#include <iostream>

#include "c_api.h"

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== extern \"C\" 演示 =====\n\n";

    // 算术接口
    std::cout << "-- 算术接口 --\n";
    std::cout << "c_api_add(10, 3) = " << c_api_add(10, 3) << '\n';
    std::cout << "c_api_subtract(10, 3) = " << c_api_subtract(10, 3) << '\n';
    std::cout << "c_api_multiply(10, 3) = " << c_api_multiply(10, 3) << '\n';
    std::cout << '\n';

    // 字符串格式化（内部用 C++ std::string 实现）
    std::cout << "-- 字符串格式化 --\n";
    std::cout << "c_api_format_int(42) = " << c_api_format_int(42) << '\n';
    std::cout << "c_api_format_int(-7) = " << c_api_format_int(-7) << '\n';
    std::cout << '\n';

    // 带状态的累加器
    std::cout << "-- 累加器 --\n";
    c_api_accumulator_reset();
    c_api_accumulator_add(10);
    c_api_accumulator_add(20);
    c_api_accumulator_add(30);
    std::cout << "累加 10+20+30 = " << c_api_accumulator_get() << '\n';

    c_api_accumulator_reset();
    std::cout << "重置后 = " << c_api_accumulator_get() << '\n';

    return 0;
}
