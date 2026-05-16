// 自定义分配器：可被 std::vector 采用，并按预期触发 allocate/deallocate。

#include <cstddef>
#include <limits>
#include <new>
#include <vector>

#include <gtest/gtest.h>

namespace {

struct {
    int allocate_hits = 0;
    int deallocate_hits = 0;
} instrument_stats;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

template <typename Typ>
class InstrumentedAllocator {
public:
    using value_type = Typ;

    InstrumentedAllocator() noexcept = default;

    template <typename Alter>
    explicit InstrumentedAllocator(const InstrumentedAllocator<Alter>& /*unused*/) noexcept {}

    [[nodiscard]] Typ* allocate(std::size_t count) {
        ++instrument_stats.allocate_hits;
        if (count > std::numeric_limits<std::size_t>::max() / sizeof(Typ)) {
            throw std::bad_array_new_length{};
        }
        return static_cast<Typ*>(::operator new(count * sizeof(Typ)));
    }

    void deallocate(Typ* ptr, std::size_t /*count*/) noexcept {
        ++instrument_stats.deallocate_hits;
        ::operator delete(ptr);
    }
};

template <typename A, typename B>
bool operator==(const InstrumentedAllocator<A>& /*left*/, const InstrumentedAllocator<B>& /*right*/) noexcept {
    return true;
}

}  // namespace

TEST(CustomAllocatorBasic, TracksHeapTrafficForGrowingVector) {
    instrument_stats.allocate_hits = 0;
    instrument_stats.deallocate_hits = 0;

    {
        std::vector<int, InstrumentedAllocator<int>> table{};
        constexpr int kTarget = 32;
        table.reserve(static_cast<std::size_t>(kTarget));
        for (int idx = 0; idx < kTarget; ++idx) {
            table.push_back(idx);
        }
        EXPECT_GE(table.size(), 16UL);
        EXPECT_GT(instrument_stats.allocate_hits, 0);
    }

    EXPECT_GT(instrument_stats.deallocate_hits, 0);
}
