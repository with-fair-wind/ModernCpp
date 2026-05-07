// 各种悬空 / 越生命周期使用的反例与正确做法。
//
// 覆盖文档中点到的几类陷阱：
//   - 返回指向局部变量的引用 / 指针。
//   - lambda 引用捕获，但 lambda 寿命比所引用对象长（被赋给 std::function）。
//   - 类成员函数中的 lambda 通过 this 捕获成员，对象销毁后 this 悬空。
//   - 返回 view（如 v | reverse）但底层容器已析构。
//
// "真的运行 UB" 的代码路径只有定义 MCPP_DEMONSTRATE_UB 时才编译；默认禁用，
// 这样 CI 的 -Werror / clang-tidy 不会被触发。

#include <functional>
#include <iostream>
#include <ranges>
#include <vector>

namespace {

#if defined(MCPP_DEMONSTRATE_UB)
// 反例 1：返回局部对象的引用
[[nodiscard]] int const& localRef() {
    int local = 42;
    return local;
}
#endif

[[nodiscard]] int localValue() {
    int local = 42;
    return local;
}

#if defined(MCPP_DEMONSTRATE_UB)
// 反例 2：把"引用捕获的 lambda"赋给比捕获对象活得更久的 std::function
[[nodiscard]] std::function<int()> makeDanglingCounter() {
    int counter = 100;
    return [&counter] { return counter; };
}
#endif

[[nodiscard]] std::function<int()> makeSafeCounter() {
    int counter = 100;
    return [counter] { return counter; };
}

// 反例 3：成员函数里的 [&] / [=] 不会真的拷贝成员 —— 它们都通过 this 访问
struct Widget {
    int value{7};

    [[nodiscard]] std::function<int()> badLambda() {
        // 捕获 this：调用方拿到的 lambda 在 *this 销毁后失效。
        return [this] { return value; };
    }

    [[nodiscard]] std::function<int()> safeLambdaCopyThis() const {
        return [*this] { return value; };  // 拷贝 *this
    }

    [[nodiscard]] std::function<int()> safeLambdaCopyValue() const {
        int v = value;
        return [v] { return v; };
    }
};

}  // namespace

int main() {
    std::cout << "[1] 返回局部值 vs 局部引用\n";
    std::cout << "  localValue() = " << localValue() << "\n";
#if defined(MCPP_DEMONSTRATE_UB)
    std::cout << "  localRef()   = " << localRef() << "  (UB!)\n";
#else
    std::cout << "  localRef()   = <skipped: see source comments>\n";
#endif

    std::cout << "\n[2] 引用捕获 lambda 越界使用\n";
    {
#if defined(MCPP_DEMONSTRATE_UB)
        auto dangling = makeDanglingCounter();
        std::cout << "  dangling() = " << dangling() << "  (读已死栈帧)\n";
#else
        std::cout << "  dangling() = <skipped: would read dead stack frame>\n";
#endif

        auto safe = makeSafeCounter();
        std::cout << "  safe()     = " << safe() << "\n";
    }

    std::cout << "\n[3] 类成员里 lambda 的捕获\n";
    {
        std::function<int()> f1;
        std::function<int()> f2;
        std::function<int()> f3;
        {
            Widget w{};
            f1 = w.badLambda();
            f2 = w.safeLambdaCopyThis();
            f3 = w.safeLambdaCopyValue();
        }
        // 离开块后 w 已死。f1 调用是 UB；f2/f3 仍然安全。
#if defined(MCPP_DEMONSTRATE_UB)
        std::cout << "  f1() = " << f1() << "  (UB!)\n";
#else
        std::cout << "  f1() = <skipped: w is dead>\n";
        (void)f1;
#endif
        std::cout << "  f2() = " << f2() << "\n";
        std::cout << "  f3() = " << f3() << "\n";
    }

    std::cout << "\n[4] view（ranges）必须比底层容器活得短\n";
    {
        std::vector<int> v{1, 2, 3, 4, 5};
        auto rv = v | std::views::reverse;
        std::cout << "  reversed: ";
        for (int x : rv) {
            std::cout << x << ' ';
        }
        std::cout << "\n";
        // 反例（仅文字说明）：从函数返回左值容器之上的 view 会悬空 —— 容器
        // 已析构，view 内部的 ref_view 指向死内存。
    }

    return 0;
}
