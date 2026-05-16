// 手写移动构造函数与移动赋值运算符：管理原始指针拥有的数组。
//
// 规则简要回顾：窃取对方指针并将源指针置空；赋值前先释放己方旧资源，
// 注意与自赋值兼容（移动赋值里常见写法是先判地址或与临时交换）。

#include <algorithm>
#include <iostream>
#include <utility>

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

void printHead(Buffer const& buf, std::size_t n) {
    int const* p = buf.data();
    if (p == nullptr) {
        std::cout << "(empty)";
        return;
    }
    std::size_t const limit = std::min(n, buf.size());
    for (std::size_t i = 0; i < limit; ++i) {
        std::cout << p[i] << (i + 1 < limit ? ", " : "");
    }
}

}  // namespace

int main() {
    Buffer a{8};
    if (a.data() != nullptr) {
        a.data()[0] = 11;
        a.data()[7] = 99;
    }

    Buffer b = std::move(a);  // 移动构造

    std::cout << "移动构造后 b 头部: ";
    printHead(b, 4);
    // 故意：展示移动后被移出对象的指针已置空（moved-from 合法状态）。
    std::cout << "\na.data() == nullptr ? " << (a.data() == nullptr ? "yes" : "no")  // NOLINT(bugprone-use-after-move)
              << '\n';

    Buffer c{4};
    if (c.data() != nullptr) {
        c.data()[0] = -1;
    }
    c = std::move(b);  // 移动赋值：释放 c 的旧块，窃取 b

    std::cout << "移动赋值后 c 头部: ";
    printHead(c, 4);
    // 故意：移动赋值后检视供体 b 已进入空状态。
    std::cout << "\nb.data() == nullptr ? " << (b.data() == nullptr ? "yes" : "no")  // NOLINT(bugprone-use-after-move)
              << '\n';

    return 0;
}
