// 模块 11 的小演示：noexcept 说明符（承诺不抛）与 noexcept 运算符（编译期查询）。

#include <iostream>
#include <utility>

namespace {

void stableApi() noexcept {
    // 意图：调用方无需 try-catch；实现变更若可能抛异常会触发编译问题。
}

int add(int x, int y) noexcept {
    return x + y;
}

void mightThrow() {
    // 空实现——仍属“允许抛异常”的异常规范，除非标为 noexcept。
}

// 移动为 noexcept 时，vector 等在扩容时更愿意移动而非拷贝。
struct WidgetForMove {
    WidgetForMove() = default;
    ~WidgetForMove() = default;
    WidgetForMove(WidgetForMove const&) = default;
    WidgetForMove& operator=(WidgetForMove const&) = default;
    WidgetForMove(WidgetForMove&&) noexcept = default;
    WidgetForMove& operator=(WidgetForMove&&) noexcept = default;
};

void touchForOdr() {
    stableApi();
    [[maybe_unused]] int const sum = add(2, 3);
    mightThrow();
}

void dumpNoexceptQueries() {
    std::cout << "noexcept(stableApi()): " << noexcept(stableApi()) << '\n';
    std::cout << "noexcept(mightThrow()): " << noexcept(mightThrow()) << '\n';
    std::cout << "noexcept(add(1,2)): " << noexcept(add(1, 2)) << '\n';

    std::cout << "noexcept(1 + 2): " << noexcept(1 + 2) << '\n';

    std::cout << "noexcept(WidgetForMove 移动构造): "
              << noexcept(WidgetForMove(std::declval<WidgetForMove&&>())) << '\n';
}

}  // namespace

int main() {
    dumpNoexceptQueries();
    touchForOdr();
    std::cout << "\n提示：移动构造函数常标为 noexcept，以便标准容器在重分配时安全地\n"
                 "迁移元素而不强依赖拷贝。\n";
    return 0;
}
