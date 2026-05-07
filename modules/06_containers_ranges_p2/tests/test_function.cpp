// std::function / std::move_only_function / std::reference_wrapper 行为。

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <version>

#include <gtest/gtest.h>

namespace {

int add(int a, int b) {
    return a + b;
}

struct Functor {
    int operator()(int a, int b) const {
        return a * b;
    }
};

}  // namespace

TEST(StdFunction, AcceptsMultipleCallableKinds) {
    std::function<int(int, int)> f = add;
    EXPECT_EQ(f(2, 3), 5);

    f = Functor{};
    EXPECT_EQ(f(2, 3), 6);

    int captured = 100;
    f = [captured](int a, int b) { return captured + a + b; };
    EXPECT_EQ(f(2, 3), 105);
}

TEST(StdFunction, EmptyCallThrows) {
    std::function<int()> empty;
    EXPECT_FALSE(static_cast<bool>(empty));
    EXPECT_TRUE(empty == nullptr);
    EXPECT_THROW(static_cast<void>(empty()), std::bad_function_call);
}

TEST(StdFunction, MemberFunctionPointer) {
    std::function<std::size_t(const std::string&)> sz = &std::string::size;
    EXPECT_EQ(sz("hello"), 5U);
}

#if defined(__cpp_lib_move_only_function) && __cpp_lib_move_only_function >= 202110L
TEST(MoveOnlyFunction, CanCaptureUniquePtr) {
    auto p = std::make_unique<int>(42);
    std::move_only_function<int()> mof = [up = std::move(p)] { return *up; };
    EXPECT_EQ(mof(), 42);

    // 移动 mof 给另一个：原来那个不可再调用
    std::move_only_function<int()> moved = std::move(mof);
    EXPECT_EQ(moved(), 42);

    // 不可拷贝（编译期检查）
    static_assert(!std::is_copy_constructible_v<std::move_only_function<int()>>);
}
#endif

TEST(ReferenceWrapper, BehavesLikeRebindablePointer) {
    int a = 1;
    int b = 2;
    std::reference_wrapper<int> r = a;
    r.get() = 10;
    EXPECT_EQ(a, 10);

    r = b;  // 重新绑定到 b
    r.get() = 20;
    EXPECT_EQ(b, 20);
    EXPECT_EQ(a, 10);  // a 不再被改
}

TEST(ReferenceWrapper, CanLiveInContainer) {
    int x = 1;
    int y = 2;
    int z = 3;
    std::vector<std::reference_wrapper<int>> refs{x, y, z};
    for (auto& r : refs) {
        r.get() *= 10;
    }
    EXPECT_EQ(x, 10);
    EXPECT_EQ(y, 20);
    EXPECT_EQ(z, 30);
}

TEST(StdRef, BindsViaFunctionWithoutCopy) {
    int counter = 0;
    auto incr = [&counter] { ++counter; };

    // 直接传 lambda：function 持有 lambda 的拷贝（捕获是引用，所以仍能改 counter）
    std::function<void()> by_value = incr;
    by_value();
    by_value();
    EXPECT_EQ(counter, 2);

    // std::ref：function 内部持有 reference_wrapper，调用打到原 lambda 上
    counter = 0;
    std::function<void()> by_ref = std::ref(incr);
    by_ref();
    EXPECT_EQ(counter, 1);
}

TEST(StdInvoke, UnifiesCallSyntax) {
    EXPECT_EQ(std::invoke(&add, 2, 3), 5);                       // 普通函数指针
    EXPECT_EQ(std::invoke([](int x) { return x * x; }, 5), 25);  // lambda
    EXPECT_EQ(std::invoke(Functor{}, 2, 3), 6);                  // functor

    std::string s{"hello"};
    EXPECT_EQ(std::invoke(&std::string::size, s), 5U);   // 成员函数指针 + 对象
    EXPECT_EQ(std::invoke(&std::string::size, &s), 5U);  // 成员函数指针 + 指针
}
