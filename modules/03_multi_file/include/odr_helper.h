#ifndef MCPP_03_MULTI_FILE_ODR_HELPER_H
#define MCPP_03_MULTI_FILE_ODR_HELPER_H

// ODR 演示：头文件放声明（函数原型 + 类定义），源文件放函数体。
// 多个 TU 可以 #include 本文件而不违反 ODR。

#include <string>

// 函数声明（原型）—— 定义在 odr_helper.cpp
int add(int a, int b);
std::string greet(std::string const& name);

// 类定义 —— ODR 要求每个 TU 最多定义一次，且各 TU 定义必须相同。
// 把类定义放头文件并 #include 即可满足。
class Counter {
public:
    Counter() = default;
    explicit Counter(int init);

    void increment();
    void decrement();
    // 默认参数应放在声明中，不应放在定义中
    void incrementBy(int step = 1);
    [[nodiscard]] int value() const;

    // 静态数据成员 —— 仅是声明，需要在源文件中定义
    static int instance_count;

private:
    int count_{0};
};

#endif  // MCPP_03_MULTI_FILE_ODR_HELPER_H
