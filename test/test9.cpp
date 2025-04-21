#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <any>
#include <print>
#include <ranges>
#include <set>

void teset1()
{
    std::vector<int> vec{1, 2, 3, 4, 5, 6};
    vec.insert(vec.begin() + 1, 7);
}

struct Student
{
    std::string name;
    int age;
};

void test2()
{
    std::unordered_map<std::shared_ptr<Student>, int> map;
    std::shared_ptr<Student> p1(new Student{"kk", 1});
    std::shared_ptr<Student> p2(new Student{"kk", 1});
    map[p1] = 1;
    map[p2] = 2;
    std::shared_ptr<Student> p3(p2);
    std::cout << map[p3] << std::endl;

    std::unordered_set<Student *> set;
    Student s3{"1", 1};
    Student s4{"1", 1};
    set.insert(&s3);
    set.insert(&s4);
    std::cout << set.size() << std::endl;
}

void test3()
{
    auto hash = [](const Student &s)
    {
        return std::hash<std::string>()(s.name) ^ (std::hash<int>()(s.age) << 1);
    };

    auto equal = [](const Student &s1, const Student &s2)
    {
        return s1.name == s2.name && s1.age == s2.age;
    };
    std::unordered_set<Student, decltype(hash), decltype(equal)> set;
}

template <typename Func, typename... Args>
void test4(Func &&func, Args &&...args)
{
    std::cout << "test4" << std::endl;
    func(std::forward<Args>(args)...);
}

void test5()
{
    int a = 10;
    int b = 20;

    int &c = b;
    std::reference_wrapper<int> ref = a;       // ref 引用 a
    std::cout << "ref = " << ref << std::endl; // 输出 10

    ref = b;                                   // ref 现在引用 b
    std::cout << "ref = " << ref << std::endl; // 输出 20

    // 修改 ref 会修改 b（现在引用的是 b）
    ref.get() = 99;
    std::cout << "b = " << b << std::endl; // 输出 99

    ref = c;
    std::cout << "ref = " << ref << std::endl;
    ref.get() = 100;
    std::cout << "c = " << c << std::endl;
    std::cout << "b = " << b << std::endl; // 输出 99
}

void test6()
{
    static_assert(sizeof(long) == 4, "错误, 不是32位平台...");
    std::cout << sizeof(long) << std::endl;
}

void test7()
{
    std::vector v{1, 2, 3, 4, 5, 6};
    for (auto it = v.begin(); it != v.end();)
    {
        if (*it == 3)
            it = v.erase(it); // 删除元素后，返回下一个迭代器
        else
            ++it; // 继续迭代
    }
}

void test8()
{
    std::map<int, int> map;
    auto ret = map.insert({1, 1});
    map.lower_bound(1); // 返回第一个大于等于 1 的元素的迭代器
    map.upper_bound(1); // 返回第一个大于 1 的元素的迭代器
}

struct A
{
};

struct B : A
{
};

void f(A a)
{
    std::cout << "A" << std::endl;
}

void test9()
{
    B b;
    f(b); // 调用 A::f(A a)，会发生 slicing
}

struct X
{
    int a = 0;
    ~X()
    {
        std::cout << "X destructor" << std::endl;
    }
};

void test10()
{
    std::map<int, X> map;
    map.emplace(0, X{1}); // 直接在 map 中构造 X 对象
    map[0] = X{2};        // 替换 map[0] 的值
}

#include <ranges>

void test11()
{
    namespace stdv = std::views;
    for (const auto oddNum : stdv::iota(1, 10) | stdv::filter([](int num)
                                                              { return num % 2 == 1; }) |
                                 stdv::take(3))
        std::cout << oddNum << ' ';
}

namespace stdr = std::ranges;
namespace stdv = std::views;

void test12()
{
    std::set<std::string, std::less<>> set;
}

void test13()
{
    std::map<int, double> m{{1, 2.0}, {3, 4.0}};
    // 如果是auto&：由于map的key是const的，key的类型为const int&
    for (auto key : m | stdv::keys)
        // std::cout << key << " "; // 修复 std::print 调用错误，改为 std::cout
        std::print("{} ", key);
}

int main()
{
    test13();
    return 0;
}
