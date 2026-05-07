// std::any：装载、any_cast、in_place、reset、swap。

#include <any>
#include <string>
#include <utility>

#include <gtest/gtest.h>

TEST(Any, DefaultIsEmpty) {
    std::any a;
    EXPECT_FALSE(a.has_value());
}

TEST(Any, HoldsCopyableValueAndAnyCastReadsIt) {
    std::any a = 42;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(std::any_cast<int>(a), 42);

    // 引用版 any_cast<T&> 直接修改 *存储* —— 不是拷贝
    std::any_cast<int&>(a) = 100;
    EXPECT_EQ(std::any_cast<int>(a), 100);
}

TEST(Any, WrongValueCastThrowsBadAnyCast) {
    std::any a = 1LL;  // long long
    EXPECT_THROW({ static_cast<void>(std::any_cast<int>(a)); }, std::bad_any_cast);
    EXPECT_EQ(std::any_cast<long long>(a), 1LL);
}

TEST(Any, PointerCastReturnsNullOnWrongType) {
    std::any a = std::string{"hi"};
    EXPECT_EQ(std::any_cast<int>(&a), nullptr);
    auto const* p = std::any_cast<std::string>(&a);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(*p, "hi");
}

TEST(Any, ResetMakesItEmpty) {
    std::any a = std::string{"value"};
    EXPECT_TRUE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
    EXPECT_EQ(std::any_cast<std::string>(&a), nullptr);
}

TEST(Any, InPlaceTypeAndEmplace) {
    std::any a{std::in_place_type<std::string>, 5, 'a'};
    EXPECT_EQ(std::any_cast<std::string>(a), "aaaaa");

    a.emplace<std::string>("emplaced");
    EXPECT_EQ(std::any_cast<std::string>(a), "emplaced");
}

TEST(Any, SwapExchangesContents) {
    std::any a = 1;
    std::any b = std::string{"hello"};
    std::swap(a, b);
    EXPECT_EQ(std::any_cast<std::string>(a), "hello");
    EXPECT_EQ(std::any_cast<int>(b), 1);
}

TEST(Any, TypeReturnsTypeInfoOfStoredObject) {
    std::any a = 3.14;
    EXPECT_EQ(a.type(), typeid(double));
    a = std::string{"x"};
    EXPECT_EQ(a.type(), typeid(std::string));
    a.reset();
    EXPECT_EQ(a.type(), typeid(void));
}
