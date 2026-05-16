// 类域 operator new / delete：包含常规分配、sized-delete（C++14）与过对齐分配。
// 注意：此处刻意不用全局替换，以免干扰本进程内其他库（例如测试框架）的分配路径。

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>

#ifdef _WIN32
#include <malloc.h>
#endif

namespace {

// aligned_alloc（POSIX）要求 size 为 alignment 的倍数；此处向上取整以免 UB。
std::size_t paddedAllocationBytes(std::size_t size_bytes, std::size_t alignment) {
    if (alignment == 0U) {
        return size_bytes;
    }
    std::size_t const remainder = size_bytes % alignment;
    if (remainder == 0U) {
        return size_bytes;
    }
    return size_bytes + (alignment - remainder);
}

#ifdef _WIN32

void* allocateAligned(std::size_t alignment, std::size_t size_bytes) {
    std::size_t const bytes = paddedAllocationBytes(size_bytes, alignment);
    void* block = _aligned_malloc(bytes, alignment);  // NOLINT(cppcoreguidelines-no-malloc)
    if (block == nullptr) {
        throw std::bad_alloc{};
    }
    return block;
}

void releaseAligned(void* block) noexcept {
    _aligned_free(block);  // NOLINT(cppcoreguidelines-no-malloc)
}

#else

void* allocateAligned(std::size_t alignment, std::size_t size_bytes) {
    std::size_t const bytes = paddedAllocationBytes(size_bytes, alignment);
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory) — POSIX
    // aligned_alloc 演示路径
    void* block = ::aligned_alloc(alignment, bytes);
    if (block == nullptr) {
        throw std::bad_alloc{};
    }
    return block;
}

void releaseAligned(void* block) noexcept {
    std::free(block);  // NOLINT(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
}

#endif

// 文件内计数：演示“确实走到了自定义分配路径”。
struct {
    int new_calls = 0;
    int delete_calls = 0;
    int sized_delete_calls = 0;
    int aligned_new_calls = 0;
    int aligned_delete_calls = 0;
} hook_counts;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// sized-delete（C++14）单独用小类型演示：若在与「过对齐 operator new」同类上并存，
// 开启 sized deallocation 的实现可能把对齐对象的释放误绑定到 sized plain delete，
// 进而用 std::free 对应非对齐分配路径，造成堆不匹配。
struct SizedDeleteSample {
    int marker{};

    // NOLINTBEGIN(cppcoreguidelines-no-malloc, hicpp-no-malloc, cppcoreguidelines-owning-memory)
    static void* operator new(std::size_t count) {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.new_calls;
        void* p = std::malloc(count);  // NOLINT(cppcoreguidelines-no-malloc)
        if (p == nullptr) {
            throw std::bad_alloc{};
        }
        return p;
    }

    static void operator delete(void* ptr) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.delete_calls;
        std::free(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
    }

    static void operator delete(void* ptr,
                                std::size_t /*sz*/) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.sized_delete_calls;
        ++hook_counts.delete_calls;
        std::free(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
    }
    // NOLINTEND(cppcoreguidelines-no-malloc, hicpp-no-malloc, cppcoreguidelines-owning-memory)
};

struct Tracked {
    int payload{};

    static void teardownAligned(Tracked* ptr, std::align_val_t boundary) noexcept {
        ptr->~Tracked();
        operator delete(static_cast<void*>(ptr), boundary);
    }

    // NOLINTBEGIN(cppcoreguidelines-no-malloc, hicpp-no-malloc, cppcoreguidelines-owning-memory)
    static void* operator new(std::size_t count) {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.new_calls;
        void* p = std::malloc(count);  // NOLINT(cppcoreguidelines-no-malloc)
        if (p == nullptr) {
            throw std::bad_alloc{};
        }
        return p;
    }

    static void operator delete(void* ptr) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.delete_calls;
        std::free(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
    }

    // 此类承担过对齐分配；为避免误绑定 sized plain delete，这里不显式声明 operator delete(void*,
    // size_t)。

    static void* operator new(std::size_t count,
                              std::align_val_t al) {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.aligned_new_calls;
        return allocateAligned(static_cast<std::size_t>(al), count);
    }

    static void operator delete(
        void* ptr, std::align_val_t /*al*/) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.aligned_delete_calls;
        releaseAligned(ptr);
    }

    static void operator delete(
        void* ptr, std::size_t /*sz*/,
        std::align_val_t /*al*/) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++hook_counts.aligned_delete_calls;
        releaseAligned(ptr);
    }
    // NOLINTEND(cppcoreguidelines-no-malloc, hicpp-no-malloc, cppcoreguidelines-owning-memory)
};

void printCounters() {
    std::cout << "new: " << hook_counts.new_calls << " delete: " << hook_counts.delete_calls
              << " sized_delete: " << hook_counts.sized_delete_calls
              << " aligned_new: " << hook_counts.aligned_new_calls
              << " aligned_delete: " << hook_counts.aligned_delete_calls << '\n';
}

}  // namespace

int main() {
    auto* sized_probe = new SizedDeleteSample{-1};  // NOLINT(cppcoreguidelines-owning-memory)
    delete sized_probe;                             // NOLINT(cppcoreguidelines-owning-memory)

    auto* plain = new Tracked{42};  // NOLINT(cppcoreguidelines-owning-memory)
    delete plain;                   // NOLINT(cppcoreguidelines-owning-memory)

    // MSVC 将 operator delete(void*, align_val_t) 同时视作 placement deallocation，
    // 触发 C2956 拒绝此写法。仅在 GCC/Clang 上演示。
#ifndef _MSC_VER
    constexpr std::size_t kOverAlign = 64U;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) — 演示 placement new（对齐分配）
    auto* big = new (std::align_val_t{kOverAlign}) Tracked{7};
    Tracked::teardownAligned(big, std::align_val_t{kOverAlign});
#endif

    printCounters();
    return 0;
}
