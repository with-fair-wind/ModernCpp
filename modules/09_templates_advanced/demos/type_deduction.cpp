// 函数模板实参推导：按值剥顶层 cv/ref（decay）；左值引用与转发引用各具规则。

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace {

template <typename T>
[[nodiscard]] std::type_identity<std::decay_t<T>> tagPassByValue(T&& /*unused*/) {
    return {};
}

template <typename T>
[[nodiscard]] std::type_identity<T> tagLvalueReference([[maybe_unused]] T& lv) {
    return {};
}

template <typename T>
[[nodiscard]] std::type_identity<T> tagForwardingReference([[maybe_unused]] T&& fwd) {
    return {};
}

}  // namespace

int main() {
    std::string mutable_str = "mutable";
    std::string const const_str = "const";

    // 按值：无论传入左值是否具有 const，`T` 都是 `std::string`。
    auto by_val_mut = tagPassByValue(mutable_str);
    auto by_val_cst = tagPassByValue(const_str);
    static_assert(
        std::is_same_v<decltype(by_val_mut)::type, std::string>);
    static_assert(
        std::is_same_v<decltype(by_val_cst)::type, std::string>);

    // 左值引用：cv 会进入 `T`。
    auto lref_mut = tagLvalueReference(mutable_str);
    auto lref_cst = tagLvalueReference(const_str);
    static_assert(std::is_same_v<decltype(lref_mut)::type, std::string>);
    static_assert(std::is_same_v<decltype(lref_cst)::type, std::string const>);

    // 转发引用 + 可变左值：`T &&` + `std::string&` ⇒ `T = std::string&`。
    auto fwd_lv = tagForwardingReference(mutable_str);
    static_assert(std::is_same_v<decltype(fwd_lv)::type, std::string&>);
    std::cout << "转发引用遇到左值时 `T` 会带引用，从而保留 const 语义。\n";

    // 转发引用 + const 左值：`T = std::string const&`。
    auto fwd_lc = tagForwardingReference(const_str);
    static_assert(std::is_same_v<decltype(fwd_lc)::type, std::string const&>);

    // 转发引用 + 右值：`T = std::string`。
    auto fwd_rv = tagForwardingReference(std::move(mutable_str));
    static_assert(std::is_same_v<decltype(fwd_rv)::type, std::string>);
    mutable_str = "reset";

    std::cout << "按值：mutable 与 const 左值的 `T` 都是 decay 后的同一类型。\n";

    static_cast<void>(by_val_mut);
    static_cast<void>(by_val_cst);

    return 0;
}
