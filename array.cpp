#include <iostream>

void example_1()
{
    std::cout << std::boolalpha << std::is_same_v<int[5], int[5]> << std::endl; // true
    std::cout << std::boolalpha << std::is_same_v<int[4], int[5]> << std::endl; // false
}

void example_2()
{
    const char array[10]{};
    using T = decltype(array[0]);
    using T = decltype(("***"));
}

int main()
{
    example_1();
}