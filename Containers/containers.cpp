#include <iostream>
#include <print>
#include <vector>
#include <iterator>

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

void demo_v3()
{
    std::vector<int> vec(5);
    std::istream_iterator<int> it{std::cin};
    vec[0] = *it;
    // output with \n as the separator
    std::ostream_iterator<int> out{std::cout, "\n"};
    for (int i = 1; i < 5; ++i)
        vec[i] = *(++it);
    for (auto &ele : vec)
        *(out++) = ele;
}

void demo_v4()
{
    std::vector<int> vec(5);
    std::copy(std::istream_iterator<int>(std::cin), std::istream_iterator<int>(), vec.begin());
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, "\n"));
}

void demo_v5()
{
    std::vector<int> vec;
    std::copy(std::istream_iterator<int>{std::cin}, std::istream_iterator<int>(), std::back_insert_iterator{vec});
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, "\n"));
}

int main()
{
    demo_v5();
    return 0;
}