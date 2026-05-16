// C++20 std::chrono 日历类型：year_month_day、月末日、weekday、日历算术。
//
// 由实现决定是否完整支持日历设施；此处用特性宏裁剪。

#include <chrono>
#include <iostream>

int main() {  // NOLINT(bugprone-exception-escape)
#if defined(__cpp_lib_chrono) && (__cpp_lib_chrono >= 201907L)
    using namespace std::chrono;

    auto const ymd = 2024y / March / 15d;
    std::cout << "year_month_day: " << static_cast<int>(ymd.year()) << '-'
              << static_cast<unsigned>(ymd.month()) << '-'
              << static_cast<unsigned>(ymd.day()) << '\n';
    std::cout << "ok(): " << (ymd.ok() ? "true" : "false") << '\n';

    auto const last_feb = 2024y / February / last;
    std::cout << "2024 年 2 月末: " << last_feb.day() << '\n';

    weekday const wd{sys_days{2024y / February / 15d}};
    std::cout << "2024-02-15 星期（ISO 编码 1=Mon..7=Sun）: " << wd.iso_encoding()
              << '\n';

    auto const shifted = ymd + months{13};
    std::cout << "+13 months 后: " << static_cast<int>(shifted.year()) << '-'
              << static_cast<unsigned>(shifted.month()) << '-'
              << static_cast<unsigned>(shifted.day()) << '\n';
#else
    std::cout << "当前编译选项未启用 C++20 chrono 日历（__cpp_lib_chrono）。\n";
#endif
    return 0;
}
