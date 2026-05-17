// 万能引用：`T&&` 在模板推导中保留左右值语义；std::forward 将值类别传给下一层。

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

namespace {

template <typename T>
constexpr bool forwardedAsLvalue(T&& arg) noexcept {
    auto&& forwarded = std::forward<T>(arg);
    return std::is_lvalue_reference_v<decltype(forwarded)>;
}

template <typename T>
decltype(auto) relayIdentity(T&& value) noexcept {
    return std::forward<T>(value);  // 保持值类别转发。
}

enum class PayloadKind : std::uint8_t {
    Empty = 0,
    Copy = 1,
    Move = 2,
};

PayloadKind observe(std::string const& /*unused*/) noexcept {
    return PayloadKind::Copy;
}

PayloadKind observe(std::string&& borrowed) noexcept {
    (void)std::move(borrowed);
    return PayloadKind::Move;
}

template <typename Payload>
decltype(auto) forwardToObserve(Payload&& payload) noexcept {
    return observe(std::forward<Payload>(payload));
}

}  // namespace

TEST(UniversalReference, DeducesLRefForVariables) {
    int x = 42;
    EXPECT_TRUE(forwardedAsLvalue(x));
    EXPECT_FALSE(forwardedAsLvalue(42));
}

TEST(UniversalReference, RelayPreservesValueCategoryForScalar) {
    int x = 7;
    int& l_ref = relayIdentity(x);
    EXPECT_EQ(l_ref, 7);

    int&& tmp = relayIdentity(static_cast<int&&>(x));
    EXPECT_EQ(tmp, 7);
}

TEST(UniversalReferenceForwarding, MovesRvaluesAndCopiesLvalues) {
    std::string buffer{"modern"};
    std::string movable = buffer;
    PayloadKind movable_kind = forwardToObserve(std::move(movable));
    PayloadKind copying_kind = forwardToObserve(buffer);

    EXPECT_EQ(movable_kind, PayloadKind::Move);
    EXPECT_EQ(copying_kind, PayloadKind::Copy);
}
