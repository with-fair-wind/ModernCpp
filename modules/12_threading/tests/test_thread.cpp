// 模块 12：std::thread 基本行为（joinable、get_id、hardware_concurrency）。

#include <latch>
#include <optional>
#include <thread>

#include <gtest/gtest.h>

TEST(Thread, JoinableTransitions) {
    std::thread t([] {});
    EXPECT_TRUE(t.joinable());
    t.join();
    EXPECT_FALSE(t.joinable());
}

TEST(Thread, ChildThreadHasDistinctId) {
    std::optional<std::thread::id> child_id;
    std::latch started{1};
    std::thread worker([&started, &child_id] {
        child_id = std::this_thread::get_id();
        started.count_down();
    });
    started.wait();
    ASSERT_TRUE(child_id.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): 已由上方 ASSERT_TRUE 保证有值
    EXPECT_NE(std::this_thread::get_id(), *child_id);
    worker.join();
}

TEST(Thread, HardwareConcurrency) {
    const unsigned int n = std::thread::hardware_concurrency();
    if (n == 0) {
        GTEST_SKIP() << "实现无法给出非零的 hardware_concurrency";
    }
    EXPECT_GE(n, 1U);
}
