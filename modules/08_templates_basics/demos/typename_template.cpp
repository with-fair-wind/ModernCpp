// typename 与 template 关键字：在待决上下文中告诉编译器如何解析嵌套从属名。
//
// 从属名（dependent name）可能是类型、模板或值，需要显式消歧义。

#include <iostream>
#include <vector>

namespace {

template <typename Container>
void printSecond(Container const& c) {
    // `Container::const_iterator` 需 typename，否则编译器默认当作值。
    auto it = c.begin();
    if (it != c.end()) {
        ++it;
    }
    if (it != c.end()) {
        std::cout << "second element: " << *it << '\n';
    } else {
        std::cout << "no second element\n";
    }
}

template <typename T>
struct Holder {
    template <int N>
    struct Inner {
        static constexpr int kValue = N;
    };
};

template <typename H, int N>
constexpr int extractInnerValue() {
    // `template` 关键字表明 Inner 是成员模板，而不是与比较符 `<` 混淆。
    return H::template Inner<N>::kValue;
}

}  // namespace

int main() {
    std::vector<int> data{10, 20, 30};
    printSecond(data);

    constexpr int kV = extractInnerValue<Holder<int>, 7>();
    static_assert(kV == 7);
    std::cout << "Holder::Inner<7>::value = " << kV << '\n';

    return 0;
}
