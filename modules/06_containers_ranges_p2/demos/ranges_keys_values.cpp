// keys / values / elements / as_const / common_view —— 元组视图与"只读化"。
//
// 关键点：
//   - keys / values / elements<i> 本质就是 elements<0/1/i>，从类元组对象里挑出某一项。
//   - 遍历 std::map 时 keys 永远是 const T&（因为 key 不可改），只读；values 可改。
//   - as_const：把 view 元素变为 const T&。注意 const view 与 view of const 不同——
//     view 自己往往要在 begin() 里写缓存，所以 const view 可能根本不能迭代。
//   - common_view (stdv::common)：把 (iterator, sentinel) 同型化，便于喂给只接受
//     common_range 的 std:: 算法。

#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace stdv = std::views;

namespace {

template <std::ranges::range R>
void dump(const char* tag, R&& r) {
    std::cout << tag << ": ";
    for (auto&& x : std::forward<R>(r)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    std::map<std::string, int> ages{{"alice", 30}, {"bob", 25}, {"carol", 40}};

    // 1) keys / values：从 map 中只取键或只取值
    dump("keys             ", ages | stdv::keys);
    dump("values           ", ages | stdv::values);

    // 2) values 可写：直接修改 map 里的整数值
    for (int& v : ages | stdv::values) {
        v += 1;
    }
    std::cout << "alice now = " << ages.at("alice") << '\n';

    // 3) elements<i>：从任意类元组容器里取第 i 列
    std::vector<std::tuple<int, std::string, double>> rows{
        {1, "x", 1.5}, {2, "y", 2.5}, {3, "z", 3.5}};
    dump("elements<0> (id) ", rows | stdv::elements<0>);
    dump("elements<2> (val)", rows | stdv::elements<2>);

    // 4) as_const：阻断后续写入；常与 transform/zip 搭配
    auto v = std::vector{1, 2, 3, 4};
    auto readonly = v | stdv::as_const;
    // readonly[0] = 99;  // 编译错：const int&
    dump("as_const         ", readonly);

    // 5) common_view：iota(1, 6) 的 end 是 sentinel；std:: 算法要求 begin/end 同类型
    auto rng = stdv::iota(1, 6) | stdv::common;
    auto std_begin = rng.begin();
    auto std_end = rng.end();
    static_assert(std::is_same_v<decltype(std_begin), decltype(std_end)>);
    int total = 0;
    for (auto x : rng) {
        total += x;
    }
    std::cout << "iota(1,6) sum via std-style = " << total << '\n';

    return 0;
}
