// <bit>（C++20）的常用辅助函数，外加用 std::bit_cast 查看浮点位模式。
//
// <bit> 里所有函数都要求无符号整数输入；对带符号 / 浮点值，
// std::bit_cast 可以在不引入 UB 的前提下重解释位模式
// （比通过指针 C 风格转型安全得多）。

#include <bit>
#include <cstdint>
#include <iostream>

int main() {
    constexpr std::uint32_t kX = 0b0000'1100U;  // = 12

    std::cout << "x                   = " << kX << '\n';
    std::cout << "has_single_bit(x)   = " << std::has_single_bit(kX) << '\n';
    std::cout << "bit_ceil(5u)        = " << std::bit_ceil(5U) << '\n';
    std::cout << "bit_width(5u)       = " << std::bit_width(5U) << '\n';
    std::cout << "countl_zero(x, u32) = " << std::countl_zero(kX) << '\n';
    std::cout << "countr_zero(x)      = " << std::countr_zero(kX) << '\n';
    std::cout << "popcount(0b1011u)   = " << std::popcount(0b1011U) << '\n';

    // 不触发 UB 地查看 IEEE-754 浮点的位模式。
    constexpr float kF = -0.0F;
    auto const bits = std::bit_cast<std::uint32_t>(kF);
    std::cout << "bits(-0.0f)         = 0x" << std::hex << bits << std::dec << '\n';

    return 0;
}
