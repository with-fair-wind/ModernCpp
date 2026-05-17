// 验证手写类型的移动构造：资源转移到目标，源指针置空且不再拥有内存。

#include <algorithm>
#include <utility>

#include <gtest/gtest.h>

namespace {

class Buffer {
public:
    explicit Buffer(std::size_t size) : data_(size == 0 ? nullptr : new int[size]), size_(size) {
        if (data_ != nullptr) {
            std::fill_n(data_, size_, 0);
        }
    }

    ~Buffer() {
        delete[] data_;
    }

    Buffer(Buffer const&) = delete;
    Buffer& operator=(Buffer const&) = delete;

    Buffer(Buffer&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
        return *this;
    }

    [[nodiscard]] std::size_t size() const {
        return size_;
    }
    [[nodiscard]] int* data() {
        return data_;
    }
    [[nodiscard]] int const* data() const {
        return data_;
    }

private:
    int* data_;
    std::size_t size_;
};

}  // namespace

TEST(MoveCtor, TransfersPointerAndZerosSource) {
    Buffer src{5};
    ASSERT_NE(src.data(), nullptr);
    src.data()[0] = 123;
    src.data()[4] = 7;

    Buffer dst = std::move(src);

    // 故意：验证源的成员在移动后被清空。
    EXPECT_EQ(src.data(), nullptr);  // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(src.size(), 0U);       // NOLINT(bugprone-use-after-move)
    EXPECT_NE(dst.data(), nullptr);
    EXPECT_EQ(dst.size(), 5U);
    EXPECT_EQ(dst.data()[0], 123);
    EXPECT_EQ(dst.data()[4], 7);
}
