#include <iostream>

// noexcept 说明符
void f1() noexcept
{
    std::cout << "66\n";
}

// noexcept 运算符
void f2() noexcept(true)
{
    std::cout << "66\n";
}

// noexcept 运算符
void f3() noexcept(false)
{
    std::cout << "66\n";
}

void f4()
{
    std::cout << "66\n";
}

void f5() noexcept(noexcept(f3()))
{
    std::cout << "66\n";
}

int main()
{
    // 不求值表达式 -> 潜在求值表达式
    // https://zh.cppreference.com/w/cpp/language/expressions#.E6.BD.9C.E5.9C.A8.E6.B1.82.E5.80.BC.E8.A1.A8.E8.BE.BE.E5.BC.8F
    // 不是调用，不求值表达式
    std::cout << std::boolalpha << noexcept(f1()) << '\n'; // true
    std::cout << std::boolalpha << noexcept(f2()) << '\n'; // true
    std::cout << std::boolalpha << noexcept(f3()) << '\n'; // false
    std::cout << std::boolalpha << noexcept(f4()) << '\n'; // false
    std::cout << std::boolalpha << noexcept(f5()) << '\n'; // false

    // C++17 以后 noexcept 是类型的一部分，但不能作为重载的区分
    auto p = f1;

    // eg: 不求值表达式
    std::size_t n = sizeof(std::cout << "44\n");
    std::cout << n << std::endl;
    int i = 1;
    noexcept(++i);
    std::cout << i << std::endl;
}