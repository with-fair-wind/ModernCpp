// Lambda 捕获、mutable、泛型 lambda（C++14）、缩写函数模板（C++20）。
//
// Lambda 本质上是一个匿名结构体，带有 operator()。捕获列表就是它的
// 数据成员列表；按值 / 按引用捕获分别对应 T / T& 类型。每个 lambda
// 表达式都有唯一的闭包类型，因此任何两个 lambda 都不是同一类型。

#include <iostream>
#include <string>

namespace {

// 缩写函数模板：每个 `auto` 形参隐式引入一个模板参数，
// 等价于 `template<class A, class B> auto add(A, B)`。
auto add(auto x, auto y) {
    return x + y;
}

}  // namespace

int main() {
    int counter = 0;

    auto bump = [&counter] { ++counter; };
    bump();
    bump();
    bump();
    std::cout << "after 3x bump, counter = " << counter << '\n';

    // 按值捕获：闭包持有自己的副本。`mutable` 去掉 operator() 上的
    // 隐式 const，让副本可以被修改。
    auto cached = [counter]() mutable {
        counter += 100;
        return counter;
    };
    std::cout << "cached() = " << cached() << ", outer counter = " << counter << " (untouched)\n";

    // 泛型 lambda：闭包本身不是模板，但其 operator() 是模板。
    auto twice = [](auto const& v) { return v + v; };
    std::cout << "twice(21)    = " << twice(21) << '\n';
    std::cout << "twice(\"ha\") = " << twice(std::string{"ha"}) << '\n';

    std::cout << "add(1, 2.5) = " << add(1, 2.5) << '\n';

    return 0;
}
