// 模板显式实例化：模板定义放在源文件中，
// 只为特定类型生成代码。用户只能使用已实例化的类型。

#include <cstddef>
#include <string>
#include <type_traits>

#include "tmpl.h"

// ========== 函数模板定义 ==========

template <typename T>
T tmplAdd(T const& a, T const& b) {
    return a + b;
}

template <typename T>
std::string tmplToString(T const& value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else {
        return std::to_string(value);
    }
}

// ========== 类模板成员定义 ==========

template <typename T>
void Stack<T>::push(T const& value) {
    if (top_ < kMaxSize) {
        data_[top_++] = value;
    }
}

template <typename T>
T Stack<T>::pop() {
    if (top_ > 0) {
        return data_[--top_];
    }
    return T{};
}

template <typename T>
bool Stack<T>::empty() const {
    return top_ == 0;
}

template <typename T>
std::size_t Stack<T>::size() const {
    return top_;
}

// ========== 显式实例化 ==========

template int tmplAdd<int>(int const&, int const&);
template double tmplAdd<double>(double const&, double const&);
template std::string tmplAdd<std::string>(std::string const&, std::string const&);

template std::string tmplToString<int>(int const&);
template std::string tmplToString<double>(double const&);
template std::string tmplToString<std::string>(std::string const&);

template class Stack<int>;
template class Stack<double>;
template class Stack<std::string>;
