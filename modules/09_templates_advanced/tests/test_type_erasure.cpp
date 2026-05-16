// 简化 `std::function<int(int)>`：多态包装可调用目标。

#include <memory>
#include <utility>

#include <gtest/gtest.h>

namespace {

class IntCallableConcept {
public:
    virtual ~IntCallableConcept() = default;
    [[nodiscard]] virtual int invoke(int value) const = 0;
};

template <typename Functor>
class IntCallableModel final : public IntCallableConcept {
private:
    Functor functor_;

public:
    explicit IntCallableModel(Functor functor) : functor_(std::move(functor)) {}

    [[nodiscard]] int invoke(int value) const override { return functor_(value); }
};

class TypeErasedFunctor {
private:
    std::unique_ptr<IntCallableConcept> target_;

public:
    template <typename Functor>
    explicit TypeErasedFunctor(Functor functor)
        : target_(std::make_unique<IntCallableModel<Functor>>(std::move(functor))) {}

    int operator()(int value) const {
        if (!target_) {
            return 0;
        }
        return target_->invoke(value);
    }
};

}  // namespace

TEST(TypeErasure, InvokesLambdaStoredThroughVtable) {
    TypeErasedFunctor fn{[](int x) { return x * x; }};
    EXPECT_EQ(fn(6), 36);
}

TEST(TypeErasure, SwapsBehaviorWithState) {
    int offset = 10;
    TypeErasedFunctor adder{[&offset](int y) { return y + offset; }};

    EXPECT_EQ(adder(5), 15);

    offset = 20;
    EXPECT_EQ(adder(5), 25);
}
