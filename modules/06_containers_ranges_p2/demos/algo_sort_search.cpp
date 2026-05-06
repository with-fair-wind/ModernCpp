// 排序与搜索：sort / stable_sort / partial_sort / nth_element / binary_search。
//
// 关键点：
//   - stdr::sort 在 N < ISORT_MAX 时退化成插入排序；递归过深时用堆排序——典型的
//     introspective sort。最坏 O(N log N) 由实现保证。
//   - stdr::stable_sort 保留等价元素的相对顺序，但通常更慢、需要 O(N) 辅助内存。
//   - stdr::partial_sort(R, mid)：只保证前 mid - begin 个最小元素就位且有序。
//   - stdr::nth_element(R, mid)：保证 *mid 是第 k 小，整体围绕它划分但其它无序。
//   - 二分搜索家族：只在已排序区间上有效；返回值类型与 std::map::lower_bound 一致。
//   - projection（最后一个参数）让排序/比较"通过某个字段"，不必写 lambda 包装。

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

namespace stdr = std::ranges;

namespace {

struct Person {
    std::string name;
    int age;
};

void printAges(const char* tag, const std::vector<Person>& ps) {
    std::cout << tag << ": ";
    for (auto const& p : ps) {
        std::cout << p.name << '/' << p.age << ' ';
    }
    std::cout << '\n';
}

void print(const char* tag, const std::vector<int>& v) {
    std::cout << tag << ": ";
    for (int x : v) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    // 1) 默认 sort（升序）
    std::vector<int> v{5, 2, 8, 1, 9, 3, 7, 4, 6};
    stdr::sort(v);
    print("sort                ", v);

    // 2) projection：按字段排序而不写自定义比较器
    std::vector<Person> ps{{.name = "alice", .age = 30},
                           {.name = "bob", .age = 25},
                           {.name = "carol", .age = 40}};
    stdr::sort(ps, std::less<>{}, &Person::age);
    printAges("sort by age (proj)  ", ps);

    // 3) stable_sort：保留相等元素的相对顺序
    std::vector<Person> ps2{{.name = "alice", .age = 30},
                            {.name = "bob", .age = 30},
                            {.name = "carol", .age = 25}};
    stdr::stable_sort(ps2, std::less<>{}, &Person::age);
    printAges("stable_sort by age  ", ps2);

    // 4) partial_sort：只把前 K 个排好；其余顺序未定
    std::vector<int> p{9, 1, 8, 2, 7, 3, 6, 4, 5};
    stdr::partial_sort(p, p.begin() + 3);
    print("partial_sort top3   ", p);

    // 5) nth_element：取第 k 小，左侧都 ≤ 它，右侧都 ≥ 它（但内部无序）
    std::vector<int> q{9, 1, 8, 2, 7, 3, 6, 4, 5};
    stdr::nth_element(q, q.begin() + 4);
    std::cout << "nth_element(k=4)    : middle=" << q[4] << '\n';

    // 6) binary_search / lower_bound / upper_bound / equal_range
    std::vector<int> sorted{1, 2, 3, 3, 3, 4, 5};
    std::cout << "binary_search 3?    : " << stdr::binary_search(sorted, 3) << '\n';
    auto lb = stdr::lower_bound(sorted, 3);
    auto ub = stdr::upper_bound(sorted, 3);
    std::cout << "equal_range size    : " << std::distance(lb, ub) << '\n';

    // 7) random + sort：典型用法验证（用 random_device 避免确定性种子）
    std::vector<int> big(20);
    std::random_device rd;
    std::mt19937 rng{rd()};
    std::uniform_int_distribution<int> dist(0, 100);
    for (int& x : big) {
        x = dist(rng);
    }
    stdr::sort(big);
    std::cout << "is_sorted(big)?     : " << stdr::is_sorted(big) << '\n';

    // 8) merge：把两个已排序序列合并到 dst
    std::vector<int> a{1, 3, 5, 7};
    std::vector<int> b{2, 4, 6, 8};
    std::vector<int> dst;
    dst.resize(a.size() + b.size());
    stdr::merge(a, b, dst.begin());
    print("merge a+b           ", dst);

    return 0;
}
