// 函数模板推导：按值 decay、左值引用保留顶层 const、转发引用推导规则。

#include <string>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

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

TEST(TypeDeduction, PassByValueStripsCvRefViaDecay) {
    std::string mutable_str = "mutable";
    std::string const const_str = "const";

    auto by_val_mut = tagPassByValue(mutable_str);
    auto by_val_cst = tagPassByValue(const_str);

    static_assert(std::is_same_v<decltype(by_val_mut)::type, std::string>);
    static_assert(std::is_same_v<decltype(by_val_cst)::type, std::string>);
}

TEST(TypeDeduction, LvalueReferencePreservesCvInT) {
    std::string mutable_str = "mutable";
    std::string const const_str = "const";

    auto lref_mut = tagLvalueReference(mutable_str);
    auto lref_cst = tagLvalueReference(const_str);

    static_assert(std::is_same_v<decltype(lref_mut)::type, std::string>);
    static_assert(std::is_same_v<decltype(lref_cst)::type, std::string const>);
}

TEST(TypeDeduction, ForwardingReferenceCollapsesForLvaluesAndRvales) {
    std::string mutable_str = "mutable";
    std::string const const_str = "const";

    auto fwd_lv = tagForwardingReference(mutable_str);
    static_assert(std::is_same_v<decltype(fwd_lv)::type, std::string&>);

    auto fwd_lc = tagForwardingReference(const_str);
    static_assert(std::is_same_v<decltype(fwd_lc)::type, std::string const&>);

    auto fwd_rv = tagForwardingReference(std::move(mutable_str));
    static_assert(std::is_same_v<decltype(fwd_rv)::type, std::string>);
    mutable_str = "reset";

    static_cast<void>(fwd_lv);
    static_cast<void>(fwd_lc);
    static_cast<void>(fwd_rv);
}
