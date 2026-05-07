// std::variant：基本 API、std::visit、monostate、emplace。

#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

namespace {

template <class... Fs>
struct Overloaded : Fs... {
    using Fs::operator()...;
};
template <class... Fs>
Overloaded(Fs...) -> Overloaded<Fs...>;

}  // namespace

TEST(Variant, DefaultsToFirstAlternative) {
    std::variant<int, std::string> v;
    EXPECT_EQ(v.index(), 0U);
    EXPECT_TRUE(std::holds_alternative<int>(v));
    EXPECT_EQ(std::get<int>(v), 0);
}

TEST(Variant, MonostateAsExplicitEmpty) {
    std::variant<std::monostate, int, std::string> v;
    EXPECT_EQ(v.index(), 0U);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(Variant, AssignSwitchesActiveAlternative) {
    std::variant<int, std::string> v = 1;
    EXPECT_TRUE(std::holds_alternative<int>(v));
    v = std::string{"two"};
    EXPECT_TRUE(std::holds_alternative<std::string>(v));
    EXPECT_EQ(std::get<std::string>(v), "two");
}

TEST(Variant, GetWrongAlternativeThrows) {
    std::variant<int, double> v = 3.14;
    EXPECT_THROW({ static_cast<void>(std::get<int>(v)); }, std::bad_variant_access);
    EXPECT_EQ(std::get_if<int>(&v), nullptr);
    ASSERT_NE(std::get_if<double>(&v), nullptr);
    EXPECT_DOUBLE_EQ(*std::get_if<double>(&v), 3.14);
}

TEST(Variant, VisitDispatchesOnActiveAlternative) {
    std::vector<std::variant<int, double, std::string>> all{
        1, 2.5, std::string{"x"},
    };
    std::vector<std::string> tags;
    for (auto const& v : all) {
        std::visit(
            Overloaded{
                [&](int) { tags.emplace_back("int"); },
                [&](double) { tags.emplace_back("double"); },
                [&](std::string const&) { tags.emplace_back("string"); },
            },
            v);
    }
    ASSERT_EQ(tags.size(), 3U);
    EXPECT_EQ(tags[0], "int");
    EXPECT_EQ(tags[1], "double");
    EXPECT_EQ(tags[2], "string");
}

TEST(Variant, EmplaceConstructsInPlace) {
    std::variant<std::monostate, std::string> v;
    v.emplace<std::string>(5, '*');  // "*****"
    EXPECT_EQ(std::get<std::string>(v), "*****");
}

TEST(Variant, InPlaceTypeForMoveOnly) {
    struct MoveOnly {
        int x;
        explicit MoveOnly(int v) : x(v) {}
        MoveOnly(MoveOnly const&) = delete;
        MoveOnly(MoveOnly&&) = default;
        MoveOnly& operator=(MoveOnly const&) = delete;
        MoveOnly& operator=(MoveOnly&&) = default;
        ~MoveOnly() = default;
    };
    std::variant<std::monostate, MoveOnly> v{std::in_place_type<MoveOnly>, 7};
    EXPECT_EQ(std::get<MoveOnly>(v).x, 7);
}

TEST(Variant, IndexedAccessForRepeatedTypes) {
    std::variant<int, int, std::string> v{std::in_place_index<1>, 42};
    EXPECT_EQ(v.index(), 1U);
    EXPECT_EQ(std::get<1>(v), 42);
    ASSERT_NE(std::get_if<1>(&v), nullptr);
    EXPECT_EQ(std::get_if<0>(&v), nullptr);
}
