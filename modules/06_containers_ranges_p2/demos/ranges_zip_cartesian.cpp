// 多 range 组合：zip / zip_transform / cartesian_product / pairwise。
//
// 关键点：
//   - zip(r1, r2, ...)：以参数形式接受多个 range，最短一方到达 end 就结束。
//     返回 std::tuple<T1&, T2&, ...>，因此 auto [a, b, c] 拿到的仍是引用。
//   - 不能写 const auto& [a, b]：得到的是 const std::tuple<T&, U&>，仍允许写！
//     真要只读就在管线末尾再加 | stdv::as_const，让元组里都是 const T&。
//   - cartesian_product(r1, r2, ...)：返回笛卡尔积，元素个数是 |r1| * |r2| * ...
//   - zip_transform(F, r1, r2, ...)：F 直接收到解包后的元素，不必再写 std::get。
//   - pairwise == adjacent<2>：滑动窗口宽度为 2，常用于做差分/邻接判断。

#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace stdv = std::views;

int main() {
    std::vector<int> ids{1, 2, 3, 4};
    std::vector<std::string> names{"alice", "bob", "carol"};
    std::vector<double> scores{91.5, 88.0, 73.2, 99.9};

    // 1) zip：按最短长度并行迭代多个容器
    std::cout << "zip(ids, names, scores):\n";
    for (auto const& [id, name, score] : stdv::zip(ids, names, scores)) {
        std::cout << "  " << id << ' ' << name << ' ' << score << '\n';
    }

    // 2) zip 可写：通过 tuple 引用直接改原元素
    for (auto [id, score] : stdv::zip(ids, scores)) {
        if (score < 80.0) {
            id = -id;  // 把不及格者的 id 取负
        }
    }
    std::cout << "ids after zip-write: ";
    for (int x : ids) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 3) zip_transform：边 zip 边变换；F 直接收到解包后的多参
    auto sums = stdv::zip_transform([](int i, double d) { return i + d; }, ids, scores);
    std::cout << "zip_transform(+) : ";
    for (auto x : sums) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 4) cartesian_product：笛卡尔积
    std::cout << "cartesian_product:\n";
    for (auto const& [a, b] :
         stdv::cartesian_product(std::vector{1, 2, 3}, std::vector<char>{'x', 'y'})) {
        std::cout << "  (" << a << ',' << b << ")\n";
    }

    // 5) pairwise / adjacent<2>：滑动窗口宽度 2，做相邻差分
    std::vector<int> v{1, 3, 6, 10, 15};
    std::cout << "pairwise diffs   : ";
    for (auto const& [prev, cur] : v | stdv::pairwise) {
        std::cout << (cur - prev) << ' ';
    }
    std::cout << '\n';

    return 0;
}
