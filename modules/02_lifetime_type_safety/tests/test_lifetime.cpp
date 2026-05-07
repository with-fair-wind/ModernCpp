// 模块 02 的核心生命周期测试：作用域析构、引用绑定、const& 延寿、Probe 计数。

#include <string>
#include <utility>

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

struct Heavy {
    static int alive;
    int x;
    explicit Heavy(int v) : x(v) {
        ++alive;
    }
    Heavy(Heavy const& o) : x(o.x) {
        ++alive;
    }
    Heavy(Heavy&& o) noexcept : x(o.x) {
        ++alive;
    }
    Heavy& operator=(Heavy const&) = default;
    Heavy& operator=(Heavy&&) noexcept = default;
    ~Heavy() {
        --alive;
    }
};
int Heavy::alive = 0;

[[nodiscard]] Heavy makeHeavy(int v) {
    return Heavy{v};
}

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

TEST(Lifetime, ConstRefExtendsTemporaryToBlockEnd) {
    Heavy::alive = 0;
    int outer_alive = 0;
    {
        Heavy const& h = makeHeavy(42);  // 临时 heavy 寿命延长到块末
        EXPECT_EQ(h.x, 42);
        outer_alive = Heavy::alive;
    }
    EXPECT_EQ(outer_alive, 1);   // 块内仅 1 个对象活着（无多余拷贝）
    EXPECT_EQ(Heavy::alive, 0);  // 离开块后被销毁
}

TEST(Lifetime, RvalueRefAlsoExtendsTemporary) {
    Heavy::alive = 0;
    {
        Heavy&& h = makeHeavy(7);
        EXPECT_EQ(h.x, 7);
        EXPECT_EQ(Heavy::alive, 1);
    }
    EXPECT_EQ(Heavy::alive, 0);
}

TEST(Lifetime, TemporaryDiesAtFullExpressionEnd) {
    Heavy::alive = 0;
    int snapshot_inside = Heavy::alive;
    int x = makeHeavy(99).x + 0;  // 临时只活到这条 ; —— 用 +0 强制不被复制延寿
    snapshot_inside = Heavy::alive;
    EXPECT_EQ(x, 99);
    EXPECT_EQ(snapshot_inside, 0);  // 这一行执行时，临时已经析构
}

TEST(Lifetime, MoveDoesNotDestroySourceImmediately) {
    Heavy::alive = 0;
    {
        Heavy a{1};
        Heavy b{std::move(a)};  // 移动构造：a 仍然活着，只是状态被转移
        EXPECT_EQ(Heavy::alive, 2);
    }
    EXPECT_EQ(Heavy::alive, 0);
}
