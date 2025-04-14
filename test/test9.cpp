#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <any>

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

int main()
{
    test6();
    return 0;
}
