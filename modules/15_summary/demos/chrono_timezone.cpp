// C++20 std::chrono 时区：zoned_time、locate_zone、current_zone 等。
//
// 依赖实现附带 IANA tzdb；不可用时应优雅降级提示。

#include <chrono>
#include <exception>
#include <iostream>

int main() {  // NOLINT(bugprone-exception-escape)
#if defined(__cpp_lib_chrono) && (__cpp_lib_chrono >= 201907L)
#if defined(__cpp_lib_tzdb) && (__cpp_lib_tzdb >= 201907L)
    try {
        using namespace std::chrono;

        zoned_time const here{current_zone(), system_clock::now()};
        std::cout << "current_zone 本地当前时刻: " << here << '\n';

        auto const* shanghai = locate_zone("Asia/Shanghai");
        zoned_time const sh{shanghai, system_clock::now()};
        std::cout << "Asia/Shanghai: " << sh << '\n';
    } catch (std::exception const& ex) {
        std::cout << "时区数据库不可用或加载失败: " << ex.what() << '\n';
    }
#else
    std::cout << "标准库未声明 __cpp_lib_tzdb，跳过时区演示。\n";
#endif
#else
    std::cout << "未启用 C++20 chrono 日历基础（__cpp_lib_chrono），跳过时区演示。\n";
#endif
    return 0;
}
