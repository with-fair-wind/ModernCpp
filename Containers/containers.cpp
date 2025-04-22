#include <iostream>
#include <print>
#include <vector>
#include <iterator>
#include <span>
#include <list>
#include <cassert>
#include <algorithm>
#include <forward_list>
#include <stack>

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

void printInfo(std::span<int> span)
{
    auto size = span.size();
    for (size_t i = 0; i < size / 2; ++i)
        span[i] *= 2;
    for (size_t i = size / 2; i < size; ++i)
        span[i] *= 3;
    for (auto it = span.begin(); it != span.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
}

void demo_v6()
{
    std::vector<int> a{1, 2, 3, 4, 5};
    std::array<int, 4> b{1, 2, 3, 4};
    int c[]{1, 2, 3};
    printInfo(a);
    printInfo(b);
    printInfo(c);
}

void demo_v7()
{
    std::vector<int> src = {2, 7, 1};
    std::list<int> dst = {3, 1, 4};
#ifdef __cpp_lib_containers_ranges
    dst.assign_range(src);
#else
    dst.assign(src.cbegin(), src.cend());
#endif
    assert(std::ranges::equal(src, dst));
}

template <typename T>
    requires requires(T val) { std::cout << val; }
void printList(const std::string str, const std::list<T> list)
{

    std::cout << str;
    for (const auto &val : list)
        std::cout << val << " ";
    std::cout << "\n";
}

void demo_v8()
{
    std::list<int> list1{1, 2, 3, 4, 5};
    std::list<int> list2{10, 80, 30, 40, 50};
    auto it = list1.begin();
    std::advance(it, 2);
    list1.splice(it, list2);
    printList("list1: ", list1);
    printList("list2: ", list2);

    list2.splice(list2.begin(), list1, it, list1.end());
    printList("list1: ", list1);
    printList("list2: ", list2);
}

void demo_v9()
{
    std::forward_list<int> a = {100, 200};
    std::forward_list<int> b = {10, 20, 30, 40, 50};

    auto before_first = std::next(b.before_begin(), 1); // 指向元素 10
    auto last = std::next(before_first, 4);             // 指向元素 50
    auto before_end_a = a.before_begin();
    while (std::next(before_end_a) != a.end())
        ++before_end_a;

    a.splice_after(before_end_a, b, before_first, last);

    std::cout << "a: ";
    for (int x : a)
        std::cout << x << ' ';
    std::cout << "\nb: ";
    for (int x : b)
        std::cout << x << ' ';
    std::cout << '\n';
}

void demo_v10()
{
    std::vector<int> vec;
    vec.reserve(10);                                      // 预留空间
    std::stack<int, std::vector<int>> s1(std::move(vec)); // 传递 vec 的所有权
    std::stack<int, std::vector<int>> s2{[]()
                                         {
                                             std::vector<int> vec;
                                             vec.reserve(100); // 预留空间
                                             return vec;
                                         }()};
}

int main()
{
    demo_v9();
    return 0;
}