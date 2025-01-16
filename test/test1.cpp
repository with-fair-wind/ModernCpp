#include <iostream>

class A
{
public:
    int get() const { return _a; }

private:
    int _a = 10;
};

class B : public A
{
public:
    B() = default;
    B(int b) : A()
    {
        _b = b;
    }

private:
    int _b = 20;
};

int main()
{
    B b(100);
    std::cout << b.get() << std::endl;
    return 0;
}