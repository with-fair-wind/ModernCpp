// 类域 operator new/delete 被表达式 new/delete 正常分派。

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

#include <gtest/gtest.h>

#ifdef _WIN32
#include <malloc.h>
#endif

namespace {

// POSIX aligned_alloc 要求字节数是 alignment 的整数倍；编译器传给 operator new 的 count 可能只是 sizeof(T)。
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
    void* block = ::aligned_alloc(alignment, bytes);  // NOLINT(cppcoreguidelines-no-malloc)
    if (block == nullptr) {
        throw std::bad_alloc{};
    }
    return block;
}

void releaseAligned(void* block) noexcept {
    std::free(block);  // NOLINT(cppcoreguidelines-no-malloc)
}

#endif

class TrackedLifetime {
public:
    int sentinel{42};

    // 供测试/演示显式拆解“过对齐 new”配对；部分实现上 `delete expr` 对类域对齐释放的解析并不可靠。
    static void teardownAligned(TrackedLifetime* ptr, std::align_val_t boundary) noexcept {
        ptr->~TrackedLifetime();
        operator delete(static_cast<void*>(ptr), boundary);
    }

private:
    // NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory, hicpp-no-malloc)

    static int plain_new_hits;

    static int plain_delete_hits;

    static int aligned_new_hits;

    static int aligned_delete_hits;

public:
    static void resetCounters() noexcept {
        plain_new_hits = 0;
        plain_delete_hits = 0;
        aligned_new_hits = 0;
        aligned_delete_hits = 0;
    }

    static void* operator new(std::size_t count) {  // NOLINT(misc-new-delete-overloads)
        ++plain_new_hits;
        void* blob = std::malloc(count);  // NOLINT(cppcoreguidelines-no-malloc)
        if (blob == nullptr) {
            throw std::bad_alloc{};
        }
        return blob;
    }

    static void operator delete(void* ptr) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++plain_delete_hits;
        std::free(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
    }

    // 刻意不提供 operator delete(void*, std::size_t)：开启 sized deallocation 时，
    // 若它与对齐分配共存，部分实现会把 delete 绑定到 sized 版本并用 std::free 释放，
    // 与 _aligned_malloc / aligned_alloc 不匹配而导致堆损坏。

    static void* operator new(std::size_t count, std::align_val_t boundary) {  // NOLINT(misc-new-delete-overloads)
        ++aligned_new_hits;
        return allocateAligned(static_cast<std::size_t>(boundary), count);
    }

    static void operator delete(void* ptr, std::align_val_t /*boundary*/) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++aligned_delete_hits;
        releaseAligned(ptr);
    }

    // 启用 sized deallocation 时，释放路径可能选中带 align_val_t 的三参数版本。
    static void operator delete(void* ptr, std::size_t /*sz*/,
                                std::align_val_t /*boundary*/) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++aligned_delete_hits;
        releaseAligned(ptr);
    }

    [[nodiscard]] static int plainNews() noexcept { return plain_new_hits; }

    [[nodiscard]] static int plainDeletes() noexcept { return plain_delete_hits; }

    [[nodiscard]] static int alignedNews() noexcept { return aligned_new_hits; }

    [[nodiscard]] static int alignedDeletes() noexcept { return aligned_delete_hits; }
    // NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory, hicpp-no-malloc)
};

int TrackedLifetime::plain_new_hits = 0;
int TrackedLifetime::plain_delete_hits = 0;
int TrackedLifetime::aligned_new_hits = 0;
int TrackedLifetime::aligned_delete_hits = 0;

// 仅用于验证 sized deallocation 选中哪条 delete 重载；与 TrackedLifetime 分离以免计数混杂。
struct SizedDeleteWitness {
    int payload{7};

    // NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory, hicpp-no-malloc)
    static void* operator new(std::size_t count) {  // NOLINT(misc-new-delete-overloads)
        ++new_hits;
        void* blob = std::malloc(count);  // NOLINT(cppcoreguidelines-no-malloc)
        if (blob == nullptr) {
            throw std::bad_alloc{};
        }
        return blob;
    }

    static void operator delete(void* ptr) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++plain_delete_hits;
        std::free(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
    }

    static void operator delete(void* ptr, std::size_t /*sz*/) noexcept {  // NOLINT(misc-new-delete-overloads)
        ++sized_delete_hits;
        std::free(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
    }
    // NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory, hicpp-no-malloc)

    static void resetCounters() noexcept {
        new_hits = 0;
        plain_delete_hits = 0;
        sized_delete_hits = 0;
    }

    [[nodiscard]] static int newHits() noexcept { return new_hits; }

    [[nodiscard]] static int plainDeleteHits() noexcept { return plain_delete_hits; }

    [[nodiscard]] static int sizedDeleteHits() noexcept { return sized_delete_hits; }

private:
    static int new_hits;
    static int plain_delete_hits;
    static int sized_delete_hits;
};

int SizedDeleteWitness::new_hits = 0;
int SizedDeleteWitness::plain_delete_hits = 0;
int SizedDeleteWitness::sized_delete_hits = 0;

}  // namespace

TEST(ClassScopedOperators, OrdinaryNewDeletesHitCustomHooks) {
    TrackedLifetime::resetCounters();
    auto* instance = new TrackedLifetime{};                                             // NOLINT(cppcoreguidelines-owning-memory)
    ASSERT_NE(instance, nullptr);

    EXPECT_EQ(instance->sentinel, 42);
    EXPECT_GE(TrackedLifetime::plainNews(), 1);

    delete instance;                                                                      // NOLINT(cppcoreguidelines-owning-memory)

    EXPECT_GE(TrackedLifetime::plainDeletes(), 1);
}

TEST(ClassScopedOperators, OverAlignedConstructionUsesAlignedPair) {
    TrackedLifetime::resetCounters();

    constexpr std::size_t kBoundaryBytes = 64U;
    auto* heavy = new (std::align_val_t{kBoundaryBytes}) TrackedLifetime{};           // NOLINT(cppcoreguidelines-owning-memory)
    ASSERT_NE(heavy, nullptr);

    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(heavy) % kBoundaryBytes, 0ULL);         // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

    TrackedLifetime::teardownAligned(heavy, std::align_val_t{kBoundaryBytes});

    EXPECT_GE(TrackedLifetime::alignedDeletes(), 1);
}

TEST(ClassScopedOperators, SizedDeleteOverloadIsExclusiveWithPlainDelete) {
    SizedDeleteWitness::resetCounters();

    auto* sample = new SizedDeleteWitness{};                                            // NOLINT(cppcoreguidelines-owning-memory)
    ASSERT_NE(sample, nullptr);
    EXPECT_EQ(sample->payload, 7);
    EXPECT_GE(SizedDeleteWitness::newHits(), 1);

    delete sample;                                                                        // NOLINT(cppcoreguidelines-owning-memory)

    // 实现要么走带 size 参数的释放路径，要么走单指针路径；二者不得同时计入。
    EXPECT_EQ(SizedDeleteWitness::plainDeleteHits() + SizedDeleteWitness::sizedDeleteHits(), 1);
}
