// <cmath> 数学函数行为验证：舍入、fma 精度、ULP、分类。

#include <cmath>
#include <limits>
#include <numbers>

#include <gtest/gtest.h>

TEST(MathFunctions, TrigBasicIdentities) {
    constexpr double kTol = 1e-12;
    EXPECT_NEAR(std::sin(0.0), 0.0, kTol);
    EXPECT_NEAR(std::cos(0.0), 1.0, kTol);
    EXPECT_NEAR(std::sin(std::numbers::pi / 2.0), 1.0, kTol);
    EXPECT_NEAR(std::atan2(0.0, 1.0), 0.0, kTol);
    EXPECT_NEAR(std::atan2(1.0, 0.0), std::numbers::pi / 2.0, kTol);
}

TEST(MathFunctions, ExponentialAndLog) {
    constexpr double kTol = 1e-12;
    EXPECT_NEAR(std::exp(0.0), 1.0, kTol);
    EXPECT_NEAR(std::log(1.0), 0.0, kTol);
    EXPECT_NEAR(std::expm1(0.0), 0.0, kTol);
    EXPECT_NEAR(std::log1p(0.0), 0.0, kTol);
    EXPECT_NEAR(std::log2(8.0), 3.0, kTol);
}

TEST(MathFunctions, RoundingBehavior) {
    EXPECT_EQ(std::ceil(2.3), 3.0);
    EXPECT_EQ(std::floor(2.7), 2.0);
    EXPECT_EQ(std::trunc(2.9), 2.0);
    EXPECT_EQ(std::trunc(-2.9), -2.0);
    EXPECT_EQ(std::round(2.5), 3.0);
    EXPECT_EQ(std::round(-2.5), -3.0);
}

TEST(MathFunctions, FmaMorePrecise) {
    constexpr double kA = 1e15;
    constexpr double kB = 1.0000000000000002;
    constexpr double kC = -1e15;
    double const fused = std::fma(kA, kB, kC);
    EXPECT_GT(std::abs(fused), 0.0);
    EXPECT_LE(std::abs(fused), 1.0);
}

TEST(MathFunctions, UlpOfFloatOne) {
    constexpr float kOne = 1.0F;
    float const next = std::nextafter(kOne, std::numeric_limits<float>::max());
    float const ulp = next - kOne;
    float const expected = std::ldexp(1.0F, -23);
    EXPECT_FLOAT_EQ(ulp, expected);
}

TEST(MathFunctions, Classification) {
    EXPECT_TRUE(std::isinf(std::numeric_limits<double>::infinity()));
    EXPECT_TRUE(std::isnan(std::numeric_limits<double>::quiet_NaN()));
    EXPECT_TRUE(std::isnormal(1.0));
    EXPECT_FALSE(std::isnormal(0.0));
    EXPECT_TRUE(std::isfinite(42.0));
    EXPECT_FALSE(std::isfinite(std::numeric_limits<double>::infinity()));
}

TEST(MathFunctions, SpecialFunctions) {
    constexpr double kTol = 1e-6;
    EXPECT_NEAR(std::tgamma(5.0), 24.0, kTol);
    EXPECT_NEAR(std::lgamma(5.0), std::log(24.0), kTol);  // NOLINT(concurrency-mt-unsafe)
    EXPECT_GT(std::erf(1.0), 0.84);
    EXPECT_LT(std::erf(1.0), 0.85);
}

TEST(MathFunctions, HypotTriple) {
    constexpr double kTol = 1e-12;
    EXPECT_NEAR(std::hypot(3.0, 4.0), 5.0, kTol);
}
