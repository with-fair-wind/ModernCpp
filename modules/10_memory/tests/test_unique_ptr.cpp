// std::unique_ptr：独占所有权语义、自定义删除器、工厂函数。

#include <memory>

#include <gtest/gtest.h>

namespace {

struct {
    int deleter_invokes = 0;
} unique_test_stats;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

struct CountDeleter {
    void operator()(int const* ptr) const noexcept {
        delete ptr;  // NOLINT(cppcoreguidelines-owning-memory)
        ++unique_test_stats.deleter_invokes;
    }
};

}  // namespace

TEST(UniquePtrBasic, OwnershipMovesAndResetsOriginal) {
    auto first = std::make_unique<double>(9.875);
    auto second = std::move(first);
    EXPECT_FALSE(first);
    ASSERT_TRUE(second);
    EXPECT_DOUBLE_EQ(*second, 9.875);
}

TEST(UniquePtrCustomDeleter, CustomDeleterRunsOnRelease) {
    unique_test_stats.deleter_invokes = 0;
    {
        std::unique_ptr<int, CountDeleter> owner{new int{1337}, CountDeleter{}};       // NOLINT(cppcoreguidelines-owning-memory)
        EXPECT_NE(owner.get(), nullptr);
    }
    EXPECT_EQ(unique_test_stats.deleter_invokes, 1);
}

TEST(UniquePtrFactories, MakeUniqueConstructsInsidePointer) {
    auto val = std::make_unique<long>(987'654LL);
    ASSERT_TRUE(val != nullptr);
    EXPECT_EQ(*val, 987'654LL);
}
