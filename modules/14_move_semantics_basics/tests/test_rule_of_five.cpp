// ResourceOwner（五法则）在拷贝/移动/赋值/析构全路径下的行为。

#include <cstddef>
#include <cstring>
#include <utility>

#include <gtest/gtest.h>

namespace {

class ResourceOwner {
public:
    explicit ResourceOwner(std::size_t bytes)
        : size_(bytes), data_(bytes == 0 ? nullptr : new std::byte[bytes]) {
        if (data_ != nullptr) {
            std::memset(data_, 0, size_);
        }
    }

    ~ResourceOwner() {
        delete[] data_;
    }

    ResourceOwner(ResourceOwner const& other)
        : size_(other.size_), data_(other.size_ == 0 ? nullptr : new std::byte[other.size_]) {
        if (data_ != nullptr && other.data_ != nullptr) {
            std::memcpy(data_, other.data_, size_);
        }
    }

    ResourceOwner& operator=(ResourceOwner const& other) {
        if (this == &other) {
            return *this;
        }
        ResourceOwner tmp{other};
        swap(tmp);
        return *this;
    }

    ResourceOwner(ResourceOwner&& other) noexcept : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
    }

    ResourceOwner& operator=(ResourceOwner&& other) noexcept {
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

    void swap(ResourceOwner& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
    }

    [[nodiscard]] std::byte* data() {
        return data_;
    }
    [[nodiscard]] std::byte const* data() const {
        return data_;
    }
    [[nodiscard]] std::size_t size() const {
        return size_;
    }

private:
    std::size_t size_;
    std::byte* data_;
};

void writeMarker(ResourceOwner& owner, unsigned char marker) {
    EXPECT_NE(owner.data(), nullptr);
    EXPECT_GE(owner.size(), 1U);
    if (owner.data() == nullptr || owner.size() < 1U) {
        return;
    }
    owner.data()[0] = static_cast<std::byte>(marker);
}

[[nodiscard]] unsigned char readMarker(ResourceOwner const& owner) {
    EXPECT_NE(owner.data(), nullptr);
    EXPECT_GE(owner.size(), 1U);
    if (owner.data() == nullptr || owner.size() < 1U) {
        return 0;
    }
    return static_cast<unsigned char>(owner.data()[0]);
}

}  // namespace

TEST(RuleOfFive, DeepCopyIsIndependent) {
    ResourceOwner a{8};
    writeMarker(a, 0xAB);

    ResourceOwner b = a;
    ASSERT_NE(a.data(), b.data());
    EXPECT_EQ(readMarker(a), 0xAB);
    EXPECT_EQ(readMarker(b), 0xAB);

    writeMarker(b, 0xCD);
    EXPECT_EQ(readMarker(a), 0xAB);
    EXPECT_EQ(readMarker(b), 0xCD);
}

TEST(RuleOfFive, CopyAssignDuplicatesPodPayload) {
    ResourceOwner x{4};
    ResourceOwner y{4};
    writeMarker(x, 0x11);
    writeMarker(y, 0x22);

    x = y;
    ASSERT_NE(x.data(), y.data());
    EXPECT_EQ(readMarker(x), readMarker(y));
}

TEST(RuleOfFive, MoveCtorLeavesSourceEmpty) {
    ResourceOwner src{16};
    writeMarker(src, 0x55);

    ResourceOwner dst = std::move(src);

    // 故意：对 moved-from 供体仍可安全查询 size/pointer。
    EXPECT_EQ(src.data(), nullptr);  // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(src.size(), 0U);       // NOLINT(bugprone-use-after-move)
    ASSERT_NE(dst.data(), nullptr);
    EXPECT_EQ(readMarker(dst), 0x55);
}

TEST(RuleOfFive, MoveAssignReplacesOwnership) {
    ResourceOwner lhs{8};
    ResourceOwner rhs{8};
    writeMarker(lhs, 0x01);
    writeMarker(rhs, 0xFE);

    lhs = std::move(rhs);

    EXPECT_EQ(rhs.data(), nullptr);  // NOLINT(bugprone-use-after-move)：故意检视移动赋值后的 rhs
    ASSERT_NE(lhs.data(), nullptr);
    EXPECT_EQ(readMarker(lhs), 0xFE);
}
