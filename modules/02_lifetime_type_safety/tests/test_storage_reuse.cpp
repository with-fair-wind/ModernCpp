// 存储复用：平凡类型 vs 非平凡类型；byte 数组承载对象。

#include <cstddef>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

namespace {

struct Trivial {
    int x;
    int y;
};
static_assert(std::is_trivially_destructible_v<Trivial>);
static_assert(std::is_trivially_copyable_v<Trivial>);

struct NonTrivial {
    static int alive;
    std::string s;
    explicit NonTrivial(std::string v) : s(std::move(v)) {
        ++alive;
    }
    NonTrivial(NonTrivial const&) = delete;
    NonTrivial(NonTrivial&&) = delete;
    NonTrivial& operator=(NonTrivial const&) = delete;
    NonTrivial& operator=(NonTrivial&&) = delete;
    ~NonTrivial() {
        --alive;
    }
};
int NonTrivial::alive = 0;

}  // namespace

TEST(StorageReuse, TrivialTypeNameIsValidAfterPlacementNewSameType) {
    Trivial t{.x = 1, .y = 2};
    Trivial* p = &t;

    // 在 t 的存储上重新构造同类型对象：t 与 p 仍然合法（无须 launder）
    ::new (&t) Trivial{.x = 10, .y = 20};
    EXPECT_EQ(t.x, 10);
    EXPECT_EQ(t.y, 20);
    EXPECT_EQ(p->x, 10);
    EXPECT_EQ(p->y, 20);
}

TEST(StorageReuse, NonTrivialNeedsManualDestructionBeforeReuse) {
    NonTrivial::alive = 0;
    alignas(NonTrivial) std::byte buf[sizeof(NonTrivial)];

    auto* a = ::new (buf) NonTrivial{"first"};
    EXPECT_EQ(NonTrivial::alive, 1);
    a->~NonTrivial();

    auto* b = ::new (buf) NonTrivial{"second"};
    EXPECT_EQ(NonTrivial::alive, 1);  // 同时只有 1 个活着 —— 我们手工管理
    EXPECT_EQ(b->s, "second");

    b->~NonTrivial();
    EXPECT_EQ(NonTrivial::alive, 0);
}

TEST(StorageReuse, ByteArrayCarriesObjectWithoutEndingItsLifetime) {
    NonTrivial::alive = 0;
    struct Box {
        alignas(NonTrivial) std::byte storage[sizeof(NonTrivial)]{};
        NonTrivial* obj{nullptr};

        Box() = default;
        Box(Box const&) = delete;
        Box(Box&&) = delete;
        Box& operator=(Box const&) = delete;
        Box& operator=(Box&&) = delete;

        void emplace(std::string s) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            obj = ::new (storage) NonTrivial{std::move(s)};
        }
        void clear() {
            if (obj != nullptr) {
                obj->~NonTrivial();
                obj = nullptr;
            }
        }
        ~Box() {
            clear();
        }
    };

    {
        Box box;
        box.emplace("inside");
        EXPECT_EQ(NonTrivial::alive, 1);
        EXPECT_EQ(box.obj->s, "inside");
        box.clear();
        EXPECT_EQ(NonTrivial::alive, 0);
        // 即便我们刚刚 clear，box 自身仍然合法 —— byte 数组不结束生命周期
        box.emplace("re-emplaced");
        EXPECT_EQ(box.obj->s, "re-emplaced");
    }
    EXPECT_EQ(NonTrivial::alive, 0);
}
