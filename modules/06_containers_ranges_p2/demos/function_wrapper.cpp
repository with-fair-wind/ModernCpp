// std::function / std::move_only_function / std::reference_wrapper。
//
// 关键点：
//   - std::function<R(Args...)> 是"类型擦除"的可调用对象包装器：能容纳函数指针、
//     functor、lambda（带捕获）、成员函数指针——但要求被包装对象 *可拷贝*。
//   - 调用空 std::function 抛 std::bad_function_call。空判用 if(f) 或 != nullptr。
//   - 缺陷：对小对象有 SOO；超出小缓冲区会触发 new/delete，性能可能比直接调用慢
//     10–20%。
//   - C++23 的 std::move_only_function 解除"必须可拷贝"的限制——例如可以装
//     unique_ptr 捕获的 lambda。
//   - std::ref/cref 把引用具现化为 std::reference_wrapper，可放进容器、bind 进
//     std::function 中（避免被拷贝吞掉）。

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <version>

namespace {

int add(int a, int b) {
    return a + b;
}

struct Adder {
    int base;
    int operator()(int a, int b) const {
        return base + a + b;
    }
};

}  // namespace

int main() {
    // 1) std::function 容纳多种可调用对象
    std::function<int(int, int)> f = add;
    std::cout << "function(普通函数)  : " << f(1, 2) << '\n';

    f = Adder{100};
    std::cout << "function(functor)   : " << f(1, 2) << '\n';

    int captured = 1000;
    f = [captured](int a, int b) { return captured + a + b; };
    std::cout << "function(lambda)    : " << f(1, 2) << '\n';

    // 2) 调用空 function 抛异常
    std::function<void()> empty;
    try {
        empty();
    } catch (const std::bad_function_call& e) {
        std::cout << "empty() 抛出: " << e.what() << '\n';
    }
    std::cout << "if(empty)?         : " << static_cast<bool>(empty) << '\n';

    // 3) std::move_only_function：可装 unique_ptr 捕获的 lambda
    //    C++23 P0288R9：libstdc++ 14+ / MSVC 17.4+ 已实现，libc++ 当前还没有；
    //    用 feature test macro 兜底跳过这一段。
#if defined(__cpp_lib_move_only_function) && __cpp_lib_move_only_function >= 202110L
    auto p = std::make_unique<int>(42);
    std::move_only_function<int()> mof = [up = std::move(p)] { return *up; };
    std::cout << "move_only_function : " << mof() << '\n';
    // std::function<int()> bad = std::move(mof);  // 编译错：mof 不可拷贝
#else
    std::cout << "move_only_function : 当前 stdlib 未实现 std::move_only_function —— 跳过\n";
#endif

    // 4) std::ref / cref：让 function 持有引用而非拷贝（生命周期由调用方负责）
    std::string log{"start"};
    auto append = [&log](std::string_view s) { log += '|', log += s; };
    std::function<void(std::string_view)> stored = std::ref(append);
    stored("a");
    stored("b");
    std::cout << "log via ref()      : " << log << '\n';

    // 5) std::reference_wrapper 也能放进容器（裸引用不能！）
    int x = 1;
    int y = 2;
    int z = 3;
    std::vector<std::reference_wrapper<int>> refs{x, y, z};
    for (auto& r : refs) {
        r.get() *= 10;
    }
    std::cout << "x,y,z refs (*=10) : " << x << ' ' << y << ' ' << z << '\n';

    return 0;
}
