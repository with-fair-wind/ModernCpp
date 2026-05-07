// 存储复用（storage reuse）的可行 / 不可行情形。
//
// 文档中的几种情况：
//   情况 1：相同类型（忽略 cv）替换 + 完全相同存储 —— 旧名字/指针/引用仍有效。
//   情况 2：被复用对象需是平凡可析构，否则原对象不会被自动析构 —— 必须自己处理。
//   情况 3：const 完整对象（局部 const T 等）不能被复用 —— 编译器可基于 const
//          做激进优化，复用后行为未定义。
//   情况 4：unsigned char / std::byte 数组本身可"承载"其他对象 —— 数组不算结束
//          生命周期，外层类的生命周期不会被打断。
//
// 这里只展示 *合法* 情形并做行为验证；非法情形通过文字注释指出。

#include <cstddef>
#include <iostream>
#include <new>
#include <string>
#include <type_traits>

namespace {

struct Trivial {
    int x;
    int y;
};
static_assert(std::is_trivially_destructible_v<Trivial>);

struct NonTrivial {
    std::string name;
    explicit NonTrivial(std::string s) : name(std::move(s)) {
        std::cout << "    NonTrivial::ctor " << name << "\n";
    }
    NonTrivial(NonTrivial const&) = delete;
    NonTrivial(NonTrivial&&) = delete;
    NonTrivial& operator=(NonTrivial const&) = delete;
    NonTrivial& operator=(NonTrivial&&) = delete;
    ~NonTrivial() {
        std::cout << "    NonTrivial::dtor " << name << "\n";
    }
};
static_assert(!std::is_trivially_destructible_v<NonTrivial>);

}  // namespace

int main() {
    std::cout << "[情况 1] 平凡类型 同类型 替换 —— 旧名字仍可用\n";
    {
        Trivial t{.x = 1, .y = 2};
        Trivial* p = &t;

        // 在 t 的存储上构造同类型新对象
        ::new (&t) Trivial{.x = 10, .y = 20};

        // 标准允许继续用名字 t 与原指针 p 访问新对象（无需 launder）：
        //   - 类型相同（忽略 cv）
        //   - 完全相同存储
        //   - 没有 const 成员
        std::cout << "  t.x=" << t.x << " t.y=" << t.y << "\n";
        std::cout << "  p->x=" << p->x << " p->y=" << p->y << "\n";
    }

    std::cout << "\n[情况 2] 非平凡类型 替换 —— 必须先析构原对象\n";
    {
        alignas(NonTrivial) std::byte buf[sizeof(NonTrivial)];
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* a = ::new (buf) NonTrivial{"first"};
        // 不能直接覆盖：那样 first 持有的 string 堆内存会泄露。
        a->~NonTrivial();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* b = ::new (buf) NonTrivial{"second"};
        std::cout << "  read b: " << b->name << "\n";
        b->~NonTrivial();
    }

    std::cout << "\n[情况 3] 局部 const 对象 —— 不可复用其存储（仅文字说明）\n";
    {
        // const int kImmortal = 7;
        // ::new (const_cast<int*>(&kImmortal)) int{42};   // UB！
        // 编译器会假定 kImmortal 永远是 7，复用属于未定义。
        std::cout << "  see comments in source\n";
    }

    std::cout << "\n[情况 4] std::byte 数组承载对象 —— 数组本身不结束生命周期\n";
    {
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

        Box box;
        box.emplace("inside-byte-buffer");
        std::cout << "  box.obj->name = " << box.obj->name << "\n";
        // 此时 storage 数组承载着 NonTrivial；标准规定 storage 本身没结束生命周期，
        // 因此 box（外层类）也仍处于生命周期内 —— 与情况 4 的承载语义一致。
        box.clear();
    }

    return 0;
}
