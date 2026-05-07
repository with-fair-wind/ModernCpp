// static_cast / const_cast / reinterpret_cast 行为测试。
// dynamic_cast 单独放在 test_dynamic_cast.cpp。

#include <bit>
#include <cstdint>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace {

enum class Color : std::uint8_t { Red = 1, Green = 2, Blue = 3 };

struct ImplicitFromInt {
    int v;
    ImplicitFromInt(int x) : v(x) {}  // 单参构造可被 static_cast 触发
};

struct Base {
    int b{1};
    virtual ~Base() = default;
};
struct Derived : Base {
    int d{2};
};

}  // namespace

TEST(StaticCast, NumericNarrowingAndEnums) {
    int i = static_cast<int>(3.9);
    EXPECT_EQ(i, 3);  // 截断小数

    auto raw = static_cast<std::uint8_t>(Color::Blue);
    EXPECT_EQ(raw, 3);
    EXPECT_EQ(static_cast<Color>(raw), Color::Blue);
}

TEST(StaticCast, ConstructsViaSingleArgumentConstructor) {
    auto x = static_cast<ImplicitFromInt>(42);
    EXPECT_EQ(x.v, 42);
}

TEST(StaticCast, UpcastIsImplicitDowncastIsExplicit) {
    Derived d;
    Base* up = &d;  // 隐式上转
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    auto* down = static_cast<Derived*>(up);
    EXPECT_EQ(down->d, 2);
}

TEST(ConstCast, AddsAndStripsCv) {
    std::string s{"hello"};
    auto const& cs = std::as_const(s);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto& mut = const_cast<std::string&>(cs);
    mut[0] = 'H';
    EXPECT_EQ(s, "Hello");
}

TEST(ReinterpretCast, PointerToIntAndBack) {
    int x = 7;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto bits = reinterpret_cast<std::uintptr_t>(&x);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    int* same = reinterpret_cast<int*>(bits);
    EXPECT_EQ(same, &x);
    EXPECT_EQ(*same, 7);
}

TEST(BitCast, ReinterpretsBitsBetweenSameSizedTrivialTypes) {
    float f = 1.0F;
    auto bits = std::bit_cast<std::uint32_t>(f);
    EXPECT_EQ(bits, 0x3F800000U);
    auto round = std::bit_cast<float>(bits);
    EXPECT_EQ(round, 1.0F);
}

TEST(StaticCastVoid, DiscardsExpressionValue) {
    int unused = 42;
    static_cast<void>(unused);  // 等价于 (void)unused; —— 抑制 unused 警告
    EXPECT_EQ(unused, 42);      // 仅证明对象未被改动
}
