// extern "C" 演示：C++ 实现文件。
// 内部使用 C++ 特性（std::string、std::to_string），
// 对外通过 extern "C" 暴露 C ABI 符号。

#include <string>

#include "c_api.h"

// C API 状态需要可变的文件级变量
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
namespace {
std::string format_buffer;
int accumulator_value = 0;
}  // namespace
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// C 接口实现（snake_case 命名符合 C 惯例）
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
    format_buffer = "int(" + std::to_string(value) + ")";
    return format_buffer.c_str();
}

void c_api_accumulator_reset(void) {
    accumulator_value = 0;
}

void c_api_accumulator_add(int value) {
    accumulator_value += value;
}

int c_api_accumulator_get(void) {
    return accumulator_value;
}

}  // extern "C"
// NOLINTEND(readability-identifier-naming)
