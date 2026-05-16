// C++23：范围、pair/tuple 等通过 std::format 与 format_kind 的一体化格式化。
//
// 关键点：
//   需编译器开启 C++23 且定义 __cpp_lib_format_ranges；
//   可将 vector、initializer_list 形式的序列与嵌套 tuple 写成可读的单行调试输出。

#include <format>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

void printSep() {
    std::cout << "---\n";
}

#if defined(__cpp_lib_format_ranges) && (__cpp_lib_format_ranges >= 202207L)

void demoSequence() {
    std::vector<int> v{3, 1, 4, 1, 5};
    std::cout << std::format("vector<int>: {}\n", v);
}

void demoNested() {
    std::tuple<std::string, int, char> row{"gamma", 2, '!'};
    std::cout << std::format("tuple: {}\n", row);
}

void demoPairsInVector() {
    std::vector<std::pair<char, std::string>> kv{{'x', "ten"}, {'y', "why"}};
    std::cout << std::format("pairs: {}\n", kv);
}

void demoAssoc() {
    std::map<std::string, int> freq{{"a", 10}, {"b", 20}};
    std::cout << std::format("map<string,int>: {}\n", freq);
}

#endif

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
#if defined(__cpp_lib_format_ranges) && (__cpp_lib_format_ranges >= 202207L)
    demoSequence();
    printSep();
    demoNested();
    printSep();
    demoPairsInVector();
    printSep();
    demoAssoc();
#else
    std::cout << "当前环境与标准库未提供 std::formatter 的范围扩展（format_ranges）。\n";
    std::cout << "请使用 C++23 工具链并重试。\n";
#endif
    return 0;
}
