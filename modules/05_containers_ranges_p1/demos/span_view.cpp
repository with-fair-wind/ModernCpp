// std::span：连续内存的非拥有视图（C++20）。
//
// 关键点：
//   - span 只是"指针 + 长度"，构造与拷贝都很廉价；它不拥有数据。
//   - 当你只想观察一段连续元素（不论它来自 vector / array / 原生数组），
//     传 span<T> 比传引用 + size 或拷贝出新容器都好。
//   - 来源容器一旦发生重分配（例如 vector 扩容），span 就悬空 —— 与裸指针一样不安全。
//   - 静态 extent (`std::span<T, N>`) 把长度编入类型；只有 std::array 与 T[N]
//     可以隐式转换得到。
//   - C++26 之前 span 不做边界检查，越界访问 UB。

#include <array>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

namespace {

// 函数形参写成 std::span，可以接受 vector / array / 原生数组 / initializer_list
int sum(std::span<const int> s) {
    return std::accumulate(s.begin(), s.end(), 0);
}

}  // namespace

int main() {
    std::vector<int> vec{1, 2, 3, 4, 5, 6};
    std::array<int, 4> arr{10, 20, 30, 40};
    int c_arr[3]{100, 200, 300};

    std::cout << "sum(vec) = " << sum(vec) << '\n';
    std::cout << "sum(arr) = " << sum(arr) << '\n';
    std::cout << "sum(c_arr) = " << sum(c_arr) << '\n';

    // 子 span：first / last / subspan
    std::span<int> s{vec};
    auto head3 = s.first(3);
    auto tail2 = s.last(2);
    auto mid = s.subspan(2, 3);  // 从下标 2 起、长 3
    std::cout << "first(3) sum = " << sum(head3) << '\n';
    std::cout << "last(2)  sum = " << sum(tail2) << '\n';
    std::cout << "subspan(2,3) sum = " << sum(mid) << '\n';

    // span 是非拥有视图：通过它修改会写回原 vector
    s[0] = 999;
    std::cout << "after s[0]=999, vec[0]=" << vec[0] << '\n';

    // 静态 extent：编译期长度 → sizeof(span) 通常只有一个指针
    std::span<int, 4> static_view{arr};
    constexpr auto kExt = decltype(static_view)::extent;
    std::cout << "static extent = " << kExt << ", dynamic_extent? " << (kExt == std::dynamic_extent)
              << '\n';

    // 转字节视图：常用于二进制 IO
    auto bytes = std::as_bytes(std::span{arr});
    std::cout << "as_bytes size = " << bytes.size_bytes() << " (= " << arr.size() * sizeof(int)
              << ")\n";

    return 0;
}
