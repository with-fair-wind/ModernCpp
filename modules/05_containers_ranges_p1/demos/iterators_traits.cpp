// 迭代器类别（concept）、迭代器特征（traits），以及通用算法
// std::advance / std::next / std::prev / std::distance 的最小演示。
//
// 关键点：
//   - C++20 起，迭代器要求由 concept 描述（input_iterator、forward_iterator、
//     bidirectional_iterator、random_access_iterator、contiguous_iterator）；
//     concept 失败时编译错误信息比旧式 SFINAE 友好得多。
//   - std::advance 在非随机访问迭代器上需要逐次 ++/--，因此对 list 是 O(n)；
//     对 vector 才是 O(1)。
//   - 指针也是迭代器；C 风格数组 + std::begin/end 同样适用。

#include <forward_list>
#include <iostream>
#include <iterator>
#include <list>
#include <type_traits>
#include <vector>

namespace {

template <class It>
constexpr const char* categoryName() {
    if constexpr (std::contiguous_iterator<It>) {
        return "contiguous";
    } else if constexpr (std::random_access_iterator<It>) {
        return "random_access";
    } else if constexpr (std::bidirectional_iterator<It>) {
        return "bidirectional";
    } else if constexpr (std::forward_iterator<It>) {
        return "forward";
    } else if constexpr (std::input_iterator<It>) {
        return "input";
    } else {
        return "(none)";
    }
}

}  // namespace

int main() {
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::list<int> lst{1, 2, 3, 4, 5};
    std::forward_list<int> flist{1, 2, 3};
    int arr[3]{10, 20, 30};  // C 风格数组：指针就是迭代器

    std::cout << "vector<int>::iterator        -> " << categoryName<decltype(vec.begin())>()
              << '\n';
    std::cout << "list<int>::iterator          -> " << categoryName<decltype(lst.begin())>()
              << '\n';
    std::cout << "forward_list<int>::iterator  -> " << categoryName<decltype(flist.begin())>()
              << '\n';
    std::cout << "int*                         -> " << categoryName<int*>() << '\n';

    // 迭代器特征：value_type / difference_type
    using VecIt = std::vector<int>::iterator;
    static_assert(std::is_same_v<std::iter_value_t<VecIt>, int>);
    static_assert(std::is_same_v<std::iter_reference_t<VecIt>, int&>);

    // std::advance / next / prev / distance
    auto it = vec.begin();
    std::advance(it, 2);  // 现在指向 3
    std::cout << "advance(begin, 2) = " << *it << '\n';
    std::cout << "next(begin, 4)    = " << *std::next(vec.begin(), 4) << '\n';
    std::cout << "prev(end, 1)      = " << *std::prev(vec.end(), 1) << '\n';
    std::cout << "distance(b, e)    = " << std::distance(vec.begin(), vec.end()) << '\n';

    // C 风格数组：std::begin/end 推导出指针
    std::cout << "std::begin(arr) is int*: "
              << std::is_same_v<decltype(std::begin(arr)), int*> << '\n';

    return 0;
}
