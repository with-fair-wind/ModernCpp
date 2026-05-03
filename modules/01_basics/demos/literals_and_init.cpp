// C++14/17/20 的字面量与统一初始化要点。
//
// 涵盖：
//   - 数字分隔符（C++14）：1'000'000、0xFF'AA、0b1010'0011
//   - 二进制字面量（C++14）：0b...
//   - 花括号初始化在编译期拒绝窄化转换
//   - 聚合体的指定初始化（C++20）

#include <cstdint>
#include <iostream>

namespace {

struct Point {
    int x{};
    int y{};
    int z{};
};

}  // namespace

int main() {
    constexpr std::uint32_t kPattern = 0xFFAA'0011U;
    constexpr int kBigDecimal = 1'000'000;
    constexpr unsigned kBinary = 0b1010'0011U;

    std::cout << std::hex << "kPattern    = 0x" << kPattern << std::dec << '\n'
              << "kBigDecimal = " << kBigDecimal << '\n'
              << "kBinary     = " << kBinary << '\n';

    // 取消注释下面这行就会编译失败 —— 花括号禁止窄化转换：
    //   std::int16_t bad{kBigDecimal};   // error: narrowing
    std::int32_t const ok{kBigDecimal};
    std::cout << "ok          = " << ok << '\n';

    // 指定初始化：显式给成员命名以提升可读性。注意 C++ 要求指定符
    // 必须按声明顺序书写（与 C99 不同）。
    constexpr Point kP{.x = 1, .y = 2, .z = 3};
    std::cout << "p           = (" << kP.x << ',' << kP.y << ',' << kP.z << ")\n";

    // 聚合初始化也允许省略尾部成员，省略的部分会值初始化为 {}。
    constexpr Point kOrigin{};
    std::cout << "origin      = (" << kOrigin.x << ',' << kOrigin.y << ',' << kOrigin.z << ")\n";

    return 0;
}
