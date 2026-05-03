// C++17 / C++20 控制流人体工学改进：
//   - 带初始化器的 if / switch（C++17）
//   - switch 内使用 using enum（C++20）
//   - 带初始化器的范围 for（C++20）
//
// 初始化器把变量名约束在所属语句中，避免泄漏到外层作用域。
// 配合 std::map / std::find 这种返回迭代器的接口尤其顺手。

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

enum class Status : std::uint8_t { Ok, Warn, Err };

}  // namespace

int main() {
    std::map<std::string, int> const scores{{"Alice", 90}, {"Bob", 72}};

    if (auto const it = scores.find("Alice"); it != scores.end()) {
        std::cout << it->first << " = " << it->second << '\n';
    } else {
        std::cout << "not found\n";
    }

    Status const status = Status::Warn;
    switch (status) {
        using enum Status;
        case Ok:
            std::cout << "ok\n";
            break;
        case Warn:
            std::cout << "warn\n";
            break;
        case Err:
            std::cout << "err\n";
            break;
    }

    // 带初始化器的范围 for —— 临时 vec 仅在循环范围内存活，
    // 循环结束立即销毁，不污染外层作用域。
    for (auto const vec = std::vector{1, 2, 3, 4}; auto const& n : vec) {
        std::cout << n << ' ';
    }
    std::cout << '\n';

    return 0;
}
