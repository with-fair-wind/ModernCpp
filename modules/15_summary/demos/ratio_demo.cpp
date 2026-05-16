// std::ratio：编译期有理数比例，常用于 duration / chrono 精度推导。

#include <iostream>
#include <ratio>
#include <type_traits>

namespace {

template <typename R>
void printRatio(char const* label) {
    std::cout << label << ": " << R::num << " / " << R::den << '\n';
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    printRatio<std::ratio_add<std::ratio<1, 3>, std::ratio<1, 6>>>("1/3 + 1/6");

    using mul_t = std::ratio_multiply<std::kilo, std::milli>;
    printRatio<mul_t>("kilo * milli");

    using sum_scaled = std::ratio_add<std::milli, std::micro>;
    using halved = std::ratio_divide<sum_scaled, std::ratio<2>>;
    printRatio<halved>("(milli + micro) / 2");

    using sub_t = std::ratio_subtract<std::ratio<5, 6>, std::ratio<1, 3>>;
    printRatio<sub_t>("5/6 - 1/3");

    static_assert(std::is_same_v<mul_t, std::ratio<1>>);
    static_assert(sub_t::num == 1 && sub_t::den == 2);

    return 0;
}
