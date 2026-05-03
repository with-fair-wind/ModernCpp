// 模块 02 的烟雾测试：对象生命周期与引用绑定规则。

#include <string>

#include <gtest/gtest.h>

namespace {

struct Counter {
    int* live;
    explicit Counter(int* l) : live(l) {
        ++*live;
    }
    Counter(Counter const& other) : live(other.live) {
        ++*live;
    }
    Counter(Counter&& other) noexcept : live(other.live) {
        ++*live;
    }
    Counter& operator=(Counter const&) = default;
    Counter& operator=(Counter&&) noexcept = default;
    ~Counter() {
        --*live;
    }
};

}  // namespace

TEST(Lifetime, ScopedObjectsAreDestroyedAtScopeExit) {
    int live = 0;
    {
        Counter const a{&live};
        EXPECT_EQ(live, 1);
        {
            Counter const b{&live};
            EXPECT_EQ(live, 2);
        }
        EXPECT_EQ(live, 1);
    }
    EXPECT_EQ(live, 0);
}

TEST(Lifetime, ReferenceBindsToInitializerNotLater) {
    std::string a{"first"};
    std::string b{"second"};
    std::string& ref = a;
    ref = b;  // 经由 ref 写入 a；不会把 ref 重绑到 b
    EXPECT_EQ(a, "second");
    EXPECT_EQ(&ref, &a);
}
