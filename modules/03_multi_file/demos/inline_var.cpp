// inline 变量与函数演示（主 TU）。
// inline_config.h 中定义了 inline 变量和 inline 函数，
// 本文件与 inline_var_user.cpp 同时包含该头文件，
// 验证它们共享同一实例（地址相同）。

#include <iostream>
#include <string>

#include "inline_config.h"

// 声明在另一个 TU 中定义的函数（inline_var_user.cpp）
int const* getVersionAddrFromTu2();
std::string const* getNameAddrFromTu2();
int doubleFromTu2(int x);

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== inline 变量与函数演示 =====\n\n";

    // 1. inline 变量 —— 跨 TU 共享同一实例
    std::cout << "-- inline 变量 --\n";
    std::cout << "g_app_version = " << g_app_version << '\n';
    std::cout << "g_app_name = " << g_app_name << '\n';

    // 验证跨 TU 地址一致
    std::cout << "本 TU   &g_app_version = " << &g_app_version << '\n';
    std::cout << "另一 TU &g_app_version = " << getVersionAddrFromTu2() << '\n';
    std::cout << "地址一致: " << (&g_app_version == getVersionAddrFromTu2() ? "是" : "否") << '\n';
    std::cout << '\n';

    std::cout << "本 TU   &g_app_name = " << &g_app_name << '\n';
    std::cout << "另一 TU &g_app_name = " << getNameAddrFromTu2() << '\n';
    std::cout << "地址一致: " << (&g_app_name == getNameAddrFromTu2() ? "是" : "否") << '\n';
    std::cout << '\n';

    // 2. inline 函数 —— 多个 TU 各自有定义，链接器去重
    std::cout << "-- inline 函数 --\n";
    std::cout << "本 TU: doubleValue(21) = " << doubleValue(21) << '\n';
    std::cout << "另 TU: doubleFromTu2(21) = " << doubleFromTu2(21) << '\n';
    std::cout << '\n';

    // 3. inline static 类成员
    std::cout << "-- inline static 类成员 --\n";
    std::cout << "AppConfig::max_connections = " << AppConfig::max_connections << '\n';
    std::cout << "AppConfig::default_host = " << AppConfig::default_host << '\n';

    // 修改后在另一个 TU 也能观察到（因为是同一实例）
    AppConfig::max_connections = 200;
    std::cout << "修改后 max_connections = " << AppConfig::getMaxConnections() << '\n';

    return 0;
}
