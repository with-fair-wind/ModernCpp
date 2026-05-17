// 对象内存布局：sizeof / alignof、填充（padding）、空基类优化（EBO）、
// [[no_unique_address]] 对空成员类型的压缩。

#include <cstddef>
#include <iostream>
#include <type_traits>

namespace {

// 空类在独立对象中仍须占至少 1 字节，以便不同对象拥有相异地址。
struct EmptyTag {};

struct Padded {
    char c{};
    // 典型平台下 int 需要 4 字节对齐，char 后会产生 3 字节填充。
    int i{};
};

// 通过继承空基类，派生类可与首个数据成员共享“尾巴”空间，避免额外 +1。
struct EboDerived : EmptyTag {
    int value{};
};

struct NonEbo {
    EmptyTag tag{};
    int value{};
};

struct WithNoUnique {
    // 空成员类型被标记为可与其他子对象共享地址，从而不必额外插入填充。
    [[no_unique_address]] EmptyTag tag{};
    int value{};
};

void printLayout() {
    std::cout << "sizeof(EmptyTag)          : " << sizeof(EmptyTag) << '\n';
    std::cout << "alignof(EmptyTag)         : " << alignof(EmptyTag) << '\n';
    std::cout << "sizeof(Padded)            : " << sizeof(Padded) << '\n';
    std::cout << "offsetof(Padded, c)       : " << offsetof(Padded, c) << '\n';
    std::cout << "offsetof(Padded, i)       : " << offsetof(Padded, i) << '\n';
    std::cout << "sizeof(EboDerived)        : " << sizeof(EboDerived) << "  (EBO)\n";
    std::cout << "sizeof(NonEbo)            : " << sizeof(NonEbo) << "  (无 EBO)\n";
    std::cout << "sizeof(WithNoUnique)      : " << sizeof(WithNoUnique)
              << "  ([[no_unique_address]])\n";
}

}  // namespace

int main() {
    printLayout();

    static_assert(sizeof(EmptyTag) >= 1);
    static_assert(sizeof(Padded) >= sizeof(char) + sizeof(int));
    static_assert(sizeof(EboDerived) <= sizeof(NonEbo));

    return 0;
}
