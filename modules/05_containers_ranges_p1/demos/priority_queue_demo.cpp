// 容器适配器：std::stack、std::queue、std::priority_queue。
//
// 关键点：
//   - 适配器把已有顺序容器"伪装"成另一种数据结构，因此不暴露迭代器。
//   - stack 默认底层为 deque；queue 同样默认 deque；priority_queue 默认 vector。
//   - 当数据量很大或缓存关系明显时，stack 可以选择 vector 作为底层（缓存更好）。
//   - priority_queue 默认是 max-heap；想得到 min-heap 必须显式传入 std::greater。
//   - priority_queue 的构造函数把比较器放在第一个参数，这是它的特殊之处。

#include <functional>
#include <iostream>
#include <queue>
#include <stack>
#include <vector>

int main() {
    // === stack 用 vector 作底层 ===
    std::stack<int, std::vector<int>> s;
    s.push(1);
    s.push(2);
    s.push(3);
    std::cout << "stack top=" << s.top() << ", size=" << s.size() << '\n';
    s.pop();
    std::cout << "after pop, top=" << s.top() << '\n';

    // === queue ===
    std::queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    std::cout << "queue front=" << q.front() << ", back=" << q.back() << '\n';
    q.pop();
    std::cout << "after pop, front=" << q.front() << '\n';

    // === priority_queue 默认是 max-heap ===
    std::priority_queue<int> max_heap;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) {
        max_heap.push(x);
    }
    std::cout << "max-heap pop order: ";
    while (!max_heap.empty()) {
        std::cout << max_heap.top() << ' ';
        max_heap.pop();
    }
    std::cout << '\n';

    // === min-heap：第三个模板参数填 std::greater ===
    std::priority_queue<int, std::vector<int>, std::greater<>> min_heap;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) {
        min_heap.push(x);
    }
    std::cout << "min-heap pop order: ";
    while (!min_heap.empty()) {
        std::cout << min_heap.top() << ' ';
        min_heap.pop();
    }
    std::cout << '\n';

    // === 由迭代器对构造（C++11 起，priority_queue 提供）===
    // Floyd 建堆 O(n)，比逐个 push 的 O(n log n) 更便宜。
    std::vector<int> data{8, 3, 7, 2, 9, 1};
    std::priority_queue<int> built{std::less<int>{}, data};  // 拷贝构造底层容器并堆化
    std::cout << "priority_queue built from vector, top=" << built.top() << '\n';

    return 0;
}
