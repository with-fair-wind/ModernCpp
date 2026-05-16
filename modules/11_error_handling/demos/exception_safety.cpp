// 模块 11 的小演示：RAII 与 copy-and-swap 实现强异常安全的赋值。

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace {

class Widget {
public:
    Widget() = default;
    explicit Widget(std::string data) : data_(std::move(data)) {}

    Widget(Widget const&) = default;
    Widget(Widget&&) noexcept = default;

    ~Widget() = default;

    Widget& operator=(Widget rhs) noexcept {
        swap(*this, rhs);
        return *this;
    }

    Widget& operator=(Widget&&) noexcept = default;

    friend void swap(Widget& a, Widget& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
    }

    [[nodiscard]] std::string const& data() const noexcept {
        return data_;
    }

private:
    std::string data_;
};

void printTwo(std::string_view label, Widget const& a, Widget const& b) {
    std::cout << label << " a=\"" << a.data() << "\" b=\"" << b.data() << "\"\n";
}

}  // namespace

int main() {
    Widget a{"left"};
    Widget b{"right"};

    printTwo("交换前", a, b);
    swap(a, b);
    printTwo("swap 后", a, b);

    std::cout << "\ncopy-and-swap 赋值: a = b\n";
    a = b;
    printTwo("赋值后（应均为 right）", a, b);

    std::cout << "\n说明：operator= 按值接收右侧，由拷贝构造完成资源准备；\n"
                 "若拷贝抛异常，左侧对象不会被部分写入 —— 强异常安全。\n";
    return 0;
}
