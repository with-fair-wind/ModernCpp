// 数值算法：iota / accumulate / partial_sum / inner_product / adjacent_difference / reduce。
//
// 关键点：
//   - iota(begin, end, val)：把 [begin, end) 填成 {val, val+1, val+2, ...}。
//   - accumulate / partial_sum / inner_product 保证按从头到尾的顺序求值；适合带状态
//     的 Op（例如折叠字符串、构造前缀和）。
//   - reduce / inclusive_scan / transform_reduce：不保证顺序，因此可向量化或并行；
//     带初值类型必须与结果类型一致——浮点初值不能写 0 而要写 0.0。
//   - C++23 起 stdr::fold_left / fold_left_first 是 accumulate 的 ranges 替代品。
//   - gcd / lcm / midpoint 在 <numeric> 里也很有用。

#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

namespace stdr = std::ranges;

namespace {

void print(const char* tag, const std::vector<int>& v) {
    std::cout << tag << ": ";
    for (int x : v) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    // 1) iota 填一段递增整数（C++23 提供 stdr::iota 的 range 版）
    std::vector<int> v(8);
    stdr::iota(v, 1);
    print("iota fill           ", v);

    // 2) accumulate：从 init 起累加；初值类型决定结果类型
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "accumulate sum      : " << sum << '\n';

    // 3) accumulate 配 std::multiplies 求阶乘
    int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<>{});
    std::cout << "accumulate product  : " << product << '\n';

    // 4) partial_sum：前缀和；输出大小至少与输入相同
    std::vector<int> prefix(v.size());
    std::partial_sum(v.begin(), v.end(), prefix.begin());
    print("partial_sum         ", prefix);

    // 5) adjacent_difference：相邻差分；首项保留原值
    std::vector<int> diffs(v.size());
    std::adjacent_difference(v.begin(), v.end(), diffs.begin());
    print("adjacent_difference ", diffs);

    // 6) inner_product：内积 / 点积
    std::vector<int> a{1, 2, 3, 4};
    std::vector<int> b{10, 20, 30, 40};
    int dot = std::inner_product(a.begin(), a.end(), b.begin(), 0);
    std::cout << "inner_product       : " << dot << '\n';

    // 7) reduce：不保证求值顺序的 accumulate；适合并行/向量化
    int reduced = std::reduce(v.begin(), v.end(), 0);
    std::cout << "reduce              : " << reduced << '\n';

    // 8) inclusive_scan / exclusive_scan：可并行的扫描
    std::vector<int> incl(v.size());
    std::inclusive_scan(v.begin(), v.end(), incl.begin());
    print("inclusive_scan      ", incl);

    std::vector<int> excl(v.size());
    std::exclusive_scan(v.begin(), v.end(), excl.begin(), 0);
    print("exclusive_scan      ", excl);

    // 9) transform_reduce：先对每个元素 transform，再 reduce，常用于内积加权
    int weighted = std::transform_reduce(a.begin(), a.end(), b.begin(), 0);
    std::cout << "transform_reduce dot: " << weighted << '\n';

    // 10) stdr::fold_left：ranges 版的 accumulate
    int fold = stdr::fold_left(v, 0, std::plus<>{});
    std::cout << "stdr::fold_left     : " << fold << '\n';

    // 11) gcd / lcm / midpoint
    std::cout << "gcd(12, 18)         : " << std::gcd(12, 18) << '\n';
    std::cout << "lcm(4, 6)           : " << std::lcm(4, 6) << '\n';
    std::cout << "midpoint(1, 9)      : " << std::midpoint(1, 9) << '\n';

    return 0;
}
