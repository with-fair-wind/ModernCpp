// 多态内存资源（PMR）：用 monotonic_buffer_resource 与线程无关池减少频繁堆调用。

#include <version>

#ifdef __cpp_lib_memory_resource

#include <array>
#include <cstddef>
#include <iostream>
#include <memory_resource>
#include <vector>

namespace {

void demonstrateMonotonic(std::byte* buffer, std::size_t size_bytes) {
    std::pmr::monotonic_buffer_resource arena{buffer, size_bytes};
    std::pmr::vector<int> ints{&arena};

    for (int i = 0; i < 8; ++i) {
        ints.push_back(i);
    }

    std::cout << "monotonic_vector size=" << ints.size() << " 首场地址距缓冲区起点偏移估计较小\n";
}

void demonstratePool(std::byte* mono_store, std::size_t mono_size_bytes) {
    std::pmr::monotonic_buffer_resource mono_upstream{mono_store, mono_size_bytes};
    std::pmr::unsynchronized_pool_resource pool{&mono_upstream};

    std::pmr::vector<double> doubles{&pool};
    std::pmr::vector<long> longs{&pool};

    doubles.resize(40, 3.14);
    longs.resize(12, -1);

    std::cout << "pool doubles=" << doubles.size() << " longs=" << longs.size()
              << "（同属一个池，按需取块）\n";
}

}  // namespace

int main() {
    std::array<std::byte, 4096U> mono_backing{};
    demonstrateMonotonic(mono_backing.data(), mono_backing.size());

    std::array<std::byte, 8192U> pool_backing{};
    demonstratePool(pool_backing.data(), pool_backing.size());

    return 0;
}

#else

#include <iostream>

int main() {
    std::cout << "当前编译器/标准库未启用 __cpp_lib_memory_resource，跳过 PMR 演示。\n";
    return 0;
}

#endif
