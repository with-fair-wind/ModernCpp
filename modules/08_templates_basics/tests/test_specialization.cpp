// 验证主模板、偏特化、全特化之间的选择优先级与标签文本。

#include <type_traits>

#include <gtest/gtest.h>

namespace {

template <typename T>
struct TypeTag {
    static constexpr char const* id() {
        return "primary";
    }
};

template <typename T>
struct TypeTag<T*> {
    static constexpr char const* id() {
        return "partial-pointer";
    }
};

template <>
struct TypeTag<bool> {
    static constexpr char const* id() {
        return "full-bool";
    }
};

}  // namespace

TEST(Specialization, PrimaryChosenForOrdinaryTypes) {
    static_assert(std::is_same_v<decltype(&TypeTag<int>::id), char const* (*)()>);
    EXPECT_STREQ(TypeTag<int>::id(), "primary");
    EXPECT_STREQ(TypeTag<double>::id(), "primary");
}

TEST(Specialization, PartialChosenForPointers) {
    EXPECT_STREQ(TypeTag<int*>::id(), "partial-pointer");
    EXPECT_STREQ(TypeTag<void*>::id(), "partial-pointer");
}

TEST(Specialization, FullSpecializationOverridesPrimaryAndPartial) {
    // bool 既匹配主模板也匹配指针偏特化，但 bool 全特化最优先。
    EXPECT_STREQ(TypeTag<bool>::id(), "full-bool");
    // bool* 仍为指针偏特化，不会被 bool 的全特化“抢走”。
    EXPECT_STREQ(TypeTag<bool*>::id(), "partial-pointer");
}
