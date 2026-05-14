// extern "C" 测试的实现 TU —— 提供 C ABI 函数定义。

#include <string>

#include "c_api.h"

// C API 状态需要可变的文件级变量
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
namespace {
std::string fmt_buf;
int acc_val = 0;
}  // namespace
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// NOLINTBEGIN(readability-identifier-naming)
extern "C" {

int c_api_add(int a, int b) {
    return a + b;
}

int c_api_subtract(int a, int b) {
    return a - b;
}

int c_api_multiply(int a, int b) {
    return a * b;
}

const char* c_api_format_int(int value) {
    fmt_buf = "int(" + std::to_string(value) + ")";
    return fmt_buf.c_str();
}

void c_api_accumulator_reset(void) {
    acc_val = 0;
}

void c_api_accumulator_add(int value) {
    acc_val += value;
}

int c_api_accumulator_get(void) {
    return acc_val;
}

}  // extern "C"
// NOLINTEND(readability-identifier-naming)
