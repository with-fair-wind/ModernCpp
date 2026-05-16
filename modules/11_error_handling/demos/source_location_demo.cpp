// 模块 11 的小演示：std::source_location 记录调用点；可选展示 std::stacktrace。

#include <iostream>
#include <source_location>
#include <string_view>

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif

namespace {

void logHere(std::string_view message, std::source_location loc = std::source_location::current()) {
    std::cout << message << '\n';
    std::cout << "  file: " << loc.file_name() << '\n';
    std::cout << "  line: " << loc.line() << ", column: " << loc.column() << '\n';
    std::cout << "  function: " << loc.function_name() << '\n';
}

void maybePrintStackTrace() {
#ifdef __cpp_lib_stacktrace
    auto const trace = std::stacktrace::current(0, 6);
    std::cout << "--- std::stacktrace（截取前几帧）---\n";
    std::cout << std::to_string(trace) << '\n';
#else
    std::cout << "当前工具链未提供 __cpp_lib_stacktrace，跳过 stacktrace 采样。\n";
    std::cout << "（许多实现需额外链接支持；仅供概览。）\n";
#endif
}

}  // namespace

int main() {
    std::cout << "=== source_location ===\n";
    logHere("从 main 间接调用 logHere");

    std::cout << '\n';
    maybePrintStackTrace();

    return 0;
}
