// std::string_view：非拥有、轻量只读序列视图。
//
// 关键点：
//   substr 与 remove_prefix/suffix 常数时间且不分配；但 view 不延长对象寿命，
//   绝不要返回「临时 std::string」上取得的 view（悬垂引用）。

#include <iostream>
#include <string>
#include <string_view>

namespace {

void printLabeled(std::string_view label, std::string_view view_value) {
    std::cout << label << " size=" << view_value.size() << " data=\"" << view_value << "\"\n";
}

}  // namespace

int main() {
    std::string owned{"hello world"};
    char stack[] = "stack buffer";
    std::string_view v0{owned};
    std::string_view v1{stack};
    std::string_view v2{"literal"};  // 字符串字面量可安全延长生命期
    printLabeled("from string", v0);
    printLabeled("from char[]", v1);
    printLabeled("from literal", v2);

    std::cout << "---\n";
    // substr 不分配：返回新的 string_view 窗口。
    auto head = v0.substr(0, 5);
    auto tail = v0.substr(6);
    printLabeled("head", head);
    printLabeled("tail", tail);

    std::cout << "---\n";
    std::string_view mut = v0;
    mut.remove_prefix(6);
    mut.remove_suffix(0);
    printLabeled("after remove_prefix(6)", mut);
    mut = v0;
    mut.remove_suffix(6);
    printLabeled("after remove_suffix(6)", mut);

    std::cout << "---\n";
    std::cout << "compare to \"hello\": " << v0.compare(0, 5, "hello") << '\n';
    std::cout << "find('o'): " << v0.find('o') << '\n';

    std::cout << "---\n";
    std::cout << "错误示范（仅解释，不要照搬到生产代码）：\n";
    // 故意演示：view 指向已析构的临时 std::string 缓冲 —— 仅教学，切勿模仿。
    std::string_view dangling = std::string{"temporary"}.substr(0, 3);  // NOLINT(bugprone-dangling-handle,clang-diagnostic-dangling-gsl)
    // dangling 现在指向已析构的临时 string 的内部缓冲 —— 未定义行为。
    // 以下输出仅用于教学说明，实际运行可能看起来“正常”或立刻崩溃。
    std::cout << "（未定义）dangling 可能输出乱数据: \"" << dangling << "\"\n";

    return 0;
}
