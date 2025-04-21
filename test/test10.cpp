#include "FunctionRegistry.h"
#include <iostream>

// 一个普通函数
int add(int a, int b)
{
    return a + b;
}

// 一个 void 返回的函数
void greet(const std::string &name)
{
    std::cout << "Hello, " << name << "!\n";
}

class Y
{
public:
    void f() {}
};

class X
{
    FunctionRegistry m_registry;

public:
    X()
    {
        m_registry.registerFunc("add", add);
        // m_registry.registerFunc("f", &Y::f);
    }
};

int main()
{
    X x;

    FunctionRegistry registry;

    // 注册函数指针
    registry.registerFunc("add", add);

    // // 注册 lambda：返回平方值
    // registry.registerFunc("square", [](double x)
    //                       { return x * x; });

    // // 注册 void 函数
    // registry.registerFunc("greet", greet);

    // 调用 add，自动推断返回类型 int
    // int sum = registry.invoke<int(int, int)>("add", 10, 15);
    int sum = registry.invoke<int>("add", 10, 15);
    std::cout << "10 + 15 = " << sum << "\n";

    // // 调用 square，自动推断返回类型 double
    // double sq = registry.invoke<double(double)>("square", 3.3);
    // std::cout << "3.3 squared = " << sq << "\n";

    // // 调用 void 函数
    // // registry.invoke<void(const std::string &)>("greet", "Alice");
    // registry.invoke<void(const std::string &)>("greet", std::string("Alice"));

    return 0;
}
