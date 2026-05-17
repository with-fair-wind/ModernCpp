// C++17 强制复制消除：prvalue 初始化场景下不应出现额外的拷贝/移动构造。

#include <gtest/gtest.h>

namespace {

class Brick {
public:
    Brick() {
        ++default_ctor;
        ++alive;
    }

    Brick(Brick const& /*unused*/) {
        ++copy_ctor;
        ++alive;
    }

    Brick(Brick&& /*unused*/) noexcept {
        ++move_ctor;
        ++alive;
    }

    Brick& operator=(Brick const&) = delete;
    Brick& operator=(Brick&&) = delete;

    ~Brick() {
        ++dtor;
        --alive;
    }

    static void reset() {
        default_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        dtor = 0;
        alive = 0;
    }

    static int defaultCtorCalls() {
        return default_ctor;
    }
    static int copyCtorCalls() {
        return copy_ctor;
    }
    static int moveCtorCalls() {
        return move_ctor;
    }
    static int dtorCalls() {
        return dtor;
    }

private:
    static inline int default_ctor = 0;
    static inline int copy_ctor = 0;
    static inline int move_ctor = 0;
    static inline int dtor = 0;
    static inline int alive = 0;
};

Brick fabricate() {
    return Brick{};
}

}  // namespace

TEST(CopyElision, PrvalueInitializationSkipsCopyAndMove) {
    Brick::reset();
    {
        Brick wall = fabricate();
        (void)wall;
    }

    EXPECT_EQ(Brick::defaultCtorCalls(), 1);
    EXPECT_EQ(Brick::copyCtorCalls(), 0);
    EXPECT_EQ(Brick::moveCtorCalls(), 0);
    EXPECT_EQ(Brick::dtorCalls(), 1);
}

TEST(CopyElision, DirectTemporaryAlsoElides) {
    Brick::reset();
    {
        Brick wall = Brick{};
        (void)wall;
    }

    EXPECT_EQ(Brick::defaultCtorCalls(), 1);
    EXPECT_EQ(Brick::copyCtorCalls(), 0);
    EXPECT_EQ(Brick::moveCtorCalls(), 0);
    EXPECT_EQ(Brick::dtorCalls(), 1);
}

TEST(CopyElision, DestructorBalancedWithSingleObject) {
    Brick::reset();
    {
        (void)fabricate();
    }

    EXPECT_EQ(Brick::defaultCtorCalls(), 1);
    EXPECT_EQ(Brick::dtorCalls(), 1);
}
