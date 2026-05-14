// ODR 演示：函数与类成员的定义放在源文件中。
// 头文件 odr_helper.h 暴露声明，多个 TU 可安全包含。

#include "odr_helper.h"

#include <string>

// 静态数据成员定义 —— 整个程序中只能定义一次
int Counter::instance_count = 0;

// 函数定义
int add(int a, int b) {
    return a + b;
}

std::string greet(std::string const& name) {
    return "Hello, " + name + "!";
}

// 类成员函数的类外定义
Counter::Counter(int init) : count_{init} {
    ++instance_count;
}

void Counter::increment() {
    ++count_;
}

void Counter::decrement() {
    --count_;
}

void Counter::incrementBy(int step) {
    count_ += step;
}

int Counter::value() const {
    return count_;
}
