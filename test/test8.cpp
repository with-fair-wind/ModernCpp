#include <cmath>
#include <compare>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <vector>

namespace fs = std::filesystem;

int test1()
{
    try
    {
        fs::path currentPath = fs::current_path();
        std::cout << "当前工作目录: " << currentPath << std::endl;
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "无法获取当前工作目录: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

struct Base
{
    virtual void process(int) { std::cout << "Base::process(int)\n"; }
    virtual void process(double) { std::cout << "Base::process(double)\n"; }
};

struct Derived : public Base
{
    virtual void process(int) override { std::cout << "Derived::process(int)\n"; }
    // 此时 Base::process(double) 在 Derived 中被隐藏了！
    using Base::process;
};

void test2()
{
    Base *p = new Derived;
    p->process(5);
    p->process(5.);
    Derived d;
    d.process(0.);
}

void test3()
{
    std::vector<int> vec{1, 2, 3, 4, 5, 6};

    // 过滤偶数后对每个元素进行平方，再取前3个元素
    auto view = vec | std::views::filter([](int n)
                                         { return n % 2 == 0; }) |
                std::views::transform([](int n)
                                      { return n * n; }) |
                std::views::take(3);

    for (int n : view)
        std::cout << n << " "; // 输出：4 16
    std::cout << std::endl;
}

void test4()
{
    // double a = NAN;
    // double b = 5.0;

    double a = -0.;
    double b = 0.;
    auto result = a <=> b;

    if (std::is_lt(result))
        std::cout << "a is less than b" << std::endl;
    else if (std::is_eq(result))
        std::cout << "a is equal to b" << std::endl;
    else if (std::is_gt(result))
        std::cout << "a is greater than b" << std::endl;
    else
        std::cout << "a and b are unordered" << std::endl;

    std::vector v1{1, 2}, v2{1, 2, 3};
    auto r = v1 <=> v2;
}

class MDArr
{
public:
    MDArr(int initRow, int initCol) : row{initRow}, col{initCol} {}
    float &operator[](int i, int j) { return arr[i * row + j]; }
    const float &operator[](int i, int j) const { return arr[i * row + j]; }

private:
    int row, col;
    float arr[100];
};

void test5()
{
    int rows = 2, cols = 3;
    MDArr arr{rows, cols};
    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < cols; ++col)
            arr[row, col] = row * row + col;
    std::cout << arr[1, 2];
}

int main()
{
    test5();
    return 0;
}
