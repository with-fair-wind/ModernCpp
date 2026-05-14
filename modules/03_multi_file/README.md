# 03_multi_file — 多文件编程 / Multi-file Programming

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

| Demo | 主题 |
|------|------|
| `preprocessor_demo` | 宏、条件编译、`__VA_OPT__`、`#undef`、`#pragma once`、feature test macro |
| `odr_basics` | ODR、声明/定义分离、头文件保护、默认参数 |
| `forward_decl` | 前置声明、打破循环包含 |
| `namespaces` | 命名空间、嵌套(C++17)、inline namespace |
| `inline_var` | `inline` 函数与 C++17 `inline` 变量 |
| `linkage_demo` | 外部/内部链接、`static`、匿名命名空间 |
| `singleton_demo` | Meyers' Singleton |
| `extern_c_demo` | `extern "C"` 语言链接 |
| `explicit_instantiation` | 模板显式实例化 |

## 测试 / Tests

| Test | 覆盖点 |
|------|--------|
| `test_preprocessor` | 宏展开、字符串化、拼接、`__VA_OPT__`、`#undef`、feature test |
| `test_odr` | 跨 TU 函数调用、类成员链接、静态成员共享、默认参数 |
| `test_namespaces` | 嵌套命名空间、inline namespace 版本控制 |
| `test_inline_var` | inline 变量跨 TU 地址一致、状态共享 |
| `test_linkage` | 外部/内部链接、`extern const` |
| `test_singleton` | 单例跨 TU 唯一性、状态共享 |
| `test_extern_c` | C ABI 函数正确链接 |
| `test_forward_decl` | 前置声明、互指指针、不完整类型 |
| `test_explicit_instantiation` | 函数模板/类模板显式实例化 |
