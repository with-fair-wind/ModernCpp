// std::weak_ptr：过期检测与 lock（）、弱引用不参与延长对象寿命。

#include <memory>

#include <gtest/gtest.h>

namespace {

struct Probe {
    int token{};
};

}  // namespace

TEST(WeakPtrLifecycle, LocksWhileSharedAliveAndExpiresLater) {
    std::weak_ptr<Probe> weak_handle;
    {
        auto shared_holder = std::make_shared<Probe>();
        shared_holder->token = 64;
        weak_handle = shared_holder;

        ASSERT_FALSE(weak_handle.expired());
        auto strong = weak_handle.lock();
        ASSERT_TRUE(strong);
        EXPECT_EQ(strong->token, 64);

        EXPECT_EQ(shared_holder.use_count(), weak_handle.use_count());
        EXPECT_GT(weak_handle.use_count(), 0);
    }

    ASSERT_TRUE(weak_handle.expired());
    EXPECT_FALSE(weak_handle.lock());
}

TEST(WeakPtrCycleBreak, KeepsParentsUseCountMinimal) {
    struct ChildFwd;

    struct ParentNode {
        std::shared_ptr<ChildFwd> child;
    };

    struct ChildFwd {
        std::weak_ptr<ParentNode> back;
    };

    auto parent = std::make_shared<ParentNode>();
    auto child = std::make_shared<ChildFwd>();
    parent->child = child;
    child->back = parent;

    // 父节点只被本地 shared_ptr 持有；子节点则由「本地句柄」与「父节点的 child」各持有一份。
    EXPECT_EQ(parent.use_count(), 1U);
    EXPECT_EQ(child.use_count(), 2U);

    ASSERT_FALSE(child->back.expired());
}
