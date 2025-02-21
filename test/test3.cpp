#include <iostream>
#include <map>
#include <string>

void demo_v1()
{
    // 创建一个multimap
    std::multimap<int, std::string> mmap;

    // 插入元素
    mmap.insert({1, "apple"});
    mmap.insert({2, "banana"});
    mmap.insert({1, "orange"});
    mmap.insert({3, "grape"});

    // 查找并显示特定键的所有元素
    std::cout << "Elements with key 1:" << std::endl;
    auto range = mmap.equal_range(1);
    for (auto it = range.first; it != range.second; ++it)
    {
        std::cout << it->first << " => " << it->second << std::endl;
    }

    // 遍历所有元素
    std::cout << "\nAll elements:" << std::endl;
    for (const auto &pair : mmap)
    {
        std::cout << pair.first << " => " << pair.second << std::endl;
    }

    // 查找特定键
    auto it = mmap.find(2);
    if (it != mmap.end())
    {
        std::cout << "\nFound element with key 2: " << it->second << std::endl;
    }

    // 删除元素
    mmap.erase(1); // 删除所有键为1的元素

    // 显示删除后的元素
    std::cout << "\nAfter erasing key 1:" << std::endl;
    for (const auto &pair : mmap)
    {
        std::cout << pair.first << " => " << pair.second << std::endl;
    }
}

#define Operator

struct Person
{
    std::string name;
    int age;

#ifdef Operator
    // 重载 < 操作符
    bool operator<(const Person &other) const
    {
        if (age == other.age)
            return name < other.name;
        return age < other.age;
    }
#endif
};

struct ComparePerson
{
    bool operator()(const Person &a, const Person &b) const
    {
        if (a.age == b.age)
            return a.name < b.name;
        return a.age < b.age;
    }
};

void demo_v2()
{
    // 自定义类型作为键必须指定比较器
#ifdef Operator
    std::multimap<Person, std::string> mmap;
#else
    std::multimap<Person, std::string, ComparePerson> mmap;
#endif

    // 插入元素
    mmap.insert({{"Alice", 30}, "Engineer"});
    mmap.insert({{"Bob", 25}, "Doctor"});
    mmap.insert({{"Alice", 30}, "Manager"});
    mmap.insert({{"Charlie", 35}, "Teacher"});

    // 查找并显示特定键的所有元素
    Person key = {"Alice", 30};
    std::cout << "Elements with key (Alice, 30):" << std::endl;
    auto range = mmap.equal_range(key);
    for (auto it = range.first; it != range.second; ++it)
    {
        std::cout << it->first.name << " (" << it->first.age << ") => " << it->second << std::endl;
    }

    // 遍历所有元素
    std::cout << "\nAll elements:" << std::endl;
    for (const auto &pair : mmap)
    {
        std::cout << pair.first.name << " (" << pair.first.age << ") => " << pair.second << std::endl;
    }

    // 查找特定键
    auto it = mmap.find({"Bob", 25});
    if (it != mmap.end())
    {
        std::cout << "\nFound element with key (Bob, 25): " << it->second << std::endl;
    }

    // 删除元素
    mmap.erase(key); // 删除所有键为(Alice, 30)的元素

    // 显示删除后的元素
    std::cout << "\nAfter erasing key (Alice, 30):" << std::endl;
    for (const auto &pair : mmap)
    {
        std::cout << pair.first.name << " (" << pair.first.age << ") => " << pair.second << std::endl;
    }
}

int main()
{
    // demo_v1();
    demo_v2();
    return 0;
}