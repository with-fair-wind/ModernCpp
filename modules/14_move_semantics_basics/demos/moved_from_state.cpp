// moved-from 对象仍处于「合法但未指定」状态：可析构、可赋值、可调用不前置条件的接口。
//
// 标准库容器/string 一般会变为空或等价空状态，但不要依赖具体数值语义以外的假设。

#include <iostream>
#include <string>
#include <vector>

namespace {

void demoString() {
    std::string s = "hello moved-from semantics";
    std::string sink = std::move(s);

    // 故意：读取 moved-from string 的常见观察值（合法但未指定状态）。
    std::cout << "[string] 移动后源 size=" << s.size()
              << " capacity=" << s.capacity()  // NOLINT(bugprone-use-after-move)
              << '\n';
    s.clear();
    std::cout << "[string] clear 后 size=" << s.size() << '\n';
    s = "reassigned";
    std::cout << "[string] 再赋值: \"" << s << "\"\n";
    std::cout << "[string] 窃取方 sink=\"" << sink << "\"\n";
}

void demoVector() {
    std::vector<int> v(8);
    for (std::size_t i = 0; i < v.size(); ++i) {
        v[i] = static_cast<int>(i * 10);
    }

    std::vector<int> other = std::move(v);

    // 故意：展示 moved-from vector 常为 empty，后续 clear/assign 仍合法。
    std::cout << "[vector] 移动后源 size=" << v.size() << '\n';  // NOLINT(bugprone-use-after-move)
    v.clear();
    std::cout << "[vector] clear 后 size=" << v.size() << '\n';
    v.assign({7, 8, 9});
    std::cout << "[vector] assign 后 size=" << v.size() << " front=" << v.front() << '\n';
    std::cout << "[vector] 窃取方 front=" << other.front() << " back=" << other.back() << '\n';
}

}  // namespace

int main() {
    demoString();
    demoVector();
    return 0;
}
