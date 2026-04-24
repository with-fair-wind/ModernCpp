// Fixed-width integers and the C++20 safe-integer-comparison helpers.
//
// The naive comparison `-1 < 1u` is `false` because the signed operand is
// promoted to unsigned first. `std::cmp_less` / `std::in_range` give you the
// mathematical answer without the surprise.

#include <cstdint>
#include <iostream>
#include <utility>

int main() {
    constexpr int          signed_value   = -1;
    constexpr unsigned int unsigned_value = 1;

    std::cout << std::boolalpha;
    std::cout << "(-1 < 1u) naive         : " << (signed_value < unsigned_value) << '\n';
    std::cout << "std::cmp_less(-1, 1u)   : "
              << std::cmp_less(signed_value, unsigned_value) << '\n';

    // in_range answers "can this value be represented as T?" safely.
    std::cout << "in_range<int8_t>(200)   : "
              << std::in_range<std::int8_t>(200) << '\n';
    std::cout << "in_range<int8_t>(100)   : "
              << std::in_range<std::int8_t>(100) << '\n';

    // Prefer fixed-width types at ABI / serialization boundaries.
    constexpr std::uint64_t mask = 0xFFFF'FFFF'0000'0000ULL;
    std::cout << "mask = " << std::hex << mask << std::dec << '\n';

    return 0;
}
