// std::variant —— 类型安全的 union；std::visit 实现访问者模式。
//
// 关键点：
//   - variant 同时记录 "当前是哪个 alternative" 与底层数据 —— 取错类型会抛
//     std::bad_variant_access（指针版 std::get_if 返回 nullptr）。
//   - variant 默认构造第一个 alternative；用 std::monostate 占位可表达"空"。
//   - std::visit 对每个 alternative 调用相应重载 —— 编译期穷尽检查，加上
//     "重载结合体"（overloaded helper）可以非常优雅。
//   - operator= 抛异常时 variant 可能进入 valueless_by_exception 状态，
//     index() 返回 variant_npos。

#include <iostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace {

// 经典的"重载组合"小工具：
template <class... Fs>
struct Overloaded : Fs... {
    using Fs::operator()...;
};
// CTAD 让 Overloaded{lambda1, lambda2} 直接成立
template <class... Fs>
Overloaded(Fs...) -> Overloaded<Fs...>;

using Shape = std::variant<std::monostate, int, double, std::string>;

}  // namespace

int main() {
    std::cout << "[1] 基本构造与访问\n";
    {
        Shape s;  // 默认是 monostate
        std::cout << "  default index = " << s.index() << "  (monostate)\n";

        s = 42;  // 现在是 int
        std::cout << "  s.index() = " << s.index() << "  std::get<int>(s) = "
                  << std::get<int>(s) << '\n';

        s = std::string{"hello"};
        std::cout << "  s = string\"" << std::get<std::string>(s) << "\"\n";
    }

    std::cout << "\n[2] holds_alternative / get_if 安全访问\n";
    {
        Shape s = 3.14;
        std::cout << "  holds<double>? " << std::holds_alternative<double>(s) << '\n';
        if (auto* p = std::get_if<double>(&s); p != nullptr) {
            std::cout << "  *get_if<double> = " << *p << '\n';
        }
        try {
            std::cout << std::get<int>(s) << '\n';  // 抛 bad_variant_access
        } catch (std::bad_variant_access const& e) {
            std::cout << "  bad_variant_access: " << e.what() << '\n';
        }
    }

    std::cout << "\n[3] std::visit + overloaded：访问者模式\n";
    {
        std::vector<Shape> all;
        all.emplace_back();             // monostate
        all.emplace_back(7);            // int
        all.emplace_back(2.5);          // double
        all.emplace_back("variant");    // string

        for (auto const& v : all) {
            std::visit(
                Overloaded{
                    [](std::monostate) { std::cout << "  <empty>\n"; },
                    [](int i) { std::cout << "  int    = " << i << '\n'; },
                    [](double d) { std::cout << "  double = " << d << '\n'; },
                    [](std::string const& s) { std::cout << "  string = " << s << '\n'; },
                },
                v);
        }
    }

    std::cout << "\n[4] in_place 构造 / emplace —— 非可拷贝类型也能装\n";
    {
        struct OnlyMovable {
            int x;
            explicit OnlyMovable(int v) : x(v) {}
            OnlyMovable(OnlyMovable const&) = delete;
            OnlyMovable(OnlyMovable&&) = default;
            OnlyMovable& operator=(OnlyMovable const&) = delete;
            OnlyMovable& operator=(OnlyMovable&&) = default;
            ~OnlyMovable() = default;
        };
        std::variant<std::monostate, OnlyMovable> v{std::in_place_type<OnlyMovable>, 99};
        std::cout << "  in_place OnlyMovable.x = " << std::get<OnlyMovable>(v).x << '\n';
        v.emplace<OnlyMovable>(100);
        std::cout << "  after emplace        x = " << std::get<OnlyMovable>(v).x << '\n';
    }

    return 0;
}
