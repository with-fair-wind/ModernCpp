// std::shared_ptr：控制块、别名构造、make_shared 的块内分配优势。

#include <iostream>
#include <memory>
#include <string>

namespace {
struct Widget {
    int id{};
    std::string label;
};

void demonstrateUseCount() {
    auto a = std::make_shared<Widget>(Widget{.id = 1, .label = "alpha"});
    std::cout << "use_count(a) 初始: " << a.use_count() << '\n';
    std::shared_ptr<Widget> b = a;  // NOLINT(performance-unnecessary-copy-initialization)
    std::cout << "共享后: " << a.use_count() << '\n';
}

void demonstrateAliasing() {
    auto owner = std::make_shared<Widget>(Widget{.id = 2, .label = "beta"});
    // 别名构造：控制块仍管理整个 Widget，但 get() 指向其某个成员子对象。
    std::shared_ptr<std::string> label_view(owner, &owner->label);
    std::cout << "别名指针 -> " << *label_view << " | owner.use_count=" << owner.use_count()
              << '\n';
}

}  // namespace

int main() {
    demonstrateUseCount();
    demonstrateAliasing();
    std::cout << "make_shared：对象与控制块通常同窗分配，堆次数更少。\n";
    return 0;
}
