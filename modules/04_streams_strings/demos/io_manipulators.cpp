// iostream 操纵器示例：radix、对齐宽度、布尔文本、数值精度与小数风格。
//
// 关键点：
//   sticky（如 uppercase、scientific）会保持到再次修改；setw 仅在下一次输出生效；
//   格式化复杂场景优先考虑 std::format，操纵器常用于既有 iostream 生态。

#include <bitset>
#include <iomanip>
#include <ios>
#include <iostream>
#include <numbers>

namespace {

void printSep() {
    std::cout << "---\n";
}

void demoRadixBool() {
    int v = 31;
    std::cout << std::noboolalpha;
    std::cout << "dec " << std::setw(6) << v << '\n';
    std::cout << std::hex << std::uppercase << "hex " << v << '\n';
    std::cout << std::oct << "oct " << v << '\n';
    std::cout << std::dec << std::nouppercase << std::boolalpha;
    std::cout << std::setw(16) << std::left << "bool:" << std::setw(12) << true << '\n';
}

void demoFloatPrecision() {
    double constexpr kPi = std::numbers::pi;
    std::cout << std::scientific << std::setprecision(4) << "sci  : " << kPi << '\n';
    std::cout << std::fixed << std::setprecision(6) << "fixed: " << kPi << '\n';
    std::cout << std::defaultfloat << "def  : " << kPi << '\n';
}

void demoBitsetManip() {
    std::cout << std::setw(14) << "bitset:" << std::bitset<8>(0xB3U) << '\n';
}

}  // namespace

int main() {
    demoRadixBool();
    printSep();
    demoFloatPrecision();
    printSep();
    demoBitsetManip();
    std::cout << "\n记得在输出链上适当 resetiosflags，避免状态泄漏到后续日志。\n";
    return 0;
}
