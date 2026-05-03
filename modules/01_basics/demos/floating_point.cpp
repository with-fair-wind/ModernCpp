// 不离开标准库就能演示的 IEEE-754 怪异行为：
//   - NaN 与任何值（含自身）比较都为不等
//   - +0.0 == -0.0，但二者位模式不同
//   - 远离零点时精度丢失（float 在约 1e7 之后 "x + 1 == x"）
//   - std::bit_cast 不触发 UB 地查看位模式

#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

int main() {
    constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
    constexpr double kInf = std::numeric_limits<double>::infinity();

    std::cout << std::boolalpha
              << "NaN == NaN  = " << (kNaN == kNaN) << '\n'
              << "NaN != NaN  = " << (kNaN != kNaN) << '\n'
              << "NaN <  1.0  = " << (kNaN < 1.0) << '\n'
              << "isnan(NaN)  = " << std::isnan(kNaN) << '\n'
              << "Inf >  1e308 = " << (kInf > 1e308) << '\n';

    // 带符号零：== 认为相等，但位模式不同。
    constexpr float kPlusZero = 0.0F;
    constexpr float kMinusZero = -0.0F;
    std::cout << "+0.0f == -0.0f          : " << (kPlusZero == kMinusZero) << '\n';
    std::cout << std::hex
              << "bits(+0.0f)             : 0x"
              << std::bit_cast<std::uint32_t>(kPlusZero) << '\n'
              << "bits(-0.0f)             : 0x"
              << std::bit_cast<std::uint32_t>(kMinusZero) << std::dec << '\n';

    // 超过 2^24 后，float 的 ULP >= 1，加 1 直接被舍入掉。
    // 在 1e10 处 ULP 大约是 2^11 = 2048，加法被吞没。
    constexpr float kBig = 1.0e10F;
    std::cout << "1e10f + 1.0f == 1e10f   : " << (kBig + 1.0F == kBig) << '\n';

    return 0;
}
