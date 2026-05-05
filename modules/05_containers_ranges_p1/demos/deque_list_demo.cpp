// deque：双端队列；list：双向链表 —— 各自独有 API 的演示。
//
// 关键点：
//   - deque 提供 push_front/push_back 双端 O(1) 操作 + 随机访问；
//     底层是"块（block）数组"，所以与 vector 不同，从前端推入不需要整体搬动。
//   - list 是基于节点的双向链表，没有随机访问，但 splice / merge / sort
//     可以在 O(1) 拼接整个区间（splice）或在 O(n log n) 内归并排序。
//   - splice 把节点从一个 list 移到另一个 list，不发生拷贝；
//     这是 list 相对 vector 的核心优势之一。

#include <deque>
#include <iostream>
#include <list>

namespace {

template <class Container>
void dump(const char* tag, const Container& c) {
    std::cout << tag << ": ";
    for (auto const& x : c) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    // === deque ===
    std::deque<int> dq;
    dq.push_back(2);
    dq.push_back(3);
    dq.push_front(1);
    dq.push_front(0);
    dump("deque after 4 pushes  ", dq);  // 0 1 2 3
    std::cout << "deque[2]              = " << dq[2] << " (随机访问)\n";

    // 端点元素的引用在再次从端点 push 时仍然有效（块本身不变）
    int& back_ref = dq.back();
    dq.push_front(-1);
    std::cout << "back ref still valid? " << back_ref << " (= dq.back() after push_front)\n";

    // === list ===
    std::list<int> a{1, 3, 5, 7};
    std::list<int> b{2, 4, 6, 8};

    // sort + merge：两个已排序链表合成一个（节点直接转移）
    a.merge(b);
    dump("list a after merge(b)", a);
    std::cout << "list b size after merge = " << b.size() << " (节点已被移走)\n";

    // splice：把整个 list 的节点接到 pos 之前
    std::list<int> head{100, 101};
    std::list<int> tail{200, 201};
    head.splice(head.end(), tail);
    dump("head after splice end ", head);
    std::cout << "tail size = " << tail.size() << '\n';

    // remove_if / unique：list 自带 O(n) 版本，比 erase + algorithm 高效
    std::list<int> nums{1, 1, 2, 2, 2, 3, 4, 4};
    nums.unique();  // 相邻去重 → 1 2 3 4
    dump("after unique          ", nums);
    nums.remove_if([](int x) { return x % 2 == 0; });
    dump("after remove_if even  ", nums);

    // sort：list 自带稳定归并排序（O(1) 额外空间）
    std::list<int> unsorted{4, 2, 5, 1, 3};
    unsorted.sort();
    dump("list::sort            ", unsorted);

    return 0;
}
