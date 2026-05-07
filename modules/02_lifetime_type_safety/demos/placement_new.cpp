// placement new 与 std::launder。
//
// 关键点：
//   - placement new 不分配内存，只在指定地址 "构造对象"；析构必须手工调用。
//   - 缓冲区必须满足类型对齐 —— 用 alignas(T) 或 std::aligned_storage_for_t。
//   - 在已构造对象的存储上再 placement new 一个 *相同类型* 的新对象后，原指针
//     在简单情形下仍可用（pointer-interconvertible），但若新旧对象类型不同，
//     或外层类型含有 const / 引用成员，必须用 std::launder 才能合法访问。
//   - placement new 后必须显式 obj.~T()，否则会泄露资源（如 std::string 的堆缓冲）。

#include <cstddef>
#include <iostream>
#include <new>
#include <string>
#include <utility>

namespace {

struct Greeter {
    std::string greeting;

    explicit Greeter(std::string s) : greeting(std::move(s)) {
        std::cout << "  Greeter::ctor  '" << greeting << "'\n";
    }
    ~Greeter() {
        std::cout << "  Greeter::dtor  '" << greeting << "'\n";
    }
    Greeter(Greeter const&) = delete;
    Greeter(Greeter&&) = delete;
    Greeter& operator=(Greeter const&) = delete;
    Greeter& operator=(Greeter&&) = delete;
};

// 含 const 成员的类型：替换存储后必须用 std::launder 才能读。
// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct WithConst {
    int const id;
    int value;
};

// 一个固定容量的小池子；放在命名空间作用域，方便模板成员函数。
struct Pool {
    static constexpr std::size_t kSlots = 2;
    alignas(Greeter) std::byte storage[sizeof(Greeter) * kSlots]{};
    bool occupied[kSlots]{false, false};

    template <class... Args>
    Greeter* construct(std::size_t slot, Args&&... args) {
        auto* mem = storage + (sizeof(Greeter) * slot);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* p = ::new (mem) Greeter{std::forward<Args>(args)...};
        occupied[slot] = true;
        return p;
    }

    void destroy(std::size_t slot) {
        if (!occupied[slot]) {
            return;
        }
        auto* mem = storage + (sizeof(Greeter) * slot);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-owning-memory)
        std::launder(reinterpret_cast<Greeter*>(mem))->~Greeter();
        occupied[slot] = false;
    }

    Pool() = default;
    Pool(Pool const&) = delete;
    Pool(Pool&&) = delete;
    Pool& operator=(Pool const&) = delete;
    Pool& operator=(Pool&&) = delete;
    ~Pool() {
        destroy(0);
        destroy(1);
    }
};

}  // namespace

int main() {
    std::cout << "[1] 在栈上的对齐缓冲区构造 / 析构\n";
    {
        alignas(Greeter) std::byte buf[sizeof(Greeter)];
        Greeter* p = ::new (buf) Greeter{"hello"};  // NOLINT(cppcoreguidelines-owning-memory)
        std::cout << "  via p: " << p->greeting << "\n";
        p->~Greeter();
    }

    std::cout << "\n[2] 同一缓冲上构造、析构、再构造（同类型，无 const 成员）\n";
    {
        alignas(Greeter) std::byte buf[sizeof(Greeter)];
        Greeter* p1 = ::new (buf) Greeter{"first"};  // NOLINT(cppcoreguidelines-owning-memory)
        std::cout << "  p1->greeting = " << p1->greeting << "\n";
        p1->~Greeter();

        Greeter* p2 = ::new (buf) Greeter{"second"};  // NOLINT(cppcoreguidelines-owning-memory)
        std::cout << "  p2->greeting = " << p2->greeting << "\n";
        // 注意：原 p1 在新对象生命周期开始后已是悬挂指针；不可再使用。
        p2->~Greeter();
    }

    std::cout << "\n[3] std::launder：含 const 成员时刷新指针\n";
    {
        alignas(WithConst) std::byte buf[sizeof(WithConst)];
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* p1 = ::new (buf) WithConst{1, 10};
        std::cout << "  p1: id=" << p1->id << " value=" << p1->value << "\n";
        p1->~WithConst();

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        ::new (buf) WithConst{2, 20};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        WithConst* p2 = std::launder(reinterpret_cast<WithConst*>(buf));
        std::cout << "  p2 (laundered): id=" << p2->id << " value=" << p2->value << "\n";
        p2->~WithConst();
    }

    std::cout << "\n[4] placement new 用于自管理的小型对象池\n";
    {
        Pool pool;
        auto* a = pool.construct(0, std::string{"slot0"});
        auto* b = pool.construct(1, std::string{"slot1"});
        std::cout << "  pool[0]=" << a->greeting << " pool[1]=" << b->greeting << "\n";
        // Pool 析构时统一释放
    }

    return 0;
}
