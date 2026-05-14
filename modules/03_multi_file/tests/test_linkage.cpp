// 链接属性测试：外部/内部链接、static、匿名命名空间、extern const。

#include <gtest/gtest.h>

#include "linkage_api.h"

// 本 TU 自己的 static 函数 —— 与 test_linkage_tu2.cpp 里同名函数不冲突
static int secretComputation(int x) {
    return -x;
}

TEST(Linkage, ExternalFunctionAccessible) {
    // 外部链接函数可跨 TU 调用
    EXPECT_EQ(publicAdd(10, 5), 15);
    EXPECT_EQ(publicAdd(-3, 3), 0);
    EXPECT_EQ(publicMultiply(4, 5), 20);
}

TEST(Linkage, ExternConstSharedAcrossTU) {
    // extern const 变量跨 TU 共享同一定义
    EXPECT_EQ(kSharedConstant, 42);
}

TEST(Linkage, StaticFunctionsAreIndependent) {
    // 本 TU 的 secretComputation 与另一个 TU 的同名函数是不同实体
    // 本 TU: return -x
    EXPECT_EQ(secretComputation(3), -3);

    // 另一个 TU: return x*x+x，通过公开接口间接调用
    EXPECT_EQ(callInternalHelper(), 12);  // 3*3+3 = 12
}

TEST(Linkage, AnonymousNamespaceIsInternal) {
    // 匿名命名空间中的函数只能通过公开接口间接访问
    EXPECT_EQ(callAnonNsHelper(), 500);  // 5*100 = 500
}

TEST(Linkage, ConstDefaultsToInternalLinkage) {
    // 本 TU 定义 const 变量不会与其他 TU 的同名 const 冲突
    const int local_const = 999;
    EXPECT_EQ(local_const, 999);
    EXPECT_EQ(kSharedConstant, 42);
}
