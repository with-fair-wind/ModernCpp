// const& / && 临时对象延寿；以及"绕一圈就丢失延寿"的几种情形。

#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace {

struct Counted {
    static int alive;
    int x;
    explicit Counted(int v) : x(v) {
        ++alive;
    }
    Counted(Counted const& o) : x(o.x) {
        ++alive;
    }
    Counted(Counted&& o) noexcept : x(o.x) {
        ++alive;
    }
    Counted& operator=(Counted const&) = default;
    Counted& operator=(Counted&&) noexcept = default;
    ~Counted() {
        --alive;
    }
};
int Counted::alive = 0;

[[nodiscard]] Counted makeCounted(int v) {
    return Counted{v};
}

struct Wrapper {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    Counted const& ref;
};

}  // namespace

TEST(Extension, ConstRefHoldsTemporaryUntilBlockEnd) {
    Counted::alive = 0;
    {
        Counted const& c = makeCounted(11);
        EXPECT_EQ(c.x, 11);
        EXPECT_EQ(Counted::alive, 1);
    }
    EXPECT_EQ(Counted::alive, 0);
}

TEST(Extension, RvalueRefAlsoExtends) {
    Counted::alive = 0;
    {
        Counted&& c = makeCounted(22);
        EXPECT_EQ(c.x, 22);
        EXPECT_EQ(Counted::alive, 1);
    }
    EXPECT_EQ(Counted::alive, 0);
}

TEST(Extension, ConstRefValueReadableInExtendedScope) {
    // const& 直接绑定函数返回的临时：值仍然可读、生命周期到引用作用域结束。
    Counted::alive = 0;
    int x_at_inner = 0;
    {
        Counted const& c = makeCounted(33);
        x_at_inner = c.x;
        EXPECT_GE(Counted::alive, 1);  // 至少自己活着
    }
    EXPECT_EQ(x_at_inner, 33);
    EXPECT_EQ(Counted::alive, 0);
    (void)Wrapper{Counted{1}};  // 编译验证：聚合体可绑定到引用成员（运行期不读 ref）
}

TEST(Extension, MovingFromConstRefDoesNotKillSource) {
    Counted::alive = 0;
    {
        Counted const& c = makeCounted(7);
        // const&& 不能用 std::move 改变绑定 —— 这只是 *引用* 的副本
        Counted const& c2 = c;
        EXPECT_EQ(Counted::alive, 1);
        EXPECT_EQ(&c, &c2);
        EXPECT_EQ(c2.x, 7);
    }
    EXPECT_EQ(Counted::alive, 0);
}
