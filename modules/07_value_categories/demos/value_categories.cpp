// 用 decltype((expr)) 配合 type_traits 观察 lvalue / xvalue / prvalue。
//
// 规则（C++）：对 decltype((e))，
//   - 若 e 为左值 → 类型为 T&；
//   - 若 e 将亡值 → 类型为 T&&；
//   - 若 e 为纯右值 → 类型为 T（非引用）。

#include <iostream>
#include <type_traits>
#include <utility>

namespace {

void printCategory(char const* expr_text, bool is_lv, bool is_xv, bool is_pv) {
    std::cout << expr_text << " → ";
    if (is_lv) {
        std::cout << "lvalue";
    } else if (is_xv) {
        std::cout << "xvalue";
    } else if (is_pv) {
        std::cout << "prvalue";
    } else {
        std::cout << "???";
    }
    std::cout << '\n';
}

}  // namespace

// decltype((e)) 需宏展开内层括号，无法用 constexpr 替代
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MCPP_CAT_LVALUE(expr) std::is_lvalue_reference_v<decltype((expr))>
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MCPP_CAT_XVALUE(expr) \
    (!MCPP_CAT_LVALUE(expr) && std::is_rvalue_reference_v<decltype((expr))>)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MCPP_CAT_PRVALUE(expr) (!std::is_reference_v<decltype((expr))>)

int main() {
    int n = 42;

    static_assert(MCPP_CAT_LVALUE(n));
    static_assert(MCPP_CAT_XVALUE(std::move(n)));
    static_assert(MCPP_CAT_PRVALUE(42));
    static_assert(MCPP_CAT_LVALUE(std::cout << ""));

    printCategory("具名变量 n", MCPP_CAT_LVALUE(n), MCPP_CAT_XVALUE(n), MCPP_CAT_PRVALUE(n));

    printCategory("std::move(n)", MCPP_CAT_LVALUE(std::move(n)), MCPP_CAT_XVALUE(std::move(n)),
                  MCPP_CAT_PRVALUE(std::move(n)));

    printCategory("整型字面量 42", MCPP_CAT_LVALUE(42), MCPP_CAT_XVALUE(42), MCPP_CAT_PRVALUE(42));

    int arr[3]{};  // NOLINT(cppcoreguidelines-avoid-c-arrays)
    (void)arr;     // 仅在 decltype 中使用，消除 unused-but-set 警告
    printCategory("数组元素 arr[0]", MCPP_CAT_LVALUE(arr[0]), MCPP_CAT_XVALUE(arr[0]),
                  MCPP_CAT_PRVALUE(arr[0]));

    std::cout << "\n判定诀窍：\n"
              << "  std::is_lvalue_reference_v<decltype((expr))> → lvalue\n"
              << "  std::is_rvalue_reference_v<decltype((expr))> → xvalue\n"
              << "  !std::is_reference_v<decltype((expr))>       → prvalue\n";

    return 0;
}

#undef MCPP_CAT_PRVALUE
#undef MCPP_CAT_XVALUE
#undef MCPP_CAT_LVALUE
