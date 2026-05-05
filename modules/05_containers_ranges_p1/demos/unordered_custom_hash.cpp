// std::unordered_map：自定义类型作为键时的两种哈希接入方式，以及桶层接口。
//
// 关键点：
//   - unordered_map 需要 Hash + KeyEqual 两个仿函数。
//   - 接入方式 A：定义一个带 operator() 的类型，作为模板参数显式传入。
//   - 接入方式 B：特化 std::hash<MyType>，从而能像 std::string 一样直接当键用。
//   - 负载因子（load_factor = size / bucket_count）超过 max_load_factor
//     就会触发 rehash；rehash 会让所有迭代器失效（基于节点，引用仍有效）。
//   - bucket_count() / bucket(key) / bucket_size(i) 提供了一窥哈希分布的接口。

#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {

// 自定义聚合体作为键
struct Point {
    int x;
    int y;
    bool operator==(const Point&) const = default;
};

// 方式 A：仿函数
struct PointHash {
    std::size_t operator()(Point const& p) const noexcept {
        // 朴素做法：异或；正经设计哈希函数请参考 Boost.hash_combine 等。
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1U);
    }
};

}  // namespace

// 方式 B：特化 std::hash —— 必须在命名空间 std 内
template <>
struct std::hash<Point> {
    std::size_t operator()(Point const& p) const noexcept {
        return std::hash<int>{}(p.x) * 31U + std::hash<int>{}(p.y);
    }
};

int main() {
    // 方式 A
    std::unordered_set<Point, PointHash> via_functor;
    via_functor.insert({1, 2});
    via_functor.insert({3, 4});
    std::cout << "via PointHash, size=" << via_functor.size() << '\n';

    // 方式 B：直接用，无需显式传 Hash
    std::unordered_map<Point, std::string> labels;
    labels[{0, 0}] = "origin";
    labels[{1, 1}] = "diag";
    std::cout << "labels[{0,0}] = " << labels.at({0, 0}) << '\n';

    // 负载因子与 reserve
    std::unordered_map<int, int> table;
    std::cout << "default max_load_factor = " << table.max_load_factor()
              << ", initial bucket_count = " << table.bucket_count() << '\n';

    // reserve(N) ≈ rehash(ceil(N / max_load_factor))，保证 N 之前不再 rehash
    table.reserve(1000);
    std::cout << "after reserve(1000), bucket_count = " << table.bucket_count() << '\n';

    for (int i = 0; i < 64; ++i) {
        table.emplace(i, i * i);
    }
    std::cout << "load_factor = " << table.load_factor() << '\n';

    // 桶接口：观察某一桶中的元素
    std::size_t bidx = table.bucket(7);
    std::cout << "key 7 lives in bucket #" << bidx << ", bucket_size=" << table.bucket_size(bidx)
              << '\n';

    return 0;
}
