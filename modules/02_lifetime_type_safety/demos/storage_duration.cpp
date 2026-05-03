// 模块 02 的小演示：存储期与生命周期打印。
//
// 演示静态存储期与自动存储期的对比，以及语言保证的构造顺序：
// 静态对象在 main() 之前完成初始化；自动对象按声明顺序在进入其
// 所属作用域时构造。

#include <iostream>
#include <string>
#include <utility>

namespace {

struct Tag {
    std::string name;
    explicit Tag(std::string n) : name(std::move(n)) {
        std::cout << "  ctor " << name << '\n';
    }
    Tag(Tag const&) = delete;
    Tag(Tag&&) = delete;
    Tag& operator=(Tag const&) = delete;
    Tag& operator=(Tag&&) = delete;
    ~Tag() {
        std::cout << "  dtor " << name << '\n';
    }
};

Tag const kGlobalStatic{"static"};

}  // namespace

int main() {
    std::cout << "main entered\n";
    Tag const local{"automatic"};
    std::cout << "main leaving\n";
    (void)kGlobalStatic;
    return 0;
}
