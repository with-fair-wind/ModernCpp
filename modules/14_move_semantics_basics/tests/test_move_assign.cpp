// 移动赋值：释放旧资源、窃取新资源；自移动赋值路径不破坏对象（合法实现下）。

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

private:
    int* data_;
    std::size_t size_;
};

}  // namespace

TEST(MoveAssign, StealsResourceAndClearsDonor) {
    Buffer lhs{3};
    Buffer rhs{7};

    lhs.data()[0] = -1;
    rhs.data()[0] = 99;

    lhs = std::move(rhs);

    // 故意：断言移动赋值后右侧进入空状态（仍为合法检视）。
    EXPECT_EQ(rhs.data(), nullptr);  // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(rhs.size(), 0U);       // NOLINT(bugprone-use-after-move)
    ASSERT_NE(lhs.data(), nullptr);
    EXPECT_EQ(lhs.size(), 7U);
    EXPECT_EQ(lhs.data()[0], 99);
}

TEST(MoveAssign, SelfMoveIsHandled) {
    Buffer x{4};
    ASSERT_NE(x.data(), nullptr);
    x.data()[0] = 42;

    Buffer& ref = x;
    x = std::move(ref);

    ASSERT_NE(x.data(), nullptr);
    EXPECT_EQ(x.size(), 4U);
    EXPECT_EQ(x.data()[0], 42);
}
