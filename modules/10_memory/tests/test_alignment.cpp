// 验证 alignas 提升对齐，并可查询 alignof。

#include <atomic>
#include <cstdint>

#include <gtest/gtest.h>

namespace {

struct alignas(64) CacheLineAligned {
    std::atomic<int> counter;
};

}  // namespace

TEST(Alignment, AlignOfMatchesRequest) {
    EXPECT_GE(alignof(CacheLineAligned), 64U);
}

TEST(Alignment, ObjectAddressIsAligned) {
    constexpr int kLoops = 3;
    for (int i = 0; i < kLoops; ++i) {
        CacheLineAligned block{};
        auto address = reinterpret_cast<std::uintptr_t>(&block);  // NOLINT
        EXPECT_EQ(address % 64ULL, 0ULL);
        (void)i;
        (void)block.counter.load(std::memory_order_relaxed);
    }
}
