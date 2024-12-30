#include <iostream>

// C++11 lambda 类类型 (无名的非联合(union)非聚合类 类型)

#define Version_2
#ifdef Version_1
// 重载了一个 operator() 的类 (const 修饰, 除非使用 mutable)
struct X : decltype([]
                    { std::cout << "666\n"; })
{
};

int main()
{
    std::cout << sizeof(X) << std::endl; // 1 空类
    auto p = [num = 1]() mutable
    { num++; std::cout << num << std::endl; };
    p();                                // 2
    std::cout << sizeof p << std::endl; // 4 有一个int成员，4字节
    X x;
    x();

    // 不捕获任何 lambda表达式编译生成的类中会自动包含一个转换函数, 用于转换为函数指针
    // https://cppinsights.io/s/1f2daa34
    // 只有在 lambda 表达式的捕获符列表为空且没有显式对象形参(C++23 起)时才定义这个用户定义转换函数
    void (*p_)(int) = [](int n)
    { std::cout << "lambda:" << n << std::endl; };

    // 泛型lambda
    auto f = [](auto a, auto b)
    { return a + b; };
    std::cout << f(1, 2) << std::endl; // 3

    auto g = []<typename T>(T a, T b)
    { return a + b; };                 // C++20
    std::cout << g(1, 2) << std::endl; // 3

    auto h = []<typename T>(T a, T b)
        requires std::is_integral_v<T>
    { return a + b; };                 // C++20
    std::cout << h(1, 2) << std::endl; // 3

    auto i = []
    { return 5; };
    constexpr int n = i();
    std::cout << n << std::endl; // 5
}

#elif defined(Version_2)
// 一元+是为了辅助推导，是为了创造合适的语境
// auto*的*可以去掉，没啥作用
auto *p = +[]
{ return 6; };

#endif