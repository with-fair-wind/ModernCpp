// auto / decltype(auto) 推导规则，外加一点结构化绑定演示。
//
// 关键点：
//   auto 默认会剥掉顶层 const 与引用，所以要真正绑成 const 引用
//   必须显式写 `const auto&`；decltype(auto) 则保留表达式的精确
//   类型，引用与 const 都不会丢。

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace {

std::string const& source() {
    static std::string const s = "hello";
    return s;
}

}  // namespace

int main() {
    auto a = source();            // std::string          （发生了拷贝）
    auto const& b = source();     // std::string const&   （不拷贝）
    decltype(auto) c = source();  // std::string const&   （保留引用与 const）

    static_assert(std::is_same_v<decltype(a), std::string>);
    static_assert(std::is_same_v<decltype(b), std::string const&>);
    static_assert(std::is_same_v<decltype(c), std::string const&>);

    std::cout << "auto a       : " << a << '\n'
              << "const auto& b: " << b << '\n'
              << "decltype(auto) c: " << c << '\n';

    // 结构化绑定可以拆解聚合体 / 元组。
    std::pair p{42, std::string{"answer"}};
    auto const& [value, label] = p;
    std::cout << label << " = " << value << '\n';

    return 0;
}
