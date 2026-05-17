// PImpl（pointer to implementation）：编译隔离与稳定 ABI 的常用手法。
// 本 demo 将全部定义放在同一翻译单元：对外只暴露不完整类型的前向声明。

#include <iostream>
#include <memory>
#include <string>

// 前置声明：头文件中通常就只写到这里。
class WidgetApi;

namespace {

class WidgetImpl {
public:
    explicit WidgetImpl(std::string name) : name_(std::move(name)) {}

    [[nodiscard]] std::string greeting() const {
        return "Hello, " + name_;
    }

private:
    std::string name_;
};

}  // namespace

class WidgetApi {
public:
    explicit WidgetApi(std::string name);

    WidgetApi(const WidgetApi&) = delete;
    WidgetApi& operator=(const WidgetApi&) = delete;

    WidgetApi(WidgetApi&&) noexcept = default;
    WidgetApi& operator=(WidgetApi&&) noexcept = default;

    ~WidgetApi() = default;

    [[nodiscard]] std::string describe() const;

private:
    std::unique_ptr<WidgetImpl> impl_;
};

WidgetApi::WidgetApi(std::string name) : impl_(std::make_unique<WidgetImpl>(std::move(name))) {}

std::string WidgetApi::describe() const {
    return impl_->greeting();
}

int main() {
    WidgetApi widget{"Modern C++"};
    std::cout << widget.describe() << '\n';
    return 0;
}
