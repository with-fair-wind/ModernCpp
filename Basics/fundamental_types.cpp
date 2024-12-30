/*
Fundamental types
    Integer
    Floating points
*/

#include <bit>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <utility>

#define Version_5
#ifdef Version_1
int main()
{
    std::cout << std::boolalpha << std::cmp_greater_equal(-1, std::string::npos) << '\n';
    std::cout << std::boolalpha << (-1 >= std::string::npos) << '\n';
}
#elif defined(Version_2)
int main()
{
    unsigned short s1 = 0xff00, s2 = 0x100, s3 = 0xffff;
    if (s1 + s2 > s3)
        std::cout << "unexpected\n";
    unsigned int i1 = 0xffff0000, i2 = 0x10000, i3 = 0xffffffff;
    if (i1 + i2 > i3)
        std::cout << "unexpected, too!\n";
    return 0;
}
#elif defined(Version_3)
int main()
{
    unsigned int a = 0x0011, b = std::bit_ceil(a); // 010001 -> 100000

    std::cout << std::format("a's bit_width: {} b: {} b's bit_width: {}\n", std::bit_width(a), b, std::bit_width(b));
    return 0;
}
#elif defined(Version_4)
unsigned int addOneInExp(unsigned int a)
{
    [[maybe_unused]] const int fracBit = 23, expBits = 8, fracMask = (1 << fracBit) - 1,
                               expMask = ((1 << expBits) - 1) << fracBit;
    unsigned int exp = a & expMask;
    unsigned int clearNum = (a & (~expMask));
    return clearNum | (exp + (1 << fracBit));
}

int main()
{
    float f1 = 2.0;
    unsigned int i1 = std::bit_cast<unsigned int>(f1);
    unsigned int i2 = addOneInExp(i1);
    float f2 = std::bit_cast<float>(i2);
    std::cout << std::format("f1 = {}, i1 = {:032b}, i2 = {:032b}, f2 = {}\n", f1, i1, i2, f2);
}
#elif defined(Version_5)
[[noreturn]] void fatal();
[[noreturn]] void fatal()
{
    std::cout << "fatal error\n";
    std::exit(1);
}

[[nodiscard]] int test(bool flag)
// [[deprecated("reson")]] int test(bool flag)
{
    if (flag)
        return 0;
    fatal();
}

int main()
{
    // test(true);
    // test(false);
    [[maybe_unused]] int res = test(false);
}

#endif
