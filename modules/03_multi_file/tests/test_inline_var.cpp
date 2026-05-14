// inline 变量与函数测试：验证跨 TU 共享同一实例。

#include <string>

#include <gtest/gtest.h>

#include "inline_config.h"

// 在另一个 TU (test_inline_var_tu2.cpp) 中定义
int const* inlineVarAddrTu2();
std::string const* inlineNameAddrTu2();
int inlineDoubleTu2(int x);
int* inlineStaticMemberAddrTu2();

TEST(InlineVar, SameAddressAcrossTU) {
    // inline 变量在所有 TU 中是同一实例（地址相同）
    EXPECT_EQ(&g_app_version, inlineVarAddrTu2());
    EXPECT_EQ(&g_app_name, inlineNameAddrTu2());
}

TEST(InlineVar, SameValueAcrossTU) {
    EXPECT_EQ(g_app_version, 42);
    EXPECT_EQ(g_app_name, "MultiFileDemo");
}

TEST(InlineVar, MutationVisibleAcrossTU) {
    // 在本 TU 修改 inline 变量，另一个 TU 也能观察到
    int original = g_app_version;
    g_app_version = 999;
    EXPECT_EQ(*inlineVarAddrTu2(), 999);
    g_app_version = original;  // 恢复
}

TEST(InlineFunction, ConsistentBehaviorAcrossTU) {
    // inline 函数在两个 TU 中行为一致
    EXPECT_EQ(doubleValue(21), 42);
    EXPECT_EQ(inlineDoubleTu2(21), 42);
    EXPECT_EQ(doubleValue(0), inlineDoubleTu2(0));
}

TEST(InlineStaticMember, SameAddressAcrossTU) {
    // inline static 成员变量也跨 TU 共享
    EXPECT_EQ(&AppConfig::max_connections, inlineStaticMemberAddrTu2());
}

TEST(InlineStaticMember, InitialValue) {
    EXPECT_EQ(AppConfig::max_connections, 100);
    EXPECT_EQ(AppConfig::default_host, "localhost");
}
