// 临时对象的生命周期延长（lifetime extension）。
//
// 核心规则：
//   - 临时对象只活到产生它那条 *完整表达式* 结束（即下一个 `;`）。
//   - 但 const T& 与 T&& 这类 *直接* 绑定到纯右值的引用可以把临时对象的寿命
//     延长到该引用的作用域结束 —— 这是少数能"延寿"的情形之一。
//   - 注意：只有 *直接* 绑定才有效。从函数返回引用、把引用塞进结构体、再
//     从中拿出来都会破坏延寿条件 —— 这是返回 string_view / 临时引用的经典坑。
//   - C++23 起，range-based for 的初始化器中产生的 *所有* 临时对象寿命也会
//     延长到 for 循环结束（修复了"for(auto& x : f().items()) 悬空"的老坑）。
//
// 本 demo 通过带计数的 Probe 显式打印构造/析构时刻，让"延寿/不延寿"一目了然。
// 反例（返回局部引用 / 通过函数包装绑定的引用）默认不编译；定义
// MCPP_DEMONSTRATE_UB 才会启用，便于学习者主动观察 UB。

#include <iostream>
#include <string>
#include <utility>

namespace {

struct Probe {
    std::string name;
    static int alive;

    explicit Probe(std::string n) : name(std::move(n)) {
        ++alive;
        std::cout << "  + ctor  " << name << "  (alive=" << alive << ")\n";
    }
    Probe(Probe const& o) : name(o.name + "/copy") {
        ++alive;
        std::cout << "  + copy  " << name << "  (alive=" << alive << ")\n";
    }
    Probe(Probe&&) = delete;
    Probe& operator=(Probe const&) = delete;
    Probe& operator=(Probe&&) = delete;
    ~Probe() {
        --alive;
        std::cout << "  - dtor  " << name << "  (alive=" << alive << ")\n";
    }
};

int Probe::alive = 0;

[[nodiscard]] Probe makeProbe(std::string n) {
    return Probe{std::move(n)};
}

#if defined(MCPP_DEMONSTRATE_UB)
// 反例（默认禁用）：返回的"引用"指向 return 行结束就死亡的临时。
[[nodiscard]] Probe const& dangerousRef() {
    return Probe{"inside_func"};
}
#endif

}  // namespace

int main() {
    std::cout << "[1] 临时对象只活到 ; ——\n";
    {
        std::cout << "  before-stmt\n";
        std::string s = makeProbe("temp1").name;  // 拷贝出 string 后立刻销毁临时
        std::cout << "  after-stmt: s=" << s << "\n";
    }

    std::cout << "\n[2] const& 直接绑定纯右值 —— 寿命延长到引用作用域\n";
    {
        Probe const& ref = makeProbe("temp2");  // 临时寿命延长到块末尾
        std::cout << "  middle: alive=" << Probe::alive << " name=" << ref.name << "\n";
    }
    std::cout << "  alive after block = " << Probe::alive << "\n";

    std::cout << "\n[3] 右值引用同样可以延寿\n";
    {
        Probe&& rref = makeProbe("temp3");
        std::cout << "  middle: name=" << rref.name << "\n";
    }

    std::cout << "\n[4] 反例：返回值经过函数 / 成员就丢失延寿\n";
#if defined(MCPP_DEMONSTRATE_UB)
    {
        Probe const& bad = dangerousRef();
        std::cout << "  alive after returning dangling ref = " << Probe::alive
                  << " (临时早已析构)\n";
        (void)bad;
    }
#else
    std::cout << "  <编译时定义 MCPP_DEMONSTRATE_UB 才会真的执行>\n";
#endif

    std::cout << "\n[5] C++23 起 range-based for 的初始化器内所有临时都延寿\n";
    {
        struct Bag {
            int data[3]{1, 2, 3};
            int* begin() {
                return data;
            }
            int* end() {
                return data + 3;
            }
        };
        for (int x : Bag{}) {
            std::cout << "  for x=" << x << "\n";
        }
    }

    return 0;
}
