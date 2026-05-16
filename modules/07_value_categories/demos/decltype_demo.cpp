// decltype 与 decltype(auto) 的差别：前者照抄表达式形态，
// decltype(auto) 则是「按初始化器推导」，细节等价于 decltype(x)。

#include <iostream>
#include <string>
#include <type_traits>

namespace {

std::string const& alphaRef() {
    static std::string const k_cache = "alpha";
    return k_cache;
}

std::string valueTemp() {
    return std::string{"beta"};
}

}  // namespace

int main() {
    using std::cout;

    int x = 7;
    int& rx = x;

    static_assert(std::is_same_v<decltype(x), int>);
    // decltype((e)) 需内层括号以得到「泛左值」类型；内层括号会触发
    // readability-redundant-parentheses
    static_assert(
        std::is_same_v<decltype((x)), int&>);  // NOLINT(readability-redundant-parentheses)
    static_assert(std::is_same_v<decltype(rx), int&>);

    cout << "decltype(x)        : " << (std::is_same_v<decltype(x), int> ? "int\n" : "其它\n");
    constexpr bool kDecltypeParenXMatchesIntRef =
        std::is_same_v<decltype((x)), int&>;  // NOLINT(readability-redundant-parentheses) —
                                              // decltype((e)) 教学演示需要内层括号
    cout << "decltype((x))       : " << (kDecltypeParenXMatchesIntRef ? "int&\n" : "其它\n");

    auto v = alphaRef();            // std::string（发生拷贝）
    decltype(auto) w = alphaRef();  // std::string const&（保持引用）

    static_assert(std::is_same_v<decltype(v), std::string>);
    static_assert(std::is_same_v<decltype(w), std::string const&>);

    cout << "auto v              : std::string（拷贝构造）\n";
    cout << "decltype(auto) w    : std::string const&（避免拷贝）\n";
    cout << "通过 w 观察到内容 : " << w << '\n';

    decltype(auto) z = valueTemp();  // decltype(auto) 推导为 std::string（按值）
    static_assert(std::is_same_v<decltype(z), std::string>);
    cout << "decltype(auto) z    : std::string（来自返回值的纯右值初始化）\n";
    cout << "z = " << z << '\n';

    return 0;
}
