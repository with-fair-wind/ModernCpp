#include <chrono>
#include <format>
#include <iostream>
// 用户自定义字面量
using namespace std::literals;
using namespace std::chrono_literals;

std::string operator""_r(const char *str, std::size_t size)
{
    return {str};
}

std::string operator""_i(std::size_t size)
{
    return std::to_string(size);
}

//"{} {}"_f(a, b) 等价于 std::format("{} {}", a, b)

namespace my
{
    struct A
    {
        constexpr A(const char *s) noexcept : str(s) {};
        const char *str;
    };

    namespace literals
    {
        template <A a>
        constexpr auto operator""_f()
        {
            return [=]<typename... Args>(Args &&...args)
            { return std::format(a.str, std::forward<Args>(args)...); };
        }
    };
};

using namespace my::literals;

int main()
{
    // 自定义
    std::cout << "123456"_r << "\n";
    // std::string
    std::cout << "kk"s << "\n";
    // C++17 std::string_view
    auto str = "ss"sv;
    auto str1 = 5s;
    auto str2 = 5ms;
    auto str3 = 5_i;

    // MSVC可以通过编译，gcc不行
    // std::cout << "{} ** {}"_f(5, 5.6) << std::endl;
}