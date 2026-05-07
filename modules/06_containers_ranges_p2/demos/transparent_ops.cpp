// 异构查找：std::less<> / std::equal_to<> + 自定义 transparent hash。
//
// 关键点：
//   - 不带模板参数的 std::less<> / std::greater<> ... 都是"transparent"的：
//     它们提供模板版 operator()，能直接比较任意可比较类型。
//   - std::set<std::string, std::less<>>::find("c-str") 不会再为 "c-str" 构造一个临时
//     std::string；这是异构（heterogeneous）查找的核心收益。
//   - 让 unordered 容器也支持异构查找需要 *两个* 改动：传入 transparent 比较器（如
//     std::equal_to<>）+ 自定义 transparent 哈希——通常基于 std::string_view 实现。
//   - 自定义 transparent hash 必须保证"等价的不同类型 → 相同哈希值"。

#include <cstddef>
#include <functional>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>

namespace {

// 1) 一个 transparent 哈希器：通过 string_view 统一所有"字符串型"key 的哈希
struct StringHash {
    using is_transparent = void;  // 让标准库识别为 transparent

    std::size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
    std::size_t operator()(const std::string& s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
    std::size_t operator()(const char* s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
};

}  // namespace

int main() {
    // 1) 旧 set<string>：contains("c-str") 仍可用，但内部会先构造临时 std::string
    std::set<std::string> classic{"alpha", "beta", "gamma"};
    std::cout << "classic contains   : " << classic.contains("alpha") << '\n';

    // 2) set<string, std::less<>>：异构查找——传入的 "key 候选" 不再被强制转换
    std::set<std::string, std::less<>> hetero{"alpha", "beta", "gamma"};
    std::cout << "hetero contains    : " << hetero.contains("alpha") << '\n';
    std::cout << "hetero contains sv : " << hetero.contains(std::string_view{"beta"}) << '\n';

    // 3) unordered_set 异构查找：必须同时给 transparent 哈希 + 透明等价比较器
    std::unordered_set<std::string, StringHash, std::equal_to<>> uset{"alpha", "beta", "gamma"};
    std::cout << "uset contains c-str: " << uset.contains("alpha") << '\n';
    std::cout << "uset contains sv   : " << uset.contains(std::string_view{"beta"}) << '\n';

    // 4) 异构 lower_bound / upper_bound 也成立
    auto lb = hetero.lower_bound("b");  // 找到第一个 >= "b"
    std::cout << "lower_bound(\"b\") -> " << *lb << '\n';

    return 0;
}
