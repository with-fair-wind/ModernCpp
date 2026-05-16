// 成员函数引用限定符 & / &&：依据对象的值类别选用不同重载。

#include <iostream>
#include <utility>

namespace {

class RelayBox {
public:
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static) — 引用限定符演示必须保留非
    // static
    void ping() & {
        std::cout << "RelayBox::ping() & —— 实例为左值\n";
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void ping() && {
        std::cout << "RelayBox::ping() && —— 实例为右值（含将亡值）\n";
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void ping() const& {
        std::cout << "RelayBox::ping() const& —— const 左值\n";
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void ping() const&& {
        std::cout << "RelayBox::ping() const&& —— const 右值\n";
    }
};

}  // namespace

int main() {
    RelayBox box{};
    RelayBox const boxed{};

    std::cout << "左值实例：\n";
    box.ping();

    std::cout << "右值临时：\n";
    RelayBox{}.ping();

    std::cout << "std::move 左值 → xvalue：\n";
    std::move(box).ping();

    std::cout << "const 左值：\n";
    boxed.ping();

    std::cout << "const 右值：\n";
    static_cast<RelayBox const&&>(RelayBox{}).ping();

    std::cout << "std::move(const 左值)：\n";
    std::move(boxed).ping();

    return 0;
}
