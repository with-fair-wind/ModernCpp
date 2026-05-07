// 成员函数指针 / std::invoke / std::bind_front。

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

namespace {

struct Counter {
    int value{};

    void add(int n) {
        value += n;
    }
    [[nodiscard]] int doubled() const {
        return value * 2;
    }
};

}  // namespace

TEST(MemberFnPtr, ObjectAndPointerSyntax) {
    using AddPtr = void (Counter::*)(int);
    AddPtr add = &Counter::add;

    Counter c{};
    (c.*add)(5);
    EXPECT_EQ(c.value, 5);

    Counter* p = &c;
    (p->*add)(3);
    EXPECT_EQ(c.value, 8);
}

TEST(StdInvoke, MemberFunctionViaObjectAndPointer) {
    Counter c{};
    c.value = 10;
    EXPECT_EQ(std::invoke(&Counter::doubled, c), 20);
    EXPECT_EQ(std::invoke(&Counter::doubled, &c), 20);
}

TEST(StdInvoke, DataMemberPointer) {
    Counter c{};
    c.value = 7;
    EXPECT_EQ(std::invoke(&Counter::value, c), 7);
    std::invoke(&Counter::value, c) = 99;  // 通过引用写回
    EXPECT_EQ(c.value, 99);
}

TEST(StdInvokeR, ConvertsResultType) {
    auto len = std::invoke_r<std::int64_t>(&std::string::size, std::string{"hello"});
    static_assert(std::is_same_v<decltype(len), std::int64_t>);
    EXPECT_EQ(len, 5);
}

TEST(BindFront, FixesLeadingParameters) {
    auto add3 = [](int a, int b, int c) { return a + b + c; };
    auto add_with10 = std::bind_front(add3, 10);
    EXPECT_EQ(add_with10(20, 30), 60);

    auto add_with10_20 = std::bind_front(add3, 10, 20);
    EXPECT_EQ(add_with10_20(30), 60);
}

TEST(BindFront, BindsMemberFunction) {
    Counter c{};
    auto add_to_c = std::bind_front(&Counter::add, std::ref(c));
    add_to_c(7);
    add_to_c(3);
    EXPECT_EQ(c.value, 10);
}

TEST(BindBack, FixesTrailingParameters) {
    auto sub = [](int a, int b) { return a - b; };
    auto sub_5 = std::bind_back(sub, 5);
    EXPECT_EQ(sub_5(10), 5);  // 10 - 5
}
