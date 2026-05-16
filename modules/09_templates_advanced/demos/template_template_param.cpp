// 模板模板形参：把「外层容器模板」作为参数，再在实例化时绑定元素类型 T。
//
// STL 容器多为「至少两个模板形参」（如 allocator），因此外层常用
// `template <template <typename...> class Outer>`；若是单参模板亦可写成
// `template <template <typename> class Container, typename T>`。

#include <deque>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {

constexpr int kSeed = 42;

// 变参形式的模板模板形参：适配 `std::vector` 等「带默认实参的多参类模板」。
template <template <typename...> class Outer, typename T>
[[nodiscard]] Outer<T> duplicateValue(T value) {
    Outer<T> out;
    out.push_back(value);
    out.push_back(value);
    return out;
}

// 严格的单形参类模板：`template <typename> class Container`。
template <typename Elem>
struct Bag {
    std::vector<Elem> store_{};
    void pushBack(Elem v) { store_.push_back(std::move(v)); }
};

template <template <typename> class Container, typename T>
[[nodiscard]] Container<T> duplicateStrict(T value) {
    Container<T> out;
    out.pushBack(value);
    out.pushBack(value);
    return out;
}

}  // namespace

int main() {
    auto vec = duplicateValue<std::vector, int>(kSeed);
    auto deq = duplicateValue<std::deque, int>(kSeed);
    auto bag = duplicateStrict<Bag, int>(kSeed);

    std::cout << "vector duplicate: ";
    for (int x : vec) {
        std::cout << x << ' ';
    }
    std::cout << "\ndeque duplicate : ";
    for (int x : deq) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    std::cout << "Bag (template<typename> class) : ";
    for (int x : bag.store_) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    static_assert(std::is_same_v<decltype(vec), std::vector<int>>);
    static_assert(std::is_same_v<decltype(deq), std::deque<int>>);
    static_assert(std::is_same_v<decltype(bag), Bag<int>>);

    return 0;
}
