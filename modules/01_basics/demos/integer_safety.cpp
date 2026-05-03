// 定宽整数类型与 C++20 的安全整数比较辅助。
//
// 朴素写法 `-1 < 1u` 会得到 `false`：带符号操作数会先被提升为无符号。
// std::cmp_less / std::in_range 给出数学上正确的答案，避免这种坑。

#include <cstdint>
#include <iostream>
#include <utility>

int main() {
    constexpr int kSignedValue = -1;
    constexpr unsigned int kUnsignedValue = 1;

    std::cout << std::boolalpha;
    // NOLINTNEXTLINE(clang-diagnostic-sign-conversion) - 故意为之：演示朴素写法的坑
    std::cout << "(-1 < 1u) naive         : " << (kSignedValue < kUnsignedValue) << '\n';
    std::cout << "std::cmp_less(-1, 1u)   : " << std::cmp_less(kSignedValue, kUnsignedValue)
              << '\n';

    // in_range 安全地回答 "这个值能用 T 表示吗？"
    std::cout << "in_range<int8_t>(200)   : " << std::in_range<std::int8_t>(200) << '\n';
    std::cout << "in_range<int8_t>(100)   : " << std::in_range<std::int8_t>(100) << '\n';

    // 在 ABI / 序列化边界上优先使用定宽整数类型。
    constexpr std::uint64_t kMask = 0xFFFF'FFFF'0000'0000ULL;
    std::cout << "mask = " << std::hex << kMask << std::dec << '\n';

    return 0;
}
