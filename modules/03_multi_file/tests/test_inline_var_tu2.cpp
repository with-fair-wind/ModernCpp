// inline 变量测试的第二个 TU。
// 包含同一头文件，提供函数让主测试 TU 查询 inline 变量/函数的地址与值。

#include <string>

#include "inline_config.h"

int const* inlineVarAddrTu2() {
    return &g_app_version;
}

std::string const* inlineNameAddrTu2() {
    return &g_app_name;
}

int inlineDoubleTu2(int x) {
    return doubleValue(x);
}

int* inlineStaticMemberAddrTu2() {
    return &AppConfig::max_connections;
}
