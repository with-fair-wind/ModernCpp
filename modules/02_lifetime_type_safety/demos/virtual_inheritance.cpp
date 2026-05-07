// 致命菱形（dreaded diamond）与虚继承。
//
// 关键点：
//   - 普通的菱形继承会让最派生类持有 *两份* 公共基类子对象 —— 既冗余，也无法
//     直接通过公共基类引用最派生对象（二义性）。
//   - 虚继承（virtual public Base）让所有派生路径共享同一份基类 —— 二义性消除，
//     但运行时 / 空间开销更大（虚基偏移表）。
//   - 虚基类的构造函数由 *最派生类* 直接调用，不能通过中间基类间接调用。
//   - 一般工程上不鼓励虚继承；混入模式（mixin pattern）通常是更轻巧的替代。

#include <iostream>
#include <string>
#include <utility>

namespace {

// ---- 反例：未虚继承的菱形 ----
struct Animal {
    std::string name;
    explicit Animal(std::string n) : name(std::move(n)) {
        std::cout << "  Animal::ctor   name=" << name << "\n";
    }
    void breathe() const {
        std::cout << "  " << name << "::breathe()\n";
    }
};

struct Elephant : Animal {
    using Animal::Animal;
};

struct Seal : Animal {
    using Animal::Animal;
};

struct ElephantSealBad : Elephant, Seal {
    ElephantSealBad(std::string e_name, std::string s_name)
        : Elephant(std::move(e_name)), Seal(std::move(s_name)) {}
};

// ---- 修复：虚继承 ----
struct AnimalV {
    std::string name;
    explicit AnimalV(std::string n) : name(std::move(n)) {
        std::cout << "  AnimalV::ctor  name=" << name << "\n";
    }
    void breathe() const {
        std::cout << "  " << name << "::breathe()\n";
    }
};

struct ElephantV : virtual AnimalV {
    using AnimalV::AnimalV;
};

struct SealV : virtual AnimalV {
    using AnimalV::AnimalV;
};

struct ElephantSealOK : ElephantV, SealV {
    // 虚基的构造由最派生类直接调用 —— 否则编译器用默认构造（这里没有默认构造，
    // 必须显式写）。
    explicit ElephantSealOK(std::string n)
        : AnimalV(std::move(n)), ElephantV("ignored"), SealV("ignored") {}
};

}  // namespace

int main() {
    std::cout << "[1] 反例：未虚继承 —— 两份基类子对象\n";
    {
        ElephantSealBad bad{"Ele", "S"};
        std::cout << "  bad.Elephant::name = " << bad.Elephant::name << "\n";
        std::cout << "  bad.Seal::name     = " << bad.Seal::name << "\n";
        std::cout << "  sizeof(bad) = " << sizeof(bad) << "  (两个 string)\n";
        // bad.name;        // 编译错误：二义
        // Animal& a = bad; // 编译错误：二义
        bad.Elephant::breathe();
        bad.Seal::breathe();
    }

    std::cout << "\n[2] 修复：虚继承 —— 共享同一份 AnimalV\n";
    {
        ElephantSealOK ok{"OK-seal"};
        std::cout << "  ok.name = " << ok.name << "\n";  // 不再二义
        AnimalV& a = ok;
        a.breathe();
        std::cout << "  sizeof(ok) = " << sizeof(ok) << "  (含虚基偏移开销)\n";
    }

    return 0;
}
