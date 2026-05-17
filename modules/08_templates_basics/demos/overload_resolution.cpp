// 函数模板与普通函数的重载决议：在可行集相同转换等级下，非模板优先于模板实例。
//
// 若存在更匹配的非模板重载，编译器不会盲目选择函数模板。

#include <iostream>
#include <string>

namespace {

void describe(int /*unused*/) {
    std::cout << "non-template void(int)\n";
}

template <typename T>
void describe(T /*unused*/) {
    std::cout << "template void(T)\n";
}

void describe(std::string const& /*unused*/) {
    std::cout << "non-template void(string const&)\n";
}

template <>
void describe<double>(double /*unused*/) {
    std::cout << "explicit specialization void(double)\n";
}

}  // namespace

int main() {
    describe(42);                   // 非模板 void(int) 更契合整型字面量。
    describe(3.14F);                // 无精确非模板：走模板，浮点实参推导为 float。
    describe(2.71);                 // 全特化 describe<double> 参与实例化族。
    describe(std::string{"text"});  // 实参已为 string，非模板重载直接命中。

    return 0;
}
