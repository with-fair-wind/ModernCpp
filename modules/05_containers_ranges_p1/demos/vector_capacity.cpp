// vector 的 size / capacity / 重分配观察。
//
// 关键点：
//   - capacity 与 size 不同：capacity 是已分配的连续空间能容纳的元素数；
//     size 是当前实际持有的元素数。
//   - push_back 在 size == capacity 时触发重分配；多数实现按 1.5×（MS）或 2×
//     （libstdc++）增长，从而保证摊还 O(1)。
//   - reserve(n) 提前扩容到 capacity >= n，但不改变 size。提前 reserve 可以避免
//     反复重分配，并在多线程持有迭代器/指针时避免悬空。
//   - shrink_to_fit() 是请求，多数实现会照办，但不强制。
//   - clear() 把 size 清零，但不释放容量。

#include <cstddef>
#include <iostream>
#include <vector>

namespace {

void printState(const char* tag, const std::vector<int>& v) {
    std::cout << tag << ": size=" << v.size() << ", capacity=" << v.capacity() << '\n';
}

}  // namespace

int main() {
    // 1) 默认构造的 vector 容量为 0
    std::vector<int> v;
    printState("default       ", v);

    // 2) 连续 push_back，观察 capacity 跳变（标准未规定具体增长因子）
    std::cout << "push_back 1..10 — capacity steps:\n";
    std::size_t last_cap = v.capacity();
    for (int i = 1; i <= 10; ++i) {
        v.push_back(i);
        if (v.capacity() != last_cap) {
            std::cout << "  size=" << v.size() << ", capacity=" << v.capacity() << '\n';
            last_cap = v.capacity();
        }
    }

    // 3) reserve 一步到位，避免反复重分配
    std::vector<int> w;
    w.reserve(100);
    printState("after reserve(100)", w);
    for (int i = 0; i < 100; ++i) {
        w.push_back(i);
    }
    printState("after 100 push_back", w);  // capacity 仍是 100

    // 4) shrink_to_fit：把多余容量交回
    v.shrink_to_fit();
    printState("after shrink ", v);

    // 5) clear 不释放容量
    v.clear();
    printState("after clear  ", v);

    // 6) erase 返回下一个有效迭代器；可用于在循环中边删边遍历
    std::vector<int> nums{1, 2, 3, 4, 5, 6};
    for (auto it = nums.begin(); it != nums.end();) {
        if (*it % 2 == 0) {
            it = nums.erase(it);  // 不要写成 ++it，否则会跳过下一个元素
        } else {
            ++it;
        }
    }
    std::cout << "after erase even: ";
    for (int x : nums) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 7) C++20: std::erase / std::erase_if，O(n) 完成"删除全部相等元素"
    std::vector<int> bag{1, 2, 3, 2, 1, 2};
    auto removed = std::erase(bag, 2);
    std::cout << "std::erase removed " << removed << " twos, now size=" << bag.size() << '\n';

    return 0;
}
