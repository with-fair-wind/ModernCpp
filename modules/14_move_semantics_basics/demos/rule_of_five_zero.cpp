// 五法则：需要显式声明/定义全部 5 个特殊成员（析构、拷贝构造、拷贝赋值、
// 移动构造、移动赋值）来正确管理裸资源。
// 零法则：成员用标准库 RAII（如 unique_ptr）包装，编译器生成的特殊成员即可。

#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <utility>

namespace {

class ResourceOwner {
public:
    explicit ResourceOwner(std::size_t bytes)
        : size_(bytes), data_(bytes == 0 ? nullptr : new std::byte[bytes]) {}

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
    [[nodiscard]] std::size_t size() const {
        return size_;
    }

private:
    std::size_t size_;
    std::byte* data_;
};

class SafeOwner {
public:
    explicit SafeOwner(std::size_t bytes)
        : size_(bytes), store_(bytes == 0 ? nullptr : std::make_unique<std::byte[]>(bytes)) {}

    [[nodiscard]] std::byte* data() {
        return store_.get();
    }
    [[nodiscard]] std::size_t size() const {
        return size_;
    }

    // 其余特殊成员全部使用编译器默认 = 零法则

private:
    std::size_t size_;
    std::unique_ptr<std::byte[]> store_;
};

}  // namespace

int main() {
    ResourceOwner a{16};
    if (a.data() != nullptr) {
        std::memset(a.data(), 0xAB, a.size());
    }

    ResourceOwner b = a;             // 深拷贝
    ResourceOwner c = std::move(a);  // 移动：a 进入空状态

    std::cout << "五法则 ResourceOwner：\n"
              << "  b.size = " << b.size() << ", c.size = " << c.size()
              << ", moved-from a.data null? " << (a.data() == nullptr ? "yes" : "no")  // NOLINT(bugprone-use-after-move)
              << '\n';  // 故意：检视五法则手写类型移动后的供体为空

    SafeOwner s{32};
    SafeOwner t = std::move(s);  // unique_ptr 移动

    // 故意：读取 unique_ptr 被移走后 SafeOwner 的 data()。
    std::cout << "零法则 SafeOwner：移动后 s.data null? " << (s.data() == nullptr ? "yes" : "no")  // NOLINT(bugprone-use-after-move)
              << ", t.size = " << t.size() << '\n';

    return 0;
}
