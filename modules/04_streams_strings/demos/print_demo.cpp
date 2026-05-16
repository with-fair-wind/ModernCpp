// C++23 <print>：向流打印且避免手动换行遗漏；本文件以 cout 为主线说明语义。
//
// 关键点：
//   在满足 __cpp_lib_print 的实现上可向 std::ostream 写入；
//   println 等价于在行尾追加 `\n`，不刷新缓冲（除非 tying 触发）。
//   MinGW + libstdc++ 目前仍缺少终端相关符号（__open_terminal 等），即使宏已定义也会链接失败，
//   因此该组合下回退到说明性输出。

#if defined(__MINGW32__) && defined(__GLIBCXX__)
#define MCPP_LIBSTDCPP_STREAM_PRINT_UNSUPPORTED 1
#endif

#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && \
    !defined(MCPP_LIBSTDCPP_STREAM_PRINT_UNSUPPORTED)
#define MCPP_HAVE_STREAM_PRINT 1
#endif

#include <iostream>
#include <sstream>
#include <string>

#ifdef MCPP_HAVE_STREAM_PRINT
#include <print>
#endif

namespace {

void printSep() {
    std::cout << "---\n";
}

#ifdef MCPP_HAVE_STREAM_PRINT

void demoPrintToOstream() {
    std::ostringstream oss;
    std::print(oss, "{} + {} = {}", 21, 21, 42);
    std::cout << "ostringstream 捕获: \"" << oss.str() << "\"\n";
}

void demoPrintlnToOstream() {
    std::ostringstream oss;
    std::println(oss, "{} 行{}", "println", '尾');
    std::cout << "ostringstream 捕获（含末尾换行）字面值: \"" << oss.str() << "\"\n";
}

#else

void reportPrintUnavailable() {
#ifdef MCPP_LIBSTDCPP_STREAM_PRINT_UNSUPPORTED
    std::cout << "检测到 MinGW libstdc++：虽有 "
                 "__cpp_lib_print，但终端写入符号缺失导致链接失败，演示跳过。\n";
#elifndef __cpp_lib_print
    std::cout << "当前翻译单元未见 __cpp_lib_print，跳过 std::print/println API 演示。\n";
#else
    std::cout << "当前工具链不满足可用的 stream print 组合。\n";
#endif
}

#endif

}  // namespace

int main() {
    std::cout << "iostream 仍为通用基座；print/println 在完整支持的 libc++/MSVC STL 上体验最佳。\n";
    printSep();

#ifdef MCPP_HAVE_STREAM_PRINT
    demoPrintToOstream();
    printSep();
    demoPrintlnToOstream();
#else
    reportPrintUnavailable();
#endif

    std::cout << "\n建议在 MSVC STL / libc++ 环境验证 println(std::cerr, \"...\");\n";
    return 0;
}
