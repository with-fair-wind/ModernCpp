// Ranges 的拼接与切分：join / join_with / split / lazy_split。
//
// 关键点：
//   - join：把"range 的 range"压平为单层 range。
//   - join_with：在元素之间插入分隔符；拼字符串时尤其有用。
//   - split：必须是 random_access_range 才能用，结果会尽量保持原 range 的类别；
//   - lazy_split：总是返回 forward_range；新 view 不能调用 .size() / data()。
//   - 数组 decay 的坑：" " 作为字面量是 char[3]，传到 join_with 会衰退成 char* 而失去 end，
//     必须用 std::string{" "} 或 std::string_view{" "} 显式传值。

#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace stdv = std::views;
namespace stdr = std::ranges;

int main() {
    // 1) join：把嵌套 vector 压平
    std::vector<std::vector<int>> nested{{1, 2}, {3, 4, 5}, {6}, {7, 8, 9}};
    std::cout << "join         : ";
    for (int x : nested | stdv::join) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 2) join_with(value)：在每段之间插入指定值
    std::cout << "join_with(0) : ";
    for (int x : nested | stdv::join_with(0)) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 3) join_with 拼字符串：注意"分隔符"也可以是 range
    std::vector<std::string> words{"Are", "you", "Okay?"};
    std::string joined;
    for (char c : words | stdv::join_with(std::string{" "})) {
        joined.push_back(c);
    }
    std::cout << R"(join_with(" "): ")" << joined << "\"\n";

    // 4) split：以分隔符把 range 切成多段
    std::string sentence{"hello cmake ranges world"};
    std::cout << "split(' ')   : ";
    for (auto word : sentence | stdv::split(' ')) {
        std::string_view sv{word.begin(), word.end()};
        std::cout << '[' << sv << "] ";
    }
    std::cout << '\n';

    // 5) split 也能用多字符分隔符
    std::string csv{"a,,b,c"};
    std::cout << "split(',')   : ";
    for (auto field : csv | stdv::split(',')) {
        std::string_view sv{field.begin(), field.end()};
        std::cout << '[' << sv << "] ";
    }
    std::cout << '\n';

    // 6) lazy_split：与 split 类似，但结果只是 forward_range，无法 .size() / .data()
    std::cout << "lazy_split   : ";
    for (auto word : sentence | stdv::lazy_split(' ')) {
        for (char c : word) {
            std::cout << c;
        }
        std::cout << ' ';
    }
    std::cout << '\n';

    // 7) split + transform：常用于做 "trim & count"
    auto pieces = sentence | stdv::split(' ')
                  | stdv::transform([](auto sub) { return stdr::distance(sub); });
    std::cout << "lengths      : ";
    for (auto n : pieces) {
        std::cout << n << ' ';
    }
    std::cout << '\n';

    return 0;
}
