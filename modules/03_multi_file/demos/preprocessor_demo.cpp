// 预处理器综合演示：宏定义、条件编译、__VA_ARGS__、__VA_OPT__、
// 字符串化 (#)、记号拼接 (##)、__has_include、feature test macro、
// do-while 安全宏模式。
//
// 本文件的核心目的是演示预处理器特性，因此大量使用宏是刻意为之。

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum,modernize-macro-to-enum)

#include <iostream>

// ========== 1. 对象宏与函数宏 ==========

#define PI 3.14159265358979
#define MAX_BUFFER_SIZE 1024

// 函数宏 —— 注意括号保护，避免优先级陷阱
#define SQUARE(x) ((x) * (x))

// 不加括号的反面教材（仅展示，实际不应这样写）
#define BAD_SQUARE(x) x* x

// ========== 2. 字符串化与记号拼接 ==========

// # 把参数转成字符串字面量
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

// ## 把参数拼接为一个记号
#define CONCAT(a, b) a##b
#define MAKE_VAR(prefix, id) prefix##_##id

// ========== 3. __VA_ARGS__ 与 C++20 __VA_OPT__ ==========

// 传统可变参数宏
#define LOG_MSG(fmt, ...) \
    std::cout << "[LOG] " << fmt << " " << __VA_ARGS__ << '\n'

// C++20 __VA_OPT__：当可变参数为空时省略逗号/内容
#define PRINT_LABELED(label, ...) \
    std::cout << label __VA_OPT__(<< ": " << __VA_ARGS__) << '\n'

// ========== 4. do-while 安全多语句宏 ==========

#define SAFE_SWAP(a, b)   \
    do {                  \
        auto tmp_ = (a);  \
        (a) = (b);        \
        (b) = tmp_;       \
    } while (0)

// ========== 5. 条件编译 ==========

#define ENABLE_VERBOSE 1

// ========== 6. __has_include 检测 ==========

#if __has_include(<optional>)
#define HAS_OPTIONAL 1
#include <optional>
#else
#define HAS_OPTIONAL 0
#endif

#if __has_include(<format>)
#define HAS_FORMAT 1
#else
#define HAS_FORMAT 0
#endif

// ========== 7. Feature test macro ==========

#include <version>

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum,modernize-macro-to-enum)

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== 预处理器演示 =====\n\n";

    // 1. 对象宏与函数宏
    std::cout << "-- 对象宏 & 函数宏 --\n";
    std::cout << "PI = " << PI << '\n';                        // NOLINT(modernize-use-std-numbers)
    std::cout << "MAX_BUFFER_SIZE = " << MAX_BUFFER_SIZE << '\n';
    std::cout << "SQUARE(5) = " << SQUARE(5) << '\n';
    std::cout << "SQUARE(2+3) = " << SQUARE(2 + 3) << " (正确: 25)\n";
    // NOLINTNEXTLINE(readability-math-missing-parentheses)
    std::cout << "BAD_SQUARE(2+3) = " << BAD_SQUARE(2 + 3)
              << " (错误: 2+3*2+3 = 11)\n\n";

    // 2. 字符串化与记号拼接
    std::cout << "-- 字符串化 & 拼接 --\n";
    std::cout << "STRINGIFY(hello) = " << STRINGIFY(hello) << '\n';
    std::cout << "TO_STRING(MAX_BUFFER_SIZE) = " << TO_STRING(MAX_BUFFER_SIZE)
              << '\n';

    int CONCAT(my, Var) = 100;  // 展开为 int myVar = 100;
    std::cout << "CONCAT(my, Var) => myVar = " << myVar << '\n';

    int MAKE_VAR(count, total) = 999;  // 展开为 int count_total = 999;
    std::cout << "MAKE_VAR(count, total) => count_total = " << count_total
              << "\n\n";

    // 3. __VA_ARGS__ 与 __VA_OPT__
    std::cout << "-- 可变参数宏 --\n";
    LOG_MSG("value:", 42);

    PRINT_LABELED("带参数", "some data");
    PRINT_LABELED("无参数");
    std::cout << '\n';

    // 4. do-while 安全宏
    std::cout << "-- do-while 安全宏 --\n";
    int x = 10;
    int y = 20;
    std::cout << "交换前: x=" << x << " y=" << y << '\n';
    SAFE_SWAP(x, y);  // NOLINT(cppcoreguidelines-avoid-do-while)
    std::cout << "交换后: x=" << x << " y=" << y << "\n\n";

    // 5. 条件编译
    std::cout << "-- 条件编译 --\n";
