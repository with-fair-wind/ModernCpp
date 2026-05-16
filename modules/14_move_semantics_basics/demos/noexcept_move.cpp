// noexcept 移动如何影响 std::vector 在扩容时的元素迁移策略。
//
// C++11 起：若元素的移动构造函数 noexcept，vector 扩容会用移动；
// 否则为维持强异常安全，往往退回为拷贝已有元素。

#include <iostream>
#include <vector>

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

    // 故意不加 noexcept：模拟「移动可能抛异常」的类型；vector 扩容因此常退回拷贝而非移动。
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

template <typename T>
void growVector() {
    resetCounters<T>();
    std::vector<T> v;
    v.reserve(1);
    for (int i = 0; i < 24; ++i) {
        v.push_back(T{i});
    }
    std::cout << "  拷贝次数: " << T::copy_count << ", 移动次数: " << T::move_count << '\n';
}

}  // namespace

int main() {
    std::cout << "StrongNoexcept（移动 noexcept）扩容迁移：\n";
    growVector<StrongNoexcept>();

    std::cout << "MaybeThrowMove（移动非 noexcept）扩容迁移：\n";
    growVector<MaybeThrowMove>();

    return 0;
}
