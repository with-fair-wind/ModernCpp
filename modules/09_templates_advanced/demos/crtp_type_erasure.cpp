// CRTP：奇异递归模板实现静态多态——基类通过 `static_cast<Derived*>` 调用派生实现且不引入虚函数开销。
// 类型擦除：`Function` 以小对象优化 + `std::unique_ptr` 持有可调用物，模拟 `std::function` 的子集。

#include <iostream>
#include <memory>
#include <utility>

namespace {

template <typename Derived>
class CounterBase {
private:
    friend Derived;
    CounterBase() = default;

public:
    [[nodiscard]] int tick() {
        return static_cast<Derived*>(this)->increment();
    }
};

class StepCounter : public CounterBase<StepCounter> {
private:
    int state_{0};
    int step_{1};

public:
    explicit StepCounter(int step) : step_(step) {}
    int increment() {
        state_ += step_;
        return state_;
    }
};

class CallableConcept {
public:
    virtual ~CallableConcept() = default;
    virtual void invoke(int value) const = 0;
};

template <typename Functor>
class CallableModel final : public CallableConcept {
private:
    Functor functor_;

public:
    explicit CallableModel(Functor functor) : functor_(std::move(functor)) {}

    void invoke(int value) const override {
        functor_(value);
    }
};

class Function {
private:
    std::unique_ptr<CallableConcept> target_;

public:
    template <typename Functor>
    Function(Functor functor)
        : target_(std::make_unique<CallableModel<Functor>>(std::move(functor))) {}

    void operator()(int value) const {
        if (target_) {
            target_->invoke(value);
        }
    }
};

}  // namespace

int main() {
    StepCounter counter{3};
    std::cout << "CRTP ticks: " << counter.tick() << ", " << counter.tick() << '\n';

    Function sink{[](int x) { std::cout << "erased call: " << x << '\n'; }};
    sink(42);

    return 0;
}
