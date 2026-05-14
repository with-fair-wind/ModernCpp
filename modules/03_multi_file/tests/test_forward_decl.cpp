// 前置声明测试：验证互相持有对方指针的两个类能正确协作。
// 对照文档「不完整类型 / 前置声明 / 循环依赖」章节。

#include <string>

#include <gtest/gtest.h>

#include "forward_a.h"
#include "forward_b.h"

TEST(ForwardDecl, ConstructAndDestruct) {
    // 两个互相前置声明的类都能正常构造和析构
    NodeA a{"Alice"};
    NodeB b{"Bob"};
    EXPECT_NE(a.describe().find("Alice"), std::string::npos);
    EXPECT_NE(b.describe().find("Bob"), std::string::npos);
}

TEST(ForwardDecl, PointerMemberDefaultsToNull) {
    // 前置声明的指针成员初始化为 nullptr，describe() 不含 partner 信息
    NodeA a{"Solo"};
    EXPECT_EQ(a.describe(), R"(NodeA("Solo"))");

    NodeB b{"Lone"};
    EXPECT_EQ(b.describe(), R"(NodeB("Lone"))");
}

TEST(ForwardDecl, SetPartnerCrossReference) {
    // 互相设置指针后，describe() 能递归获取对方信息
    NodeA a{"Alice"};
    NodeB b{"Bob"};

    a.setPartner(&b);
    b.setPartner(&a);

    std::string desc_a = a.describe();
    EXPECT_NE(desc_a.find("Alice"), std::string::npos);
    EXPECT_NE(desc_a.find("Bob"), std::string::npos);

    std::string desc_b = b.describe();
    EXPECT_NE(desc_b.find("Bob"), std::string::npos);
    EXPECT_NE(desc_b.find("Alice"), std::string::npos);
}

TEST(ForwardDecl, PartnerCanBeReset) {
    NodeA a{"X"};
    NodeB b{"Y"};

    a.setPartner(&b);
    EXPECT_NE(a.describe().find('Y'), std::string::npos);

    // 重置为 nullptr
    a.setPartner(nullptr);
    EXPECT_EQ(a.describe(), R"(NodeA("X"))");
}
