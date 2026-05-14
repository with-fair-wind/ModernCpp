#ifndef MCPP_03_MULTI_FILE_INLINE_CONFIG_H
#define MCPP_03_MULTI_FILE_INLINE_CONFIG_H

// C++17 inline 变量 & inline 函数演示。
// 本头文件可被多个 TU 包含，不会引发多重定义。

#include <string>

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

// inline 变量（C++17）—— 每个 TU 都有定义，链接器合并为唯一实例
// 演示目的：验证跨 TU 的地址唯一性和可变状态共享，因此必须为 non-const。
inline int g_app_version = 42;
// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
inline std::string g_app_name{"MultiFileDemo"};

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// inline 函数 —— 每个 TU 可见定义，链接器去重
inline int doubleValue(int x) {
    return x * 2;
}

// 包含 inline static 成员的类
class AppConfig {
public:
    // C++17 起可直接在类定义中初始化 inline static 成员
    inline static int max_connections =
        100;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    // NOLINTNEXTLINE(bugprone-throwing-static-initialization,cppcoreguidelines-avoid-non-const-global-variables)
    inline static std::string default_host{"localhost"};

    static int getMaxConnections() {
        return max_connections;
    }
};

#endif  // MCPP_03_MULTI_FILE_INLINE_CONFIG_H
