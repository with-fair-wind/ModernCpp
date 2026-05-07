// placement new 与 std::launder 的行为测试。

#include <cstddef>
#include <new>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace {

struct Tracker {
    static int alive;
    std::string label;

    explicit Tracker(std::string s) : label(std::move(s)) {
        ++alive;
    }
    Tracker(Tracker const&) = delete;
    Tracker(Tracker&&) = delete;
    Tracker& operator=(Tracker const&) = delete;
    Tracker& operator=(Tracker&&) = delete;
    ~Tracker() {
        --alive;
    }
};
int Tracker::alive = 0;

struct WithConstField {
    int const id;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    int value;
};

}  // namespace

TEST(PlacementNew, ConstructsAndDestroysInExternalBuffer) {
    Tracker::alive = 0;
    alignas(Tracker) std::byte buf[sizeof(Tracker)];
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        Tracker* p = ::new (buf) Tracker{"hello"};
        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_EQ(p->label, "hello");
        p->~Tracker();
    }
    EXPECT_EQ(Tracker::alive, 0);
}

TEST(PlacementNew, ReuseBufferForSameType) {
    Tracker::alive = 0;
    alignas(Tracker) std::byte buf[sizeof(Tracker)];
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    Tracker* p1 = ::new (buf) Tracker{"first"};
    EXPECT_EQ(p1->label, "first");
    p1->~Tracker();

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    Tracker* p2 = ::new (buf) Tracker{"second"};
    EXPECT_EQ(p2->label, "second");
    EXPECT_EQ(Tracker::alive, 1);
    p2->~Tracker();
    EXPECT_EQ(Tracker::alive, 0);
}

TEST(PlacementNew, ForgettingExplicitDtorWouldLeak) {
    // 这条测试只在 *逻辑上* 提醒：忘记 obj.~T() 会把 string 持有的堆缓冲泄漏。
    // 这里用计数器证实：若我们不调析构，alive 不会减。
    Tracker::alive = 0;
    {
        alignas(Tracker) std::byte buf[sizeof(Tracker)];
        // NOLINTNEXTLINE(bugprone-unused-return-value)
        ::new (buf) Tracker{"leak-on-purpose"};
        EXPECT_EQ(Tracker::alive, 1);
        // 故意不调用 ~Tracker —— 注意：此举仅作教学；实际产线代码必须显式析构。
        // 我们手动补回，避免 ASan / 静态计数残留。
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        std::launder(reinterpret_cast<Tracker*>(buf))->~Tracker();
    }
    EXPECT_EQ(Tracker::alive, 0);
}

TEST(PlacementNew, LaunderRequiredAfterReuseWithConstMember) {
    alignas(WithConstField) std::byte buf[sizeof(WithConstField)];
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* p1 = ::new (buf) WithConstField{.id = 1, .value = 100};
    EXPECT_EQ(p1->id, 1);
    EXPECT_EQ(p1->value, 100);
    p1->~WithConstField();

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    ::new (buf) WithConstField{.id = 2, .value = 200};
    // 旧指针 p1 在含 const 成员时不可再用；必须 std::launder 出新的指针
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* p2 = std::launder(reinterpret_cast<WithConstField*>(buf));
    EXPECT_EQ(p2->id, 2);
    EXPECT_EQ(p2->value, 200);
    p2->~WithConstField();
}
