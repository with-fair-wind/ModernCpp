#include <iostream>
#include <print>
#include <vector>

void demo_v1()
{
    std::vector<bool> vec{true, false, true, false, true, false};
    auto a = vec[0];
    std::vector<bool>::reference b = vec[0];
    a = false;
    std::cout << std::boolalpha << vec[0] << std::endl;
    std::cout << std::boolalpha << b << std::endl;
}

#if 0
#include <mdspan>
void demo_v2()
{
    std::vector v{1, 2, 3, 4, 5, 6};
    std::mdspan view{v.data(), 2, 3};
    for (size_t i = 0; i < view.extent(0); ++i)
    {
        for (size_t j = 0; j < view.extent(1); ++j)
            std::print("{}, ", view[i, j]);
        std::cout << std::endl;
    }
}
#endif

int main()
{
    demo_v1();
    return 0;
}