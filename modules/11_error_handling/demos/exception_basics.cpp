// 模块 11 的小演示：try-catch、标准异常层次与栈展开时局部对象的析构。

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct ScopeGuard {
    std::string label_;
    explicit ScopeGuard(std::string label) : label_(std::move(label)) {
        std::cout << "进入作用域: " << label_ << '\n';
    }
    ~ScopeGuard() {
        std::cout << "析构（栈展开也会执行）: " << label_ << '\n';
    }

    ScopeGuard(ScopeGuard const&) = delete;
    ScopeGuard& operator=(ScopeGuard const&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;
};

void throwRuntime() {
    throw std::runtime_error{"模拟运行时错误"};
}

void throwLogic() {
    throw std::logic_error{"逻辑前提被破坏"};
}

void callChain() {
    ScopeGuard inner{"inner"};
    throwRuntime();
}

}  // namespace

int main() {
    std::cout << "--- std::logic_error ---\n";
    try {
        throwLogic();
    } catch (std::logic_error const& ex) {
        std::cout << "按 logic_error 捕获: " << ex.what() << '\n';
    }

    std::cout << "\n--- std::runtime_error + 栈展开 ---\n";
    try {
        ScopeGuard outer{"outer"};
        callChain();
    } catch (std::runtime_error const& ex) {
        std::cout << "按 runtime_error 捕获: " << ex.what() << '\n';
    } catch (std::exception const& ex) {
        std::cout << "通用 std::exception: " << ex.what() << '\n';
    }

    std::cout << "\n--- 按引用重抛后再捕获 ---\n";
    try {
        try {
            throw std::invalid_argument{"invalid_argument 示例"};
        } catch (std::exception const& ex) {
            std::cout << "内层捕获: " << ex.what() << "，随后重抛\n";
            throw;
        }
    } catch (std::invalid_argument const& ex) {
        std::cout << "外层仍看到具体类型: " << ex.what() << '\n';
    }

    return 0;
}
