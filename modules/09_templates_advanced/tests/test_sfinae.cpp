// SFINAE：`std::enable_if` 选择、`std::void_t` 探测。

#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

namespace {

template <typename T>
// NOLINTNEXTLINE(modernize-use-constraints) — 本节演示 SFINAE 与 enable_if
auto pickLabel([[maybe_unused]] T value) -> std::enable_if_t<std::is_integral_v<T>, char> {
    return 'I';
}

template <typename T>
// NOLINTNEXTLINE(modernize-use-constraints) — 本节演示 SFINAE 与 enable_if
auto pickLabel([[maybe_unused]] T value) -> std::enable_if_t<std::is_floating_point_v<T>, char> {
    return 'F';
}

template <typename, typename = void>
struct HasValueTypeTrait : std::false_type {};

template <typename T>
struct HasValueTypeTrait<T, std::void_t<typename T::value_type>> : std::true_type {};

}  // namespace

TEST(Sfinae, EnableIfSelectsIntegralVersusFloating) {
    EXPECT_EQ(pickLabel(4), 'I');
    EXPECT_EQ(pickLabel(4.0F), 'F');
}

TEST(Sfinae, VoidTDetectsContainers) {
    static_assert(HasValueTypeTrait<std::vector<int>>::value);

    struct Raw {
        double x{};
    };
    static_assert(!HasValueTypeTrait<Raw>::value);

    EXPECT_TRUE(HasValueTypeTrait<std::vector<int>>::value);
    EXPECT_FALSE(HasValueTypeTrait<Raw>::value);
}
