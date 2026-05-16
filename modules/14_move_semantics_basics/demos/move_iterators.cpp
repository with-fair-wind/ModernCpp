// std::move_iterator / std::make_move_iterator：把「读取」转为移动语义，
// 常用于在未改名算法里搬运元素。

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

void demoMakeMoveIterator() {
    std::vector<std::string> src{"alfa", "bravo", "charlie"};
    std::vector<std::string> dst;
    dst.reserve(src.size());

    std::copy(std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()),
              std::back_inserter(dst));

    std::cout << "[make_move_iterator + copy]\n";
    std::cout << "  dst[0]=" << dst[0] << '\n';
    std::cout << "  src[0] moved-from empty? " << (src[0].empty() ? "yes" : "no") << '\n';
}

void demoMoveIteratorType() {
    std::vector<std::string> pool{"one", "two"};
    std::vector<std::string> sink(2);

    auto first = std::move_iterator(pool.begin());
    auto last = std::move_iterator(pool.end());
    std::copy(first, last, sink.begin());

    std::cout << "[move_iterator + copy]\n";
    std::cout << "  sink[1]=" << sink[1] << '\n';
    std::cout << "  pool[1] empty? " << (pool[1].empty() ? "yes" : "no") << '\n';
}

}  // namespace

int main() {
    demoMakeMoveIterator();
    demoMoveIteratorType();
    return 0;
}
