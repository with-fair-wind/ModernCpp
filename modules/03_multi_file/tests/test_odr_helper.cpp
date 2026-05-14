// ODR 测试辅助 TU：提供头文件中声明的函数和类的定义。
// 与 test_odr.cpp 链接，验证跨 TU 行为。

#include <string>

#include "odr_helper.h"

// 静态成员定义
int Counter::instance_count = 0;

// 函数定义
int add(int a, int b) {
    return a + b;
}

std::string greet(std::string const& name) {
    return "Hello, " + name + "!";
}

// 成员函数定义
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
