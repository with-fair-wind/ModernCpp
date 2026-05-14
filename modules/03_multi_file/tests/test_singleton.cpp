// 单例测试：验证 Meyers' Singleton 跨 TU 的唯一性和状态共享。

#include <string>
#include <type_traits>

#include <gtest/gtest.h>

#include "singleton.h"

// 在另一个 TU 中定义
Logger* getSingletonAddrFromTu2();
void logFromTu2(std::string const& msg);

TEST(Singleton, SameInstanceAcrossTU) {
    // 无论从哪个 TU 获取，都是同一实例
    Logger* addr_here = &Logger::instance();
    Logger* addr_there = getSingletonAddrFromTu2();
    EXPECT_EQ(addr_here, addr_there);
}

TEST(Singleton, StateSharedAcrossTU) {
    Logger& logger = Logger::instance();

    // 重置状态
    logger.setPrefix("[TEST]");
    int initial_count = logger.logCount();

    // 从另一个 TU 记录日志
    logFromTu2("message from tu2");

    // 本 TU 能观察到计数增加
    EXPECT_EQ(logger.logCount(), initial_count + 1);
}

TEST(Singleton, PrefixMutationVisibleAcrossTU) {
    Logger::instance().setPrefix("[MODIFIED]");
    // 通过另一个 TU 获取的引用也能观察到修改
    EXPECT_EQ(getSingletonAddrFromTu2()->prefix(), "[MODIFIED]");
}

TEST(Singleton, DeletedCopyAndAssignment) {
    // Logger 不可拷贝
    static_assert(!std::is_copy_constructible_v<Logger>);
    static_assert(!std::is_copy_assignable_v<Logger>);
}
