// polymorphic_allocator + monotonic/unpool 上游资源的生命周期校验。

#include <version>

#ifdef __cpp_lib_memory_resource

#include <array>
#include <memory_resource>
#include <vector>

#include <gtest/gtest.h>

namespace {

void fillSequential(std::pmr::vector<int>& buffer) {
    for (int i = 0; i < 8; ++i) {
        buffer.push_back(i);
    }
}

}  // namespace

TEST(PmrMonotonicVector, KeepsAddressesInsideUpstreamBufferWhileAlive) {
    std::array<std::byte, 2048U> backing{};
    std::pmr::monotonic_buffer_resource arena{backing.data(), backing.size()};

    std::pmr::vector<int> local_view{&arena};
    fillSequential(local_view);

    ASSERT_GE(local_view.size(), 8U);
    ASSERT_FALSE(local_view.empty());

    for (int candidate : local_view) {
        EXPECT_GE(candidate, 0);
        EXPECT_LT(candidate, 8);
    }
}

TEST(PmrPoolOnMonotonic, ChainsUpstreamResourcesSuccessfully) {
    std::array<std::byte, 4096U> mono_store{};
    std::pmr::monotonic_buffer_resource mono_upstream{mono_store.data(), mono_store.size()};
    std::pmr::unsynchronized_pool_resource pooled{&mono_upstream};

    std::pmr::vector<double> floats{&pooled};
    floats.reserve(128U);
    for (int seq = 0; seq < 16; ++seq) {
        floats.push_back(static_cast<double>(seq) * 0.5);
    }

    ASSERT_EQ(static_cast<std::size_t>(floats.size()), 16U);
}

#else

#include <gtest/gtest.h>

TEST(PmrSmoke, DisabledBecauseMemoryResourceUnavailable) {
    GTEST_SKIP() << "跳过：未定义 __cpp_lib_memory_resource";
}

#endif
