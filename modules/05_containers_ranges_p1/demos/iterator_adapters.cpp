// 迭代器适配器：reverse_iterator、back/front/insert_iterator，以及流迭代器。
//
// 关键点：
//   - reverse_iterator 物理底层指向"当前所引用元素之后"的位置；
//     rit.base() 返回的就是这个底层迭代器，因此 rbegin().base() == end()。
//   - back_inserter / front_inserter / inserter 都是输出迭代器：
//     `*it = val` 实际触发 push_back / push_front / insert。
//   - ostream_iterator / istream_iterator 把流伪装成迭代器，便于和算法组合。
//   - 注意：对 vector / deque，把整段 range 一次性插入通常比 inserter 一个一个插入更高效。

#include <algorithm>
#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <vector>

int main() {
    std::vector<int> src{1, 2, 3, 4, 5};

    // 1) reverse_iterator —— ++ 实际是 --
    std::cout << "reverse iterate vec : ";
    // 教学示例：本节重点演示 reverse_iterator 的传统迭代器写法，
    // 故意保留显式 rbegin()/rend() 循环，不改成 range-based for。
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (auto it = src.rbegin(); it != src.rend(); ++it) {
        std::cout << *it << ' ';
    }
    std::cout << '\n';

    // base() 把反向迭代器转成正向迭代器
    auto r = src.rbegin();
    ++r;  // 反向第二个 → 正向倒数第二
    std::cout << "rbegin().base() points one-past-end: " << (src.rbegin().base() == src.end())
              << '\n';
    std::cout << "after ++rbegin, base() points to    : " << *r.base() << '\n';

    // 2) back_inserter —— *it = val 等价于 push_back(val)
    // 教学意图：演示传统三参版 std::copy 与输出迭代器适配器的搭配，
    // 故意保留 (begin, end, out) 形态，不改成 std::ranges::copy。
    std::vector<int> sink;
    // NOLINTNEXTLINE(modernize-use-ranges)
    std::copy(src.begin(), src.end(), std::back_inserter(sink));
    std::cout << "after back_inserter copy, sink.size = " << sink.size() << '\n';

    // front_inserter —— 元素会被反序压入（因为每个 push_front 都是头插）
    std::deque<int> dq;
    // NOLINTNEXTLINE(modernize-use-ranges)
    std::copy(src.begin(), src.end(), std::front_inserter(dq));
    std::cout << "after front_inserter copy, dq[0] = " << dq.front() << ", dq.back = " << dq.back()
              << '\n';

    // inserter —— 每次插入到指定位置之前
    std::list<int> lst{10, 99};
    // NOLINTNEXTLINE(modernize-use-ranges)
    std::copy(src.begin(), src.end(), std::inserter(lst, std::next(lst.begin())));
    std::cout << "after inserter list  : ";
    for (int v : lst) {
        std::cout << v << ' ';
    }
    std::cout << '\n';

    // 3) 流迭代器：ostream_iterator 配合 std::copy 打印
    std::cout << "via ostream_iterator: ";
    // NOLINTNEXTLINE(modernize-use-ranges)
    std::copy(src.begin(), src.end(), std::ostream_iterator<int>{std::cout, " "});
    std::cout << '\n';

    // istream_iterator：从字符串流读入直到流不可解析
    std::istringstream iss{"7 8 9 oops"};
    std::vector<int> read;
    // istream_iterator 的两端就是经典 (begin, end) 写法，没有 ranges 直接对应。
    // NOLINTNEXTLINE(modernize-use-ranges)
    std::copy(std::istream_iterator<int>{iss}, std::istream_iterator<int>{},
              std::back_inserter(read));
    std::cout << "from istream_iterator (stops at 'oops'), read.size = " << read.size() << '\n';

    return 0;
}
