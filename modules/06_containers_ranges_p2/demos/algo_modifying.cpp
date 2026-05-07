// 修改型算法：remove / unique / replace / rotate / reverse / shift_left。
//
// 关键点：
//   - 算法本身从不改变容器大小：std::remove/unique 只把"应保留"的元素压到前面，
//     返回新逻辑末尾迭代器；要真正缩小容器需要再调用 v.erase(new_end, v.end())。
//   - C++20 起 std::erase / std::erase_if 一步完成 remove + erase。
//   - rotate(begin, mid, end) 是循环左移；shift_left/shift_right 不是循环——被挤出
//     去的位置内容未指定。
//   - 项目鼓励 modern style，因此 demo 主要展示 stdr:: 版本（除明确演示对照之外）。

#include <algorithm>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>
#include <version>

namespace stdr = std::ranges;
namespace stdv = std::views;

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
    // 1) erase-remove 惯用法：算法不删元素，只把保留项压到前面，再用 .erase 收尾
    std::vector<int> v{1, 2, 2, 3, 2, 4, 5, 2};
    auto [logical_end, _] = stdr::remove(v, 2);  // 返回 subrange<It, It>
    std::cout << "stdr::remove logical end at index " << std::distance(v.begin(), logical_end)
              << ", size 仍为 " << v.size() << '\n';
    v.erase(logical_end, v.end());
    print("after erase-remove   ", v);

    // 2) C++20 std::erase_if：一步完成；按 predicate 删
    std::vector<int> w{1, 2, 3, 4, 5, 6, 7};
    auto removed = std::erase_if(w, [](int x) { return x % 2 == 0; });
    std::cout << "erase_if removed " << removed << " evens; ";
    print("                     ", w);

    // 3) stdr::unique：仅去除*相邻*重复（要先 sort 才能去重所有）
    std::vector<int> u{1, 1, 2, 3, 3, 3, 4, 4, 5};
    auto unique_tail = stdr::unique(u);
    u.erase(unique_tail.begin(), unique_tail.end());
    print("unique 已排序输入    ", u);

    // 4) stdr::replace / replace_if
    std::vector<int> r{0, 1, 0, 2, 0, 3};
    stdr::replace(r, 0, -1);
    print("replace 0 -> -1      ", r);
    stdr::replace_if(r, [](int x) { return x < 0; }, 99);
    print("replace_if (<0)->99  ", r);

    // 5) stdr::rotate：把 [begin, mid) 左旋到末尾
    std::vector<int> rot{1, 2, 3, 4, 5};
    stdr::rotate(rot, rot.begin() + 2);
    print("rotate(mid=2)        ", rot);

    // 6) stdr::reverse
    std::vector<int> rv{1, 2, 3, 4, 5};
    stdr::reverse(rv);
    print("reverse              ", rv);

    // 7) stdr::shift_left：左移 2 位（不是 rotate，被挤出去的位置内容未指定）。
    //    C++23 P2440R1 把 shift_left/right 加进 std::ranges；feature test macro
    //    __cpp_lib_shift 升到 202202L 表示已实现 ranges 版。MSVC STL 17.5+ 已实现，
    //    libstdc++ 15、libc++ 当前都还没有 —— 缺则跳过这一段，其余 demo 不受影响。
#if defined(__cpp_lib_shift) && __cpp_lib_shift >= 202202L
    std::vector<int> sl{1, 2, 3, 4, 5};
    auto valid_prefix = stdr::shift_left(sl, 2);
    std::cout << "shift_left(2): valid prefix size = " << stdr::distance(valid_prefix)
              << "; data: ";
    for (int x : valid_prefix) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
#else
    std::cout << "shift_left: 当前 stdlib 未实现 std::ranges::shift_left（P2440R1）—— 跳过\n";
#endif

    // 8) iter_swap
    std::vector<int> sw{10, 20, 30, 40};
    stdr::iter_swap(sw.begin(), sw.begin() + 3);
    print("iter_swap(0,3)       ", sw);

    // 9) 流水线收集：filter + transform + reverse + to<vector>
    std::vector<int> piped = stdv::iota(1, 11) | stdv::filter([](int x) { return x % 2 == 0; }) |
                             stdv::transform([](int x) { return x * x; }) | stdv::reverse |
                             stdr::to<std::vector<int>>();
    print("piped 偶数^2 反转    ", piped);

    return 0;
}
