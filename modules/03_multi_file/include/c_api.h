#pragma once

// extern "C" 语言链接演示。
// 本头文件可同时被 C 和 C++ 代码包含（通过 __cplusplus 保护宏）。
// 内部实现用 C++ 编写，但对外暴露 C ABI 接口。

#ifdef __cplusplus
extern "C" {
#endif

// C ABI 接口统一使用 snake_case，符合 C 命名惯例
// NOLINTBEGIN(readability-identifier-naming)

// 简单算术接口
int c_api_add(int a, int b);
int c_api_subtract(int a, int b);
int c_api_multiply(int a, int b);

// 字符串处理：返回静态缓冲区中的结果（简化演示）
const char* c_api_format_int(int value);

// 带状态的接口：累加器
void c_api_accumulator_reset(void);
void c_api_accumulator_add(int value);
int c_api_accumulator_get(void);

// NOLINTEND(readability-identifier-naming)

#ifdef __cplusplus
}  // extern "C"
#endif
