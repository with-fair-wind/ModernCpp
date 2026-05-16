// time_point 与各时钟的基本特性（steady_clock 单调性等）。

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

TEST(TimePointBasics, SteadyClockAdvancesAfterSleep) {
    using clock = std::chrono::steady_clock;
    auto const start = clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    auto const finish = clock::now();
    EXPECT_GT(finish, start);
    auto const elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    EXPECT_GE(elapsed, 10);
}

TEST(TimePointBasics, SystemClockEpochMsNonNegative) {
    auto const now = std::chrono::system_clock::now();
    auto const ms_since_epoch =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    EXPECT_GT(ms_since_epoch, 0);
}

TEST(TimePointBasics, HighResolutionClockProducesTimePoint) {
    auto const a = std::chrono::high_resolution_clock::now();
    auto const b = std::chrono::high_resolution_clock::now();
    EXPECT_GE(b.time_since_epoch().count(), a.time_since_epoch().count());
}
