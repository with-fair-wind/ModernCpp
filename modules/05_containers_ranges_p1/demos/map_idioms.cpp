// std::map 的常用习语：访问、插入、查找、节点提取。
//
// 关键点：
//   - operator[] 在键不存在时会插入默认值；读取也可能"无中生有"，因此不能用于 const map。
//   - 真正的"只查询"应使用 find / contains（C++20）/ count。
//   - 插入失败时 insert / emplace 会保持原值，try_emplace 进一步避免构造未使用的 value。
//   - insert_or_assign（C++17）在已存在时覆盖，是真正的"有则更新，无则插入"。
//   - C++17 起，extract 可以把节点摘出来；node_handle 既可单独修改键/值，也能被 insert
//     回任意同类型 map（甚至 multimap，反之亦然）。

#include <iostream>
#include <map>
#include <string>

namespace {

void dump(const char* tag, const std::map<std::string, int>& m) {
    std::cout << tag << ": ";
    for (auto const& [k, v] : m) {
        std::cout << k << '=' << v << ' ';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    std::map<std::string, int> scores;

    // 1) operator[] 会插入默认值（值类型必须可默认构造）
    ++scores["alice"];
    ++scores["alice"];
    ++scores["bob"];
    dump("after op[]++   ", scores);

    // 2) 真正的查询：find / contains
    if (auto it = scores.find("alice"); it != scores.end()) {
        std::cout << "alice = " << it->second << '\n';
    }
    std::cout << "contains carol? " << scores.contains("carol") << '\n';

    // 3) insert vs insert_or_assign vs try_emplace
    auto [it1, ok1] = scores.insert({"alice", 999});
    std::cout << "insert(alice,999) ok=" << ok1 << " (键存在则失败，仍指向旧值=" << it1->second
              << ")\n";

    scores.insert_or_assign("alice", 999);
    dump("after IOA      ", scores);

    // try_emplace：键已存在时不会构造 value（这里只是 int，但若是 unique_ptr<Texture>
    // 这种昂贵对象，try_emplace 比 emplace 更安全，因为构造可能很贵）
    scores.try_emplace("dave", 50);
    scores.try_emplace("alice", 0);  // 已存在 → 不构造 value
    dump("after try_emplace", scores);

    // 4) lower_bound / upper_bound / equal_range
    std::map<int, std::string> tag{{1, "a"}, {3, "c"}, {5, "e"}, {7, "g"}};
    auto lb = tag.lower_bound(3);  // *lb >= 3   → key=3
    auto ub = tag.upper_bound(3);  // *ub  > 3   → key=5
    std::cout << "lower_bound(3) -> key=" << lb->first << '\n';
    std::cout << "upper_bound(3) -> key=" << ub->first << '\n';

    // 5) 节点提取：把"alice"换成"alex"，无需移动 value
    if (auto node = scores.extract("alice"); !node.empty()) {
        node.key() = "alex";
        scores.insert(std::move(node));
    }
    dump("after rename   ", scores);

    // 6) 结构化绑定：遍历键值对的简洁写法
    int total = 0;
    for (auto const& [k, v] : scores) {
        (void)k;
        total += v;
    }
    std::cout << "score total = " << total << '\n';

    return 0;
}
