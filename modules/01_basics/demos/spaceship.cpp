// 三向比较运算符 `<=>` 的默认实现（C++20）。
//
// `operator<=> = default` 自动合成 <、<=、>、>= 四个运算；额外
// `= default` 一个 operator==，就一并得到 == 与 !=。结果的"序类别"
// 取所有成员中最弱的一种：只要含有 double 成员，结果就降级到
// std::partial_ordering（NaN 之间是无序的）。

#include <compare>
#include <iostream>
#include <limits>

namespace {

struct Point {
    int x;
    int y;
    auto operator<=>(Point const&) const = default;     // strong_ordering
};

struct Mass {
    double kg;
    auto operator<=>(Mass const&) const = default;      // partial_ordering
};

}  // namespace

int main() {
    constexpr Point kA{1, 2};
    constexpr Point kB{1, 3};

    std::cout << std::boolalpha
              << "a <  b = " << (kA < kB) << '\n'
              << "a == b = " << (kA == kB) << '\n'
              << "a != b = " << (kA != kB) << '\n';

    constexpr auto kCat = kA <=> kB;
    std::cout << "is_lt(a<=>b) = " << std::is_lt(kCat) << '\n';

    Mass const nan_mass{std::numeric_limits<double>::quiet_NaN()};
    Mass const one_kg{1.0};
    auto const fp_cat = nan_mass <=> one_kg;
    std::cout << "nan <  1kg     = " << (nan_mass < one_kg) << '\n'
              << "nan == 1kg     = " << (nan_mass == one_kg) << '\n'
              << "fp_cat unordered = "
              << (fp_cat == std::partial_ordering::unordered) << '\n';

    return 0;
}
