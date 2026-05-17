// 自定义分配器：演示满足 C++ Allocator 基本要求的最小实现，并接入 std::vector。

#include <cstddef>
#include <iostream>
#include <limits>
#include <new>
#include <vector>

namespace {

struct {
    int allocate_calls = 0;
    int deallocate_calls = 0;
} counting_stats;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

template <typename ValueType>
struct CountingAllocator {
    using value_type = ValueType;

    CountingAllocator() = default;

    template <typename U>
    explicit CountingAllocator(const CountingAllocator<U>& /*other*/) noexcept {}

    [[nodiscard]] ValueType* allocate(std::size_t n) {
        ++counting_stats.allocate_calls;
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(ValueType)) {
            throw std::bad_array_new_length{};
        }
        return static_cast<ValueType*>(::operator new(n * sizeof(ValueType)));
    }

    void deallocate(ValueType* p, std::size_t /*n*/) noexcept {
        ++counting_stats.deallocate_calls;
        ::operator delete(p);
    }
};

template <typename A, typename B>
bool operator==(const CountingAllocator<A>& /*a*/, const CountingAllocator<B>& /*b*/) noexcept {
    return true;
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    std::vector<int, CountingAllocator<int>> nums{};
    nums.reserve(16);
    for (int i = 0; i < 10; ++i) {
        nums.push_back(i);
    }

    std::cout << "vector 长度: " << nums.size() << '\n';
    std::cout << "allocate 次数累计: " << counting_stats.allocate_calls << '\n';
    std::cout << "deallocate 次数累计: " << counting_stats.deallocate_calls << '\n';

    return 0;
}
