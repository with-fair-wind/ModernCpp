// 四种 C++ 命名转换 + C 风格转换的对照。
//
// 关键点：
//   - static_cast：编译期"显式但合理"的转换 —— 数值缩窄、枚举互转、上下转、
//     void* 与具体指针互转、构造新对象、转 void 丢值等等。
//   - dynamic_cast：仅用于多态类型的安全继承转换；失败则指针返回 nullptr，
//     引用抛 std::bad_cast。详见 dynamic_cast_rtti.cpp。
//   - const_cast：去掉 / 加上 const / volatile —— 不会改变对象本身，只是改变
//     指针 / 引用的访问权。对原本不可写的对象写入是 UB。
//   - reinterpret_cast：低层位级转换；几乎只在指针互转 / 指针 ↔ 整数时使用。
//     真正的"按位重读"应该用 std::bit_cast 或 std::memcpy。
//   - C 风格 (T)x：以上几种依次尝试，并能忽略访问控制 —— 失去了显式提示的
//     好处，建议永远不用。

#include <bit>
#include <cstdint>
#include <iostream>
#include <string>

namespace {

enum class Color : std::uint8_t { Red = 1, Green = 2, Blue = 3 };

struct Counted {
    int value;
    explicit Counted(int v) : value(v) {}  // 单参构造可被 static_cast<Counted>(int) 选中
};

struct Base {
    int b{1};
    virtual ~Base() = default;
};
struct Derived : Base {
    int d{2};
};

}  // namespace

int main() {
    std::cout << "[1] static_cast：数值 / 枚举 / 构造对象 / 上下转\n";
    {
        // 数值缩窄（显式表达"我知道在缩窄"）
        double dpi = 3.14;
        int truncated = static_cast<int>(dpi);
        std::cout << "  double->int trunc = " << truncated << '\n';

        // 枚举类与底层整数互转
        Color c = Color::Green;
        auto raw = static_cast<std::uint8_t>(c);
        Color back = static_cast<Color>(raw);
        std::cout << "  Color(2) <-> raw=" << +raw << " back="
                  << static_cast<int>(back) << '\n';

        // 用单参构造函数构造新对象
        auto counted = static_cast<Counted>(42);
        std::cout << "  Counted{42}.value = " << counted.value << '\n';

        // 多态指针上转 / 下转
        Derived d;
        Base* up = static_cast<Base*>(&d);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        Derived* down = static_cast<Derived*>(up);  // 已知底层就是 Derived，所以安全
        std::cout << "  up->b=" << up->b << "  down->d=" << down->d << '\n';
    }

    std::cout << "\n[2] const_cast：去掉 const 进入非 const 接口（必须保证原对象可写）\n";
    {
        std::string s{"hello"};
        // 第三方 API 设计失误：参数应当是 string& 但写成了 string const& —— 我们手上
        // 的 s 实际上可写，所以 const_cast 安全。
        auto fix_in_place = [](std::string const& cs) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            auto& mut = const_cast<std::string&>(cs);
            mut[0] = 'H';
        };
        fix_in_place(s);
        std::cout << "  after const_cast write: " << s << '\n';

        // 反例（不写代码，仅文字说明）：对真正只读对象用 const_cast 写入是 UB。
        //   const int kImmortal = 1;
        //   const_cast<int&>(kImmortal) = 2;   // UB
    }

    std::cout << "\n[3] reinterpret_cast：指针 ↔ 整数 / 不同对象指针互转（受严格别名约束）\n";
    {
        unsigned int x = 0xDEADBEEF;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto p = reinterpret_cast<std::uintptr_t>(&x);
        std::cout << "  &x as uintptr_t = 0x" << std::hex << p << std::dec << '\n';
        // 转回去得到原指针 —— 与 reinterpret_cast<unsigned int*>(p) 等价
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
        unsigned int* q = reinterpret_cast<unsigned int*>(p);
        std::cout << "  *q = 0x" << std::hex << *q << std::dec << '\n';
    }

    std::cout << "\n[4] 想按位重读应该用 std::bit_cast，而非 reinterpret_cast\n";
    {
        float f = 1.0F;
        auto bits = std::bit_cast<std::uint32_t>(f);  // 1065353216 = 0x3F800000
        std::cout << "  bit_cast<uint32_t>(1.0f) = 0x" << std::hex << bits << std::dec << '\n';
    }

    std::cout << "\n[5] static_cast<void>(expr) —— 显式丢弃返回值 / 抑制 unused 警告\n";
    {
        int unused = 42;
        static_cast<void>(unused);  // 与 (void)unused; 等价
        std::cout << "  ok\n";
    }

    return 0;
}
