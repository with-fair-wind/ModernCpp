// C++20 chrono 日历类型的构造与算术验证。

#include <chrono>

#include <gtest/gtest.h>

TEST(CalendarTypes, ConstructValidateAndShift) {
#if defined(__cpp_lib_chrono) && (__cpp_lib_chrono >= 201907L)
    using namespace std::chrono;

    auto const ymd = 2023y / February / 28d;
    ASSERT_TRUE(ymd.ok());

    auto const invalid = year_month_day{2023y, February, 29d};
    EXPECT_FALSE(invalid.ok());

    auto const shifted = ymd + months{1};
    ASSERT_TRUE(shifted.ok());
    EXPECT_EQ(static_cast<unsigned>(shifted.month()),
              static_cast<unsigned>(March));
    EXPECT_EQ(static_cast<unsigned>(shifted.day()), 28U);

    auto const last_day = 2024y / February / last;
    ASSERT_TRUE(last_day.ok());
    EXPECT_EQ(static_cast<unsigned>(last_day.day()), 29U);

    weekday const wd{sys_days{2024y / January / 1d}};
    EXPECT_GE(wd.iso_encoding(), 1U);
    EXPECT_LE(wd.iso_encoding(), 7U);
#else
    GTEST_SKIP() << "未启用 C++20 chrono 日历（__cpp_lib_chrono）";
#endif
}
