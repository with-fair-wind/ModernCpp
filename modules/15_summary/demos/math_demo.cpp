// <cmath> 常用数学函数与 ULP 概念演示。

#include <cmath>
#include <format>
#include <iostream>
#include <limits>
#include <numbers>

namespace {

void showTrigonometric() {
    std::cout << "--- 三角函数 ---\n";
    constexpr double kAngle = std::numbers::pi / 6.0;
    std::cout << std::format("  sin(π/6) = {:.6f}  (期望 0.5)\n", std::sin(kAngle));
    std::cout << std::format("  cos(π/6) = {:.6f}  (期望 ≈0.866)\n", std::cos(kAngle));
    std::cout << std::format("  atan2(1, 1) = {:.6f}  (期望 π/4 ≈ {:.6f})\n", std::atan2(1.0, 1.0),
                             std::numbers::pi / 4.0);
}

void showExponentialLog() {
    std::cout << "\n--- 指数 / 对数 ---\n";
    // NOLINTNEXTLINE(modernize-use-std-numbers)
    std::cout << std::format("  exp(1)    = {:.6f}  (期望 e ≈ {:.6f})\n", std::exp(1.0),
                             std::numbers::e);
    std::cout << std::format("  expm1(0)  = {:.6e}  (精确 0)\n", std::expm1(0.0));
    std::cout << std::format("  log1p(0)  = {:.6e}  (精确 0)\n", std::log1p(0.0));
    std::cout << std::format("  logb(1024)= {:.1f}  (期望 10)\n", std::logb(1024.0));
}

void showRounding() {
    std::cout << "\n--- 舍入 ---\n";
    constexpr double kVal = 2.7;
    std::cout << std::format("  ceil({})  = {}\n", kVal, std::ceil(kVal));
    std::cout << std::format("  floor({}) = {}\n", kVal, std::floor(kVal));
    std::cout << std::format("  trunc({}) = {}\n", kVal, std::trunc(kVal));
    std::cout << std::format("  round({}) = {}\n", kVal, std::round(kVal));

    constexpr double kNeg = -2.7;
    std::cout << std::format("  trunc({}) = {}  (朝零截断)\n", kNeg, std::trunc(kNeg));
}

void showFma() {
    std::cout << "\n--- fma（融合乘加，精度优于 a*b+c）---\n";
    constexpr double kA = 1e15;
    constexpr double kB = 1.0000000000000002;
    constexpr double kC = -1e15;
    double const naive = (kA * kB) + kC;
    double const fused = std::fma(kA, kB, kC);
    std::cout << std::format("  a*b+c (朴素) = {:.6f}\n", naive);
    std::cout << std::format("  fma(a,b,c)   = {:.6f}  (通常更精确)\n", fused);
}

void showUlp() {
    std::cout << "\n--- ULP（最后一位单元）---\n";
    constexpr float kOne = 1.0F;
    float const ulp_one = std::nextafter(kOne, std::numeric_limits<float>::max()) - kOne;
    std::cout << std::format("  ULP(1.0f) = {:.10e}  (期望 2^-23 ≈ {:.10e})\n",
                             static_cast<double>(ulp_one), std::pow(2.0, -23.0));

    constexpr double kLarge = 1024.0;
    double const ulp_large = std::nextafter(kLarge, std::numeric_limits<double>::max()) - kLarge;
    std::cout << std::format("  ULP(1024.0) = {:.10e}\n", ulp_large);
}

void showClassification() {
    std::cout << "\n--- 浮点分类 ---\n";
    auto const inf = std::numeric_limits<double>::infinity();
    auto const nan = std::numeric_limits<double>::quiet_NaN();
    std::cout << std::format("  isinf(inf)       = {}\n", std::isinf(inf));
    std::cout << std::format("  isnan(NaN)       = {}\n", std::isnan(nan));
    std::cout << std::format("  isnormal(1.0)    = {}\n", std::isnormal(1.0));
    std::cout << std::format("  isnormal(1e-310) = {}\n", std::isnormal(1e-310));
}

void showSpecialFunctions() {
    std::cout << "\n--- 特殊函数 ---\n";
    std::cout << std::format("  erf(1.0)   = {:.6f}  (≈0.8427)\n", std::erf(1.0));
    std::cout << std::format("  tgamma(5)  = {:.1f}  (即 4! = 24)\n", std::tgamma(5.0));
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::cout << std::format("  lgamma(5)  = {:.6f}  (即 ln(24) ≈ {:.6f})\n", std::lgamma(5.0),
                             std::log(24.0));
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    showTrigonometric();
    showExponentialLog();
    showRounding();
    showFma();
    showUlp();
    showClassification();
    showSpecialFunctions();

    std::cout << "\n--- 建议：始终用 std::abs 而非 C 的 abs ---\n";
    std::cout << std::format("  std::abs(-3.14) = {:.2f}\n", std::abs(-3.14));

    return 0;
}
