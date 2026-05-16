// std::ranges::move / std::move（三迭代器）、move_backward，
// 以及与 remove / unique 的典型组合用法。

#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace {

void demoRangesMove() {
    std::vector<std::string> src{"alfa", "bravo", "charlie", "delta", "echo"};
    std::vector<std::string> dst{};
    dst.resize(src.size());

    [[maybe_unused]] auto const moved = std::ranges::move(src, dst.begin());
    auto const out_end = moved.out;

    std::cout << "[ranges::move] dst: ";
    for (auto const& s : dst) {
        std::cout << s << ' ';
    }
    std::cout << "\n              src 首部 moved-from 通常为空串? ";
    if (!src.empty()) {
        std::cout << "front.empty=" << (src.front().empty() ? "yes" : "no");
    } else {
        std::cout << "(empty)";
    }
    std::cout << ", out 抵达偏移: " << static_cast<std::ptrdiff_t>(out_end - dst.begin()) << '\n';
}

void demoMoveBackward() {
    std::vector<int> buf{10, 20, 30, 40, 50, 0, 0, 0};
    // 将前三元素搬到缓冲区尾部（重叠区间必须用 move_backward）。
    auto const first = buf.begin();
    auto const mid = buf.begin() + 3;
    auto const dest_end = buf.end();
    std::move_backward(first, mid, dest_end);

    std::cout << "[move_backward] buf: ";
    for (int x : buf) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

void demoRemoveUniqueMoves() {
    std::vector<std::string> words{"a", "b", "b", "c", "c", "c", "d"};
    // 故意使用 classic unique + 迭代器对，配合后续 erase；演示与 ranges 写法等价语义。
    auto const logical_end =
        std::unique(words.begin(), words.end());  // NOLINT(modernize-use-ranges) 移动相邻重复元素

    std::cout << "[unique 区间长度] " << static_cast<std::ptrdiff_t>(logical_end - words.begin())
              << '\n';
    std::cout << "[unique 后容器 size 仍为] " << words.size() << " —— erase 才真正缩小\n";

    words.erase(logical_end, words.end());

    std::cout << "[erase-remove 后] ";
    for (auto const& w : words) {
        std::cout << w << ' ';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    demoRangesMove();
    demoMoveBackward();
    demoRemoveUniqueMoves();
    return 0;
}
