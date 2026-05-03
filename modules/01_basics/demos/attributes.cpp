// 标准属性：[[nodiscard]]、[[noreturn]]、[[fallthrough]]、
// [[likely]] / [[unlikely]]、[[maybe_unused]]。
//
// 属性是在不引入新关键字的前提下扩展语言的官方手段。它们既向编译器
// 表达意图，也向阅读者说明语义；以 [[fallthrough]] 为例，它一举两得：
// 既表明"故意贯穿"，又关闭了 -Wimplicit-fallthrough 警告。

#include <cstdlib>
#include <iostream>

namespace {

[[nodiscard("dropping the parsed value is almost certainly a bug")]]
constexpr int doubleIt(int x) noexcept {
    return x * 2;
}

[[noreturn]] void die(char const* msg) {
    std::cerr << "fatal: " << msg << '\n';
    std::abort();
}

constexpr int classify(int x) {
    if (x == 0) [[unlikely]] {
        return 0;
    }
    return x > 0 ? 1 : -1;
}

}  // namespace

int main() {
    [[maybe_unused]] int const reserved = 42;

    std::cout << "doubleIt(7) = " << doubleIt(7) << '\n';
    std::cout << "classify(-3) = " << classify(-3) << '\n';

    int const cmd = 1;
    switch (cmd) {
        case 0:
        case 1:
            std::cout << "fast path\n";
            [[fallthrough]];                              // 故意贯穿
        case 2:
            std::cout << "shared tail\n";
            break;
        default:
            die("unknown command");
    }
    return 0;
}