#if ENABLE_VERBOSE
    std::cout << "VERBOSE 模式已启用\n";
#else
    std::cout << "VERBOSE 模式已禁用\n";
#endif

#ifdef __cpp_consteval
    std::cout << "__cpp_consteval 已定义 (值: " << __cpp_consteval << ")\n";
#else
    std::cout << "__cpp_consteval 未定义\n";
#endif
    std::cout << '\n';

    // 6. __has_include 检测
    std::cout << "-- __has_include 检测 --\n";
    std::cout << "<optional> 可用: " << (HAS_OPTIONAL ? "是" : "否") << '\n';
    std::cout << "<format> 可用: " << (HAS_FORMAT ? "是" : "否") << '\n';

#if HAS_OPTIONAL
    std::optional<int> opt = 42;
    std::cout << "std::optional<int> 值: " << opt.value() << '\n';
#endif
    std::cout << '\n';

    // 7. Feature test macro
    std::cout << "-- Feature test macro --\n";
    std::cout << "__cplusplus = " << __cplusplus << '\n';

#ifdef __cpp_lib_optional
    std::cout << "__cpp_lib_optional = " << __cpp_lib_optional << '\n';
#endif

#ifdef __cpp_lib_ranges
    std::cout << "__cpp_lib_ranges = " << __cpp_lib_ranges << '\n';
#endif

#ifdef __cpp_lib_expected
    std::cout << "__cpp_lib_expected = " << __cpp_lib_expected << '\n';
#else
    std::cout << "__cpp_lib_expected 未定义\n";
#endif

    // 8. #undef —— 取消宏定义
    std::cout << "-- #undef --\n";
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define TEMP_MACRO 42
    std::cout << "TEMP_MACRO = " << TEMP_MACRO << '\n';
#undef TEMP_MACRO
// NOLINTEND(cppcoreguidelines-macro-usage)
    // TEMP_MACRO 此后不再可用（若取消注释下行会编译失败）
    // std::cout << TEMP_MACRO;
    std::cout << "TEMP_MACRO 已被 #undef\n\n";

    // 9. #pragma once vs 传统 header guard
    // #pragma once —— 编译器自动阻止同一文件被多次包含（本项目所有头文件均使用）
    // 传统 header guard 写法：
    //   #ifndef MY_HEADER_H
    //   #define MY_HEADER_H
    //   ...内容...
    //   #endif
    // 二者效果相同；#pragma once 更简洁但非标准（几乎所有主流编译器均支持）
    std::cout << "-- #pragma once vs header guard --\n";
    std::cout << "本项目所有头文件使用 #pragma once（详见源码注释）\n\n";

    // 10. #error / #warning —— 编译期诊断指令
    // #error "message"  —— 直接终止编译并输出错误信息
    // #warning "message" —— 产生编译器警告（C++23 正式标准化，此前为扩展）
    // 用法示例（不能实际执行，否则编译失败）：
    //   #if !defined(TARGET_OS)
    //   #error "必须定义 TARGET_OS 宏"
    //   #endif
    //   #warning "此功能尚未完成"
    std::cout << "-- #error / #warning --\n";
    std::cout << "#error 会终止编译，#warning 产生警告（详见源码注释）\n";

    return 0;
}
