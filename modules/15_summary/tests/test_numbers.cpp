// C++20 `<numbers>` 数学常数验证：值与类型。

#include <cmath>
#include <numbers>
#include <type_traits>

#include <gtest/gtest.h>

TEST(NumbersConstants, PiIsAccurate) {
    constexpr double kTol = 1e-14;
    EXPECT_NEAR(std::numbers::pi, 3.14159265358979323846, kTol);
    EXPECT_NEAR(std::sin(std::numbers::pi), 0.0, 1e-15);
}

TEST(NumbersConstants, EulerNumber) {
    constexpr double kTol = 1e-14;
    EXPECT_NEAR(std::numbers::e, 2.71828182845904523536, kTol);
    EXPECT_NEAR(std::exp(1.0), std::numbers::e, kTol);
}

TEST(NumbersConstants, Sqrt2) {
    constexpr double kTol = 1e-14;
    EXPECT_NEAR(std::numbers::sqrt2 * std::numbers::sqrt2, 2.0, kTol);
}

TEST(NumbersConstants, Ln2AndLog2e) {
    constexpr double kTol = 1e-14;
    EXPECT_NEAR(std::numbers::ln2, std::log(2.0), kTol);
    EXPECT_NEAR(std::numbers::log2e, 1.0 / std::log(2.0), kTol);
}

TEST(NumbersConstants, GoldenRatio) {
    constexpr double kTol = 1e-14;
    double const phi = std::numbers::phi;
    EXPECT_NEAR(phi * phi, phi + 1.0, kTol);
}

TEST(NumbersConstants, FloatTemplateVariant) {
    static_assert(std::is_same_v<std::remove_cv_t<decltype(std::numbers::pi_v<float>)>, float>);
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(std::numbers::pi_v<long double>)>, long double>);
    constexpr float kPiF = std::numbers::pi_v<float>;
    EXPECT_NEAR(static_cast<double>(kPiF), 3.14159, 1e-4);
}
