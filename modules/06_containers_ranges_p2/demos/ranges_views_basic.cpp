// Ranges 基础视图：iota / filter / take / drop / transform / reverse / stride。
//
// 关键点：
//   - View 是惰性求值的——元素只在你迭代时才被计算。
//   - 管道运算符 | 把 range 与 range adaptor 串成流水线，可读性接近函数式。
//   - filter / take / drop 不会改变底层容器；它们只是改变"如何遍历"。
//   - transform 通过返回值做变换，且每次访问都可能重算（不缓存）。
//   - stride(k) 是 Python `range(a, b, k)` 的推广，对任意 range 都适用。

#include <iostream>
#include <ranges>
#include <utility>
#include <vector>

namespace stdv = std::views;
namespace stdr = std::ranges;

namespace {

template <stdr::range R>
void dump(const char* tag, R&& r) {
    std::cout << tag << ": ";
    for (auto&& x : std::forward<R>(r)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    // 1) iota：无须容器即可生成 [1, 10) 的整数 view
    dump("iota(1,10)        ", stdv::iota(1, 10));

    // 2) 管道：取奇数，再取前 3 个
    dump("odd | take(3)     ", stdv::iota(1, 10) | stdv::filter([](int x) { return x % 2 == 1; })
                                   | stdv::take(3));

    // 3) take_while / drop_while：按谓词截断/跳过
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7};
    dump("take_while(<4)    ", v | stdv::take_while([](int x) { return x < 4; }));
    dump("drop_while(<4)    ", v | stdv::drop_while([](int x) { return x < 4; }));

    // 4) drop / take 组合：跳过前 2 个，再取 3 个
    dump("drop(2)|take(3)   ", v | stdv::drop(2) | stdv::take(3));

    // 5) reverse：双向 range 才允许；要避免与缓存型 view 复杂耦合
    dump("reverse           ", v | stdv::reverse);

    // 6) transform：通过返回值变换；不缓存，每次访问重算
    dump("transform(x*x)    ", v | stdv::transform([](int x) { return x * x; }));

    // 7) stride(k)：每隔 k 个取一个，对任意 range 都行
    dump("iota(1,10)|stride(3)", stdv::iota(1, 10) | stdv::stride(3));

    // 8) 可写视图：对 vector 的 view 写入会改原元素
    auto evens = v | stdv::filter([](int x) { return x % 2 == 0; });
    for (auto& x : evens) {
        x *= 10;
    }
    dump("after writing evens", v);  // 偶数被放大 10 倍，奇数不变

    // 9) 流水线 + 急切收集：把 view 倒回 vector 仍然需要 stdr::to
    auto squared = stdv::iota(1, 6) | stdv::transform([](int x) { return x * x; })
                   | stdr::to<std::vector<int>>();
    dump("to<vector>        ", squared);

    return 0;
}
