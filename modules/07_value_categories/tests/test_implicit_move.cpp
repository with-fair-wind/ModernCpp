// 隐式移动：按值返回函数形参或局部对象时，典型实现会产生移动而非多余的拷贝。

#include <utility>

#include <gtest/gtest.h>

namespace {

class Counted {
public:
    Counted() {
        ++default_count;
    }

    Counted(Counted const& /*unused*/) {
        ++copy_count;
    }

    Counted(Counted&& /*unused*/) noexcept {
        ++move_count;
    }

    Counted& operator=(Counted const&) = default;
    Counted& operator=(Counted&&) noexcept = default;

    ~Counted() = default;

    static void reset() {
        default_count = 0;
        copy_count = 0;
        move_count = 0;
    }

    static int defaults() {
        return default_count;
    }
    static int copies() {
        return copy_count;
    }
    static int moves() {
        return move_count;
    }

private:
    static inline int default_count = 0;
    static inline int copy_count = 0;
    static inline int move_count = 0;
};

Counted relayBox(Counted boxed) {
    return boxed;
}

Counted chooseBranch(bool flag) {
    Counted left{};
    Counted right{};
    if (flag) {
        return left;
    }
    return right;
}

}  // namespace

TEST(ImplicitMove, ReturningParameterAvoidsCopies) {
    Counted::reset();
    {
        Counted outer = relayBox(Counted{});
        (void)outer;
    }

    EXPECT_EQ(Counted::copies(), 0);
    EXPECT_GE(Counted::defaults(), 1);
    // 强制复制消除 / NRVO 可能连移动一并省略；拷贝次数仍是最佳观测指标。
    EXPECT_LE(Counted::moves(), 3);
}

TEST(ImplicitMove, DualLocalReturnsAvoidCopies) {
    Counted::reset();
    {
        Counted sink = chooseBranch(true);
        (void)sink;
    }

    EXPECT_EQ(Counted::copies(), 0);
    EXPECT_GE(Counted::defaults(), 2);
    EXPECT_LE(Counted::moves(), 6);
}

TEST(ImplicitMove, AlternateBranchAlsoAvoidsCopies) {
    Counted::reset();
    {
        Counted sink = chooseBranch(false);
        (void)sink;
    }

    EXPECT_EQ(Counted::copies(), 0);
    EXPECT_GE(Counted::defaults(), 2);
    EXPECT_LE(Counted::moves(), 6);
}

TEST(ImplicitMove, ExplicitMoveIncrementsMoveCounter) {
    Counted::reset();
    Counted donor{};
    Counted sink = std::move(donor);
    (void)sink;

    EXPECT_EQ(Counted::copies(), 0);
    EXPECT_EQ(Counted::moves(), 1);
    EXPECT_EQ(Counted::defaults(), 1);
}
