#ifndef MCPP_03_MULTI_FILE_TMPL_H
#define MCPP_03_MULTI_FILE_TMPL_H

// 模板显式实例化演示。
// 模板声明放在头文件，定义放在源文件 tmpl_impl.cpp。
// 源文件中通过显式实例化指令生成特定类型的代码。
// 用户只能使用已被显式实例化的类型，否则链接失败。

#include <string>

// 函数模板 —— 仅声明，不含定义
template <typename T>
T tmplAdd(T const& a, T const& b);

template <typename T>
std::string tmplToString(T const& value);

// 类模板 —— 仅声明成员函数，不含定义
template <typename T>
class Stack {
public:
    void push(T const& value);
    T pop();
    [[nodiscard]] bool empty() const;
    [[nodiscard]] std::size_t size() const;

private:
    static constexpr std::size_t kMaxSize = 64;
    T data_[kMaxSize]{};
    std::size_t top_{0};
};

#endif  // MCPP_03_MULTI_FILE_TMPL_H
