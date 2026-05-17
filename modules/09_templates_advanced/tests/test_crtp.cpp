// CRTP：`CounterBase<Derived>` 通过派生类静态多态递增状态。

#include <gtest/gtest.h>

namespace {

template <typename Derived>
class CounterBase {
private:
    friend Derived;
    CounterBase() = default;

public:
    int bump() {
        return static_cast<Derived*>(this)->step();
    }
};

class LinearCounter : public CounterBase<LinearCounter> {
private:
    int state_{0};

public:
    int step() {
        state_ += 1;
        return state_;
    }

    [[nodiscard]] int read() const noexcept {
        return state_;
    }
};

}  // namespace

TEST(Crtp, AdvancesWithoutVirtuals) {
    LinearCounter counter{};
    EXPECT_EQ(counter.bump(), 1);
    EXPECT_EQ(counter.bump(), 2);
    EXPECT_EQ(counter.read(), 2);
}
