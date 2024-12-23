#include <iostream>
#include <memory>

#define Version_6
#ifdef Version_1
// 在函数声明中，所有在拥有默认实参的形参之后的形参必须拥有在这个或同一作用域中先前的声明中所提供的默认实参。
void f(int, int, int = 10);
void f(int, int = 6, int);
void f(int = 4, int, int);
void f(int a, int b, int c) { std::cout << a << ' ' << b << ' ' << c << '\n'; }

int main()
{
    f(); // 4 6 10
}
#elif defined(Version_2)
// 有例外情况是.除非该形参是从某个形参包展开得到的或是函数形参包,如下:
template <class... T>
struct X
{
    void f(int n = 0, T... t)
    {
        std::cout << n << ' ';
        ((std::cout << t << ' '), ...);
        std::cout << std::endl;
    };
};
template <class... Args>
void f_(int n = 6, Args... args)
{
    std::cout << n << ' ';
    ((std::cout << args << ' '), ...);
    std::cout << std::endl;
}

int main()
{
    X().f();
    X<int>().f(1, 2); // 实例化了X::f(int n=0,int)
    X<double, int, std::string>().f(3, 3.5, 2, "kk");
    f_(7, "xiongda", 3.5, 48);
}
#elif defined(Version_3)
#if 0
class C
{
public:
    void f(int i = 3);
    void g(int i, int j = 99);
    C(int arg); // 非默认构造函数
};

// void C::f(int i = 3) {} // 默认实参重定义
void C::f(int i) {}
void C::g(int i = 88, int j) {}
C::C(int arg = 1) {} // 使非默认构造函数转为了默认构造，程序非良构
#else
class C
{
public:
    void f(int i = 3);
    void g(int i, int j = 99);
    C(int arg = 1); // 默认构造函数
};

// void C::f(int i = 3) {} // 默认实参重定义
void C::f(int i) {}
void C::g(int i = 88, int j) {}
C::C(int arg) { std::cout << arg << std::endl; }
#endif

int main()
{
    C c;
}
#elif defined(Version_4)
// 虚函数的覆盖函数不会从基类定义获得默认实参，而在进行虚函数调用时，默认实参根据对象的静态类型确定
struct Base
{
    virtual void f(int a = 7) { std::cout << "Base " << a << std::endl; }
};
struct Derived : Base
{
    void f(int a = 8) override { std::cout << "Derived " << a << std::endl; }
};

int main()
{
    std::unique_ptr<Base> ptr{new Derived};
    ptr->f(); // Derived 7
}
#elif defined(Version_5)
// 默认实参中能在不求值语境使用局部变量，sizeof显然是不求值的
int main()
{
    int f = 0;
    void f2(int n = sizeof f);
    f2();
}

void f2(int n)
{
    std::cout << n << '\n';
}
#elif defined(Version_6)
struct X
{
    int n = 6;
    static const int a = 6;
    void f(int n = sizeof +n) { std::cout << n << '\n'; } // 默认实参中不能使用非静态的类成员（即使它们不被求值）编译器没有遵守规则
    void f_(int n = a) { std::cout << n << '\n'; }
};

int main()
{
    X().f();
    X().f_();
}
#endif