/*
1: 成员指针分为: 成员函数指、数据成员指针
    "数据成员指针"或"虚函数成员指针"并没有真的指向一个内存，它只是表示表示在当前的类，那个字段的位置而已，比如&X::value表示的只是这个数据成员value在X类中的位置。
2: 数据成员指针和虚成员函数指针是一个类似偏移量的东西，成员函数指针则真的存储了一个地址
3: 成员指针也没办法脱离类的实例对象单独使用，不管是非静态数据成员指针还是非静态成员函数指针
4: 静态数据成员和静态成员函数不与类关联，也就不参与这个成员指针的讨论; 成员指针没被 std::is_pointer 包含在内，也不同于一般的指针的行为，成员指针不是常规的指针
*/
#include <cstdint>
#include <functional>
#include <iostream>

#define Version_8
#ifdef Version_1
// 成员函数指针
struct X
{
    void f() { std::cout << "6"; }
};
int main()
{
    void (X::*p)() = &X::f;
    X x;
    (x.*p)(); // 6
}

#elif defined(Version_2)
struct X
{
    void f() { std::cout << "6"; }
};

void f2(void (X::*p)(), X &x)
{
    (x.*p)();
}

int main()
{
    X x;
    f2(&X::f, x); // 6
}

#elif defined(Version_3)
// 必须使用static_cast指定类型的成员函数到成员函数指针类型转换来消解函数重载的歧义
// 除了成员函数这样解决，其他的普通函数重载都可以使用这种方式解决
struct Test_bind
{
    void t(int n)
    {
        for (; n; n--)
            std::cout << "t\n";
    }
    void t()
    {
        std::cout << "重载的t\n";
    }
};

int main()
{
    Test_bind t_b;
    auto n = std::bind(static_cast<void (Test_bind::*)(int)>(&Test_bind::t), &t_b, 3);
    n();
}

#elif defined(Version_4)
struct X
{
    void f() { std::cout << "6\n"; }
    template <typename Ty>
    // operator.*不可以重载，但是operator->*可以重载
    auto operator->*(Ty v)
    {
        return (this->*v)();
    }
};

int main()
{
    X x;
    x->*&X::f;
}

#elif defined(Version_5)
// 数据成员指针
struct X
{
    int x = 1;
};

int main()
{
    int X::*n = &X::x;
    X x;
    std::cout << x.x << '\n'; // 1
    int &v = (x.*n);
    v = 100;
    std::cout << x.x << '\n'; // 100
}

#elif defined(Version_6)
struct X
{
    int x = 1;
    int y = 2;
};

void f(int X::*v, X *x)
{
    (x->*v) = 66;
}

int main()
{
    X x;
    f(&X::x, &x);
    // f(&X::y, &x);
    std::cout << x.x << " " << x.y << '\n'; // 66
}

#elif defined(Version_7)
class Test
{
public:
    uint16_t num = 0;
    void f()
    {
        std::cout << "66\n";
    }
};

int main()
{
    Test c;
    uint16_t &i = std::invoke(&Test::num, &c); // 绑定数据成员返回引用
    i = 9999;
    std::cout << c.num << '\n';
    std::invoke(&Test::f, &c); // 直接调用，无返回值
}

#elif defined(Version_8)
#define virtual_member_function_point
#ifdef member_function_point
// 普通的成员函数指针是存了明确的地址的
struct X
{
    int a = 0;
    void f() { std::cout << "func\n"; }
    void g() { std::cout << this->a << "\n"; }
};
int main()
{
    using Func = void (*)(X *const); // 普通函数指针
#if 0
    auto p = &X::f;
    auto func = (Func)(p); // 成员函数指针强制转换为普通函数指针 未定义行为
    func(nullptr);         // 打印func
#else
    auto p = &X::g;
    X x;
    auto func = (Func)(p); // 强制转化
    func(&x);              // 打印func
#endif
}

#elif defined(data_member_point)
struct X
{
    int a, b;
    double d;
};
int main()
{
    auto p = &X::a;
    auto p2 = &X::b;
    auto p3 = &X::d;
    std::cout << *reinterpret_cast<int *>(&p) << '\n';  // 0
    std::cout << *reinterpret_cast<int *>(&p2) << '\n'; // 4
    std::cout << *reinterpret_cast<int *>(&p3) << '\n'; // 8
}

#elif defined(virtual_member_function_point)
struct X
{
    virtual void func()
    {
        std::cout << "X\n";
    }
};
int main()
{
    auto ptr = &X::func;
    auto func = *(int *)(&ptr);
    std::cout << func << '\n'; // 1
}
#endif
#endif