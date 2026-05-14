// inline 变量演示的第二个翻译单元。
// 同样包含 inline_config.h，与主 TU 共享 inline 变量实例。

#include "inline_config.h"

#include <string>

int const* getVersionAddrFromTu2() { return &g_app_version; }

std::string const* getNameAddrFromTu2() { return &g_app_name; }

int doubleFromTu2(int x) { return doubleValue(x); }
