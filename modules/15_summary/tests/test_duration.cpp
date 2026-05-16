// duration 算术与 duration_cast 精度验证。

#include <chrono>
#include <numbers>

#include <gtest/gtest.h>

TEST(DurationArithmetic, AdditionSubtraction) {
    using namespace std::chrono_literals;

    auto mixed = 75ms + 250ms;
    EXPECT_EQ(mixed, 325ms);

    mixed -= 100ms;
    EXPECT_EQ(mixed, 225ms);

    mixed *= 2;
    EXPECT_EQ(mixed, 450ms);
}

TEST(DurationArithmetic, DurationCastTruncatesTowardZero) {
    using namespace std::chrono_literals;

    constexpr auto kPrecise = 3599ms + 800us;
    auto const sec =
        std::chrono::duration_cast<std::chrono::seconds>(kPrecise);
    EXPECT_EQ(sec.count(), 3);

    auto const millis_back =
        std::chrono::duration_cast<std::chrono::milliseconds>(sec);
    EXPECT_EQ(millis_back.count(), 3000);
}

TEST(DurationArithmetic, FloatingDurationKeepsFraction) {
    std::chrono::duration<double, std::ratio<1>> const pi_seconds{
        std::numbers::pi};
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(pi_seconds);
    EXPECT_GE(millis.count(), 3141);
    EXPECT_LE(millis.count(), 3142);
}
