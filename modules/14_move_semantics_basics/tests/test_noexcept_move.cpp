// noexcept 移动：vector 扩容时优先用移动迁移旧元素；否则为强异常安全常退回拷贝。

#include <vector>

#include <gtest/gtest.h>

namespace {

struct StrongNoexcept {
    static inline int copy_count = 0;
    static inline int move_count = 0;

    explicit StrongNoexcept(int v = 0) : value_(v) {}

    StrongNoexcept(StrongNoexcept const& other) : value_(other.value_) {
        ++copy_count;
    }

    StrongNoexcept& operator=(StrongNoexcept const& other) {
        if (this != &other) {
            value_ = other.value_;
            ++copy_count;
        }
        return *this;
    }

    StrongNoexcept(StrongNoexcept&& other) noexcept : value_(other.value_) {
        ++move_count;
    }

    StrongNoexcept& operator=(StrongNoexcept&& other) noexcept {
        if (this != &other) {
            value_ = other.value_;
            ++move_count;
        }
        return *this;
    }

    ~StrongNoexcept() = default;

    [[nodiscard]] int value() const {
        return value_;
    }

private:
    int value_;
};

struct MaybeThrowMove {
    static inline int copy_count = 0;
    static inline int move_count = 0;

    explicit MaybeThrowMove(int v = 0) : value_(v) {}

    MaybeThrowMove(MaybeThrowMove const& other) : value_(other.value_) {
        ++copy_count;
    }

    MaybeThrowMove& operator=(MaybeThrowMove const& other) {
        if (this != &other) {
            value_ = other.value_;
            ++copy_count;
        }
        return *this;
    }

    // 故意不加 noexcept，与 demo 一致，用于观测 vector 扩容回拷贝。
    // NOLINTNEXTLINE(cppcoreguidelines-noexcept-move-operations,performance-noexcept-move-constructor)
    MaybeThrowMove(MaybeThrowMove&& other) : value_(other.value_) {
        ++move_count;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-noexcept-move-operations,performance-noexcept-move-constructor)
    MaybeThrowMove& operator=(MaybeThrowMove&& other) {
        if (this != &other) {
            value_ = other.value_;
            ++move_count;
        }
        return *this;
    }

    ~MaybeThrowMove() = default;

    [[nodiscard]] int value() const {
        return value_;
    }

private:
    int value_;
};

template <typename T>
void resetCounters() {
    T::copy_count = 0;
    T::move_count = 0;
}

}  // namespace

TEST(NoexceptMove, VectorReallocUsesMoveWhenNoexcept) {
    resetCounters<StrongNoexcept>();

    std::vector<StrongNoexcept> v;
    v.reserve(1);
    for (int i = 0; i < 48; ++i) {
        v.emplace_back(i);
    }

    EXPECT_EQ(StrongNoexcept::copy_count, 0);
    EXPECT_GT(StrongNoexcept::move_count, 0);
}

TEST(NoexceptMove, VectorReallocFallsBackToCopyWhenMoveMayThrow) {
    resetCounters<MaybeThrowMove>();

    std::vector<MaybeThrowMove> v;
    v.reserve(1);
    for (int i = 0; i < 48; ++i) {
        v.emplace_back(i);
    }

    EXPECT_GT(MaybeThrowMove::copy_count, 0);
    EXPECT_EQ(MaybeThrowMove::move_count, 0);
}
