// Highlights from <bit> (C++20) plus std::bit_cast for float punning.
//
// All <bit> helpers require unsigned integer inputs. For signed / floating
// values, std::bit_cast reinterprets the bit pattern without undefined
// behaviour (unlike C-style casts through pointers).

#include <bit>
#include <cstdint>
#include <iostream>

int main() {
    constexpr std::uint32_t x = 0b0000'1100u;  // = 12

    std::cout << "x                   = " << x << '\n';
    std::cout << "has_single_bit(x)   = " << std::has_single_bit(x) << '\n';
    std::cout << "bit_ceil(5u)        = " << std::bit_ceil(5u) << '\n';
    std::cout << "bit_width(5u)       = " << std::bit_width(5u) << '\n';
    std::cout << "countl_zero(x, u32) = " << std::countl_zero(x) << '\n';
    std::cout << "countr_zero(x)      = " << std::countr_zero(x) << '\n';
    std::cout << "popcount(0b1011u)   = " << std::popcount(0b1011u) << '\n';

    // Inspect the IEEE-754 bit pattern of a float without UB.
    constexpr float f = -0.0f;
    auto const bits = std::bit_cast<std::uint32_t>(f);
    std::cout << "bits(-0.0f)         = 0x" << std::hex << bits << std::dec << '\n';

    return 0;
}
