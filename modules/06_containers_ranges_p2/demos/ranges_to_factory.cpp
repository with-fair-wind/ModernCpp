// Range factory + stdr::to + subrange + fold_left。
//
// 关键点：
//   - Range factory 不需要"已有"容器，直接产生 view：iota / single / empty / repeat。
//   - stdr::to<Container>() 是 C++23 新增的"急切收集"工具：流水线终点把 view 物化成容器。
//   - subrange(first, last [, size]) 把任意一对迭代器/哨兵打包成 range；提供 size 后
//     可以让 size() 是 𝑂(1)。
//   - fold_left(R, init, Op) 对应旧 std::accumulate，是 ranges 版的"折叠"。

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace stdv = std::views;
namespace stdr = std::ranges;

int main() {
    // 1) iota：无限序列也合法，但必须用 take 截断
    auto first10 = stdv::iota(0) | stdv::take(10);
    std::cout << "iota(0)|take(10) : ";
    for (auto x : first10) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 2) single(value)：长度恰为 1 的 view，常用于"补一个边界"
    std::cout << "single(42)       : ";
    for (auto x : stdv::single(42)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 3) empty<T>：空 view；与上面拼接的占位场景
    auto e = stdv::empty<int>;
    std::cout << "empty<int> size  : " << stdr::distance(e) << '\n';

    // 4) repeat(value, k)：重复 k 次
    std::cout << "repeat('x', 5)   : ";
    for (auto c : stdv::repeat('x', 5)) {
        std::cout << c;
    }
    std::cout << '\n';

    // 5) stdr::to<vector>：把 view 物化成容器（终结流水线）
    auto squares = stdv::iota(1, 6) | stdv::transform([](int x) { return x * x; }) |
                   stdr::to<std::vector<int>>();
    std::cout << "to<vector>       : ";
    for (auto x : squares) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 6) stdr::to<map>：源是 (key, value) 元组的 view 时，可以直接收成 map
    auto m = stdv::iota(0, 4) | stdv::transform([](int i) {
                 return std::pair{i, std::string(1, static_cast<char>('a' + i))};
             }) |
             stdr::to<std::map<int, std::string>>();
    std::cout << "to<map>          : ";
    for (auto const& [k, v] : m) {
        std::cout << k << "->" << v << ' ';
    }
    std::cout << '\n';

    // 7) subrange：把一对迭代器包成 range；size() 由 distance 推算
    std::vector<int> v{10, 20, 30, 40, 50};
    auto middle = stdr::subrange(v.begin() + 1, v.begin() + 4);
    std::cout << "subrange size    : " << middle.size() << '\n';

    // 8) fold_left：等价于 std::accumulate，但更安全（要求严格 range）
    auto sum = stdr::fold_left(stdv::iota(1, 11), 0, std::plus<>{});
    std::cout << "fold_left sum    : " << sum << '\n';

    // 9) fold_left_first：以首元素为初值；空 range 时返回 std::optional 的空值
    auto product_opt = stdr::fold_left_first(std::vector{1, 2, 3, 4}, std::multiplies<>{});
    std::cout << "fold_left_first  : " << product_opt.value_or(-1) << '\n';

    return 0;
}
