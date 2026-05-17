// consteval：立即函数，只能在常量求值语境中调用；constinit：强制静态常量初始化。
//
// consteval 与 constexpr 的差别在于前者禁止产生“仅存于运行期”的调用。

#include <iostream>

namespace {

consteval int squareImmediate(int x) noexcept {
    return x * x;
}

// constinit 防止静态变量的“暂缓初始化”；此处值必须在编译期确定。
constexpr int kSeed = 7;

struct Widget {
    int value{};
    constexpr explicit Widget(int v) noexcept : value{v} {}
};

// 函数内 static constinit 易被命名检查当作普通变量；放到匿名命名空间以匹配全局常量风格。
constinit const int kInitializedAtCompileTime = squareImmediate(kSeed);
constinit const Widget kInitializedWidget{squareImmediate(4)};

}  // namespace

int main() {
    // squareImmediate(...) 可在需要常量表达式的位置使用。
    static constexpr int kCompiled = squareImmediate(11);
    std::cout << "consteval square(11) = " << kCompiled << '\n';
    std::cout << "constinit static int = " << kInitializedAtCompileTime << '\n';
    std::cout << "constinit Widget.value = " << kInitializedWidget.value << '\n';

    // 下面这行若取消注释将无法通过编译：consteval 不可在纯运行期调用。
    // int r = 3;
    // std::cout << squareImmediate(r) << '\n';

    return 0;
}
