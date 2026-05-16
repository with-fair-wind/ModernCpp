// 类模板实参推导（CTAD）：构造函数实参可推导类模板参数；
// 推导指引（deduction guide）为「非构造函数」场景提供补充规则。

#include <iostream>
#include <tuple>
#include <utility>

namespace {

template <typename Head, typename Tail>
struct Tracker {
    Head head;
    Tail tail;
    Tracker(Head head_in, Tail tail_in) : head(std::move(head_in)), tail(std::move(tail_in)) {}
};

// 用户自定义推导指引：把同类型实参绑到 `Tracker<T, T>`。
template <typename T>
Tracker(T, T) -> Tracker<T, T>;

}  // namespace

int main() {
    std::pair numbers{1, 2.0};  // std::pair<int, double>
    std::tuple triple{1, 2, 3};   // std::tuple<int, int, int>

    Tracker mixed{std::string{"hi"}, 3.14};  // Tracker<std::string, double>
    Tracker same{8, 9};                        // 指引参与 → Tracker<int, int>

    std::cout << "pair: " << numbers.first << ", " << numbers.second << '\n';
    std::cout << "tracker mixed: " << mixed.head << ", " << mixed.tail << '\n';
    std::cout << "tracker same : " << same.head << ", " << same.tail << '\n';
    std::cout << "tuple size   : " << std::tuple_size_v<decltype(triple)> << '\n';

    return 0;
}
