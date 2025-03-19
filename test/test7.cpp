#include <iostream>

struct T
{
    T() = default;
    T(const T &&) { std::cout << "move" << std::endl; }
    T &&operator=(const T &&) { std::cout << "operator move" << std::endl; }
};

T &&f()
{
    T t;
    return std::move(t);
}

int main()
{
    T &&t = f();
    return 0;
}