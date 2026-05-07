// 多重继承基础 + 同名成员消歧 + using 声明。
//
// 关键点：
//   - 一个类可以有多个直接基类；每多一个 *多态* 基类，就多一个 vptr。
//   - 不同基类有同名成员时，派生类访问需要 Base::method 显式消歧，否则编译报错。
//   - using Base::method 把基类成员引入派生类作用域，重新打开访问 / 暴露重载。
//   - 构造函数继承（using Base::Base）允许派生类直接复用父类构造函数 —— 但
//     编译器自动生成的特殊成员（如默认构造）不会被继承。

#include <iostream>
#include <string>
#include <utility>

namespace {

struct Swimmer {
    int swim_speed = 50;
    void move() const {
        std::cout << "  Swimmer::move() —— 游泳，速度=" << swim_speed << "\n";
    }
    void stamina() const {
        std::cout << "  Swimmer::stamina = " << swim_speed << "\n";
    }
};

struct Runner {
    int run_speed = 80;
    void move() const {
        std::cout << "  Runner::move() —— 奔跑，速度=" << run_speed << "\n";
    }
    void stamina() const {
        std::cout << "  Runner::stamina = " << run_speed << "\n";
    }
};

// 多重继承：派生类同时是 Swimmer 与 Runner
struct Triathlete : Swimmer, Runner {
    // 解决同名成员二义性：用 using 显式选择 / 暴露 / 重命名
    using Runner::move;      // move() 沿用 Runner 版本
    using Swimmer::stamina;  // stamina() 沿用 Swimmer 版本
};

// 构造函数继承：BaseHolder 把 std::string 的构造交给基类
struct StringHolder {
    std::string value;
    explicit StringHolder(std::string v) : value(std::move(v)) {}
    explicit StringHolder(int n) : value(n, 'x') {}  // n 个 'x'
    void show() const {
        std::cout << "  value=" << value << "\n";
    }
};

struct LoggingHolder : StringHolder {
    using StringHolder::StringHolder;  // 继承所有 StringHolder 构造函数
    void show() const {
        std::cout << "[log] ";
        StringHolder::show();
    }
};

}  // namespace

int main() {
    std::cout << "[1] 同名成员的二义性 —— 用 using 解决\n";
    {
        Triathlete t;
        t.move();     // 因为 using Runner::move; -> Runner 版本
        t.stamina();  // 因为 using Swimmer::stamina; -> Swimmer 版本

        // 没有 using 也可显式 t.Swimmer::move();
        t.Swimmer::move();
        t.Runner::stamina();
    }

    std::cout << "\n[2] 构造函数继承：派生类自动暴露父类的所有构造\n";
    {
        LoggingHolder a{"abc"};  // 用 std::string 构造
        LoggingHolder b{3};      // 用 int 构造（n 个 'x'）
        a.show();
        b.show();
    }

    std::cout << "\n[3] 多重继承下的对象大小（每个多态基类各占一个 vptr）\n";
    {
        struct A {
            virtual ~A() = default;
        };
        struct B {
            virtual ~B() = default;
        };
        struct C : A, B {};
        std::cout << "  sizeof(A) = " << sizeof(A) << '\n';
        std::cout << "  sizeof(B) = " << sizeof(B) << '\n';
        std::cout << "  sizeof(C) = " << sizeof(C) << "  (>= 2 * sizeof(void*))\n";
    }

    return 0;
}
