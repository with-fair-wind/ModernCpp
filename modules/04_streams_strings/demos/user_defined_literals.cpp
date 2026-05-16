// 标准字面量（chrono、string、string_view）与自定义字面量运算符示例。
//
// 关键点：
//   `using namespace std::literals` 会引入多种 UDL，注意名字冲突；
//   自定义 `operator""_xxx` 应在命名空间内实现，避免污染全局。

#include <chrono>
#include <complex>
#include <iostream>
#include <string>
#include <string_view>

namespace {

// 千米 → 米的自定义字面量（演示用，非物理库）。
[[nodiscard]] constexpr long double operator""_km(long double km) noexcept {
    return km * 1000.0L;
}

[[nodiscard]] constexpr std::uint64_t operator""_ki(unsigned long long x) noexcept {
    return static_cast<std::uint64_t>(x) * 1024ULL;
}

void printSep() {
    std::cout << "---\n";
}

void demoStdStringLiterals() {
    using namespace std::string_literals;
    auto s = "hello"s;  // std::string
    std::cout << "operator\"\"s length=" << s.size() << " value=\"" << s << "\"\n";
}

void demoStringViewLiterals() {
    using namespace std::string_view_literals;
    constexpr std::string_view kSv = "immutable window"sv;
    std::cout << "operator\"\"sv size=" << kSv.size() << " first=" << kSv.front() << '\n';
}

void demoChronoLiterals() {
    using namespace std::chrono_literals;
    auto d = 90min + 1500ms;
    std::cout << "90min + 1500ms 总毫秒数量级: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << '\n';

    constexpr auto kOneDay = 24h;
    std::cout << "24h 包含 " << std::chrono::duration_cast<std::chrono::minutes>(kOneDay).count()
              << " 分钟\n";
}

void demoComplexLiterals() {
    using namespace std::complex_literals;
    auto z = 1.0 + 2.5i;  // std::complex<double>
    std::cout << "complex 实部=" << z.real() << " 虚部=" << z.imag() << '\n';
}

void demoCustomLiterals() {
    constexpr auto kMeters = 3.5_km;
    std::cout << "3.5_km → 米量级: " << static_cast<long double>(kMeters) << '\n';
    std::cout << "8_ki × 字节: " << 8_ki << '\n';
}

}  // namespace

int main() {
    demoStdStringLiterals();
    printSep();
    demoStringViewLiterals();
    printSep();
    demoChronoLiterals();
    printSep();
    demoComplexLiterals();
    printSep();
    demoCustomLiterals();
    std::cout << "\n自定义字面量能提升可读性，但团队需统一后缀命名与语义。\n";
    return 0;
}
