// std::shared_ptr：别名构造引用同一控制块、use_count / make_shared 行为。

#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace {

struct Gadget {
    int id{};
    std::string name;
};

}  // namespace

TEST(SharedPtrCounting, CopySharesControlBlock) {
    auto gadget = std::make_shared<Gadget>();
    gadget->id = 5;

    EXPECT_EQ(gadget.use_count(), 1U);
    auto clone = gadget;
    EXPECT_EQ(gadget.use_count(), 2U);
    EXPECT_EQ(clone->id, 5);

    gadget.reset();

    ASSERT_TRUE(static_cast<bool>(clone));
    EXPECT_EQ(clone.use_count(), 1U);
}

TEST(SharedPtrAliasingCtor, HoldsMemberViaAlias) {
    auto owner =
        std::make_shared<Gadget>(Gadget{.id = 11, .name = std::string{"alias"}});
    std::shared_ptr<std::string> name_view(owner, &owner->name);

    EXPECT_EQ(owner.use_count(), 2U);  // name_view 别名仍抬升引用计数

    ASSERT_TRUE(name_view);
    EXPECT_EQ(*name_view, "alias");

    owner.reset();  // 实际对象仍存在，因为有 name_view 共享控制块

    ASSERT_TRUE(static_cast<bool>(name_view));
    EXPECT_EQ(*name_view, "alias");

    name_view.reset();  // 最终释放整块 Gadget

    ASSERT_FALSE(static_cast<bool>(name_view));
}

TEST(SharedPtrMakeShared, ObjectAndControlCreatedTogether) {
    auto pooled =
        std::make_shared<Gadget>(Gadget{.id = -3, .name = std::string{"stack-like"}});
    ASSERT_TRUE(static_cast<bool>(pooled));
    EXPECT_EQ(pooled->id, -3);
    EXPECT_FALSE(pooled->name.empty());
}
