#include <cstdint>
#include <iostream>
#include <type_traits>
#include <utility>

#define Enumeration
#ifdef Enumeration
enum class Day : std::uint8_t
{
    Monday,
    Tuesday,
    Wednesday = 10,
    Thursday,
    Friday,
    Saturday = 20,
    Sunday
};

void demo_1()
{
    Day day{Day::Thursday};
    using DayType = std::underlying_type_t<Day>;
    // using DayType = std::underlying_type<Day>::type;
    DayType data = std::to_underlying<Day>(day); // C++23
    std::cout << std::dec << static_cast<int>(data) << std::endl;
}

enum access_t
{
    read = 1,
    write = 2,
    exec = 4
}; // range: 0~7

enum foo
{
    a = 0,
    b = UINT_MAX
}; // range[0, UINT_MAX]

void demo_2()
{
    access_t rwe = static_cast<access_t>(7);
    access_t x = static_cast<access_t>(8.0); // UB
    access_t y = static_cast<access_t>(8);   // UB

    std::cout << "x: " << x << ", y: " << y << std::endl;

    // foo x = foo(-1); // UB
}

int main()
{
    demo_2();
}
#endif