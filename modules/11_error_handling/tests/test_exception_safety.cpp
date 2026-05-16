// 模块 11：copy-and-swap 赋值的强异常安全——拷贝抛异常时左侧不变。

#include <stdexcept>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace {

class UnsafeCopyWidget {
public:
    explicit UnsafeCopyWidget(int id, std::string data = {}) : id_(id), data_(std::move(data)) {}

    UnsafeCopyWidget(UnsafeCopyWidget const& other) : id_(other.id_), data_(other.data_) {
        if (fail_next_copy) {
            throw std::runtime_error("copy aborted");
        }
    }

    UnsafeCopyWidget(UnsafeCopyWidget&& other) noexcept
        : id_(other.id_), data_(std::move(other.data_)) {}

    ~UnsafeCopyWidget() = default;

    UnsafeCopyWidget& operator=(UnsafeCopyWidget rhs) noexcept {
        swap(*this, rhs);
        return *this;
    }

    UnsafeCopyWidget& operator=(UnsafeCopyWidget&&) noexcept = default;

    friend void swap(UnsafeCopyWidget& a, UnsafeCopyWidget& b) noexcept {
        using std::swap;
        swap(a.id_, b.id_);
        swap(a.data_, b.data_);
    }

    [[nodiscard]] int id() const noexcept {
        return id_;
    }
    [[nodiscard]] std::string const& data() const noexcept {
        return data_;
    }

    static void setFailNextCopy(bool v) noexcept {
        fail_next_copy = v;
    }

private:
    static bool fail_next_copy;
    int id_;
    std::string data_;
};

bool UnsafeCopyWidget::fail_next_copy = false;

}  // namespace

TEST(ExceptionSafety, AssignWhenCopyThrowsLeavesLhsIntact) {
    UnsafeCopyWidget::setFailNextCopy(false);
    UnsafeCopyWidget lhs{1, "hello"};
    UnsafeCopyWidget rhs{2, "world"};

    UnsafeCopyWidget::setFailNextCopy(true);
    EXPECT_THROW(lhs = rhs, std::runtime_error);
    UnsafeCopyWidget::setFailNextCopy(false);

    EXPECT_EQ(lhs.id(), 1);
    EXPECT_EQ(lhs.data(), "hello");
}

TEST(ExceptionSafety, AssignSucceedsWhenCopyAllowed) {
    UnsafeCopyWidget::setFailNextCopy(false);
    UnsafeCopyWidget lhs{1, "a"};
    UnsafeCopyWidget rhs{9, "z"};

    EXPECT_NO_THROW(lhs = rhs);
    EXPECT_EQ(lhs.id(), 9);
    EXPECT_EQ(lhs.data(), "z");
}
