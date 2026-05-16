// std::format 常用格式说明符：进制、对齐、填充、宽度与精度演示。
//
// 关键点：
//   替换域形如 `{ [...] ':' 格式说明 ] }`；
//   对齐符号 `<`、`>`、`^` 与填充字符、`#`、宽度、精度可组合；
//   本地化与 iostream manipulator 不同，format 默认为“类 sprintf”风格。

#include <format>
#include <iostream>
#include <string>

namespace {

void printSep() { std::cout << "---\n"; }

void demoIntSpec() {
    int n = -42;
    std::cout << std::format("decimal {:d}\n", n);
    std::cout << std::format("hex     {:x} / {:X}\n", 255, 255);
    std::cout << std::format("oct     {:o}\n", 9);
    std::cout << std::format("带 # 前缀 {:#x} {:#o}\n", 255, 9);
}

void demoAlignFill() {
    std::cout << std::format("右对齐 {:>10}\n", 7);
    std::cout << std::format("左对齐 {:<10}\n", 7);
    std::cout << std::format("居中   {:^10}\n", 7);
    std::cout << std::format("填充   {:*>8}\n", 42);
    std::cout << std::format("字符串 {:_^12}\n", "ab");
}

void demoFloatSpec() {
    double x = 12.3456;
    std::cout << std::format("默认     {}\n", x);
    std::cout << std::format("fixed    {:.2f}\n", x);
    std::cout << std::format("scientific {:e}\n", x);
    std::cout << std::format("宽度精度 {:10.3f}\n", x);
}

void demoBoolString() {
    std::cout << std::format("bool 默认 {}\n", true);
    std::cout << std::format("bool 文本 {:s}\n", false);
    std::string name = "format";
    std::cout << std::format("转义花括号 {{}} 示例: {{}} -> {}\n", name);
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    demoIntSpec();
    printSep();
    demoAlignFill();
    printSep();
    demoFloatSpec();
    printSep();
    demoBoolString();
    return 0;
}
