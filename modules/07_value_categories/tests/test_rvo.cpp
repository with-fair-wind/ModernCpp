// RVO / NRVO：依赖优化，但复制侧应长期为 0；移动次数在 NRVO 成功时可为 0。

#include <gtest/gtest.h>

namespace {

class Probe {
public:
    Probe() {
        ++default_ctor;
        ++alive;
    }

    Probe(Probe const& /*unused*/) {
        ++copy_ctor;
        ++alive;
    }

    Probe(Probe&& /*unused*/) noexcept {
        ++move_ctor;
        ++alive;
    }

    Probe& operator=(Probe const&) = delete;
    Probe& operator=(Probe&&) = delete;

    ~Probe() {
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

Probe makeAnonymous() {
    return Probe{};
}

Probe makeNamed(int salt) {
    Probe local{};
    (void)salt;
    return local;
}

}  // namespace

TEST(Rvo, PrvalueReturnNeverCopies) {
    Probe::reset();
    {
        Probe slot = makeAnonymous();
        (void)slot;
    }

    EXPECT_EQ(Probe::copyCtorCalls(), 0);
    EXPECT_LE(Probe::moveCtorCalls(), 1);
    EXPECT_EQ(Probe::defaultCtorCalls(), 1);
    EXPECT_EQ(Probe::dtorCalls(), 1);
}

TEST(Rvo, NamedReturnAvoidsCopies) {
    Probe::reset();
    {
        Probe slot = makeNamed(9);
        (void)slot;
    }

    EXPECT_EQ(Probe::copyCtorCalls(), 0);
    EXPECT_LE(Probe::moveCtorCalls(), 1);
    EXPECT_EQ(Probe::defaultCtorCalls(), 1);
}

TEST(Rvo, MoveCountDoesNotExplodeOnSuccessiveReturns) {
    Probe::reset();
    for (int i = 0; i < 3; ++i) {
        Probe tmp = makeAnonymous();
        (void)tmp;
    }

    EXPECT_EQ(Probe::copyCtorCalls(), 0);
    EXPECT_EQ(Probe::defaultCtorCalls(), 3);
    EXPECT_EQ(Probe::dtorCalls(), 3);
    EXPECT_LE(Probe::moveCtorCalls(), 3);
}
