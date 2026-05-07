// 混入模式（mixin pattern）—— 多重继承的"安全"用法。
//
// 关键点：
//   - 把每种"能力"做成只含纯虚函数 / 没有数据成员的抽象基类（接口），
//     派生类按需多重继承组合 —— 等价于 Java/C# 的 interface。
//   - 因为接口没有数据成员，菱形继承不会重复存数据；vptr 开销也是有限的。
//   - 这种风格让 "战争模拟里的可攻击 / 可防御 / 可移动单位" 这类场景能用
//     dynamic_cast 或 std::visit 在运行时按能力分派。

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

// 三个能力接口：纯接口（无数据成员，只有 = 0 的虚函数）
struct Attacker {
    virtual ~Attacker() = default;
    [[nodiscard]] virtual int attackPower() const = 0;
};

struct Defender {
    virtual ~Defender() = default;
    [[nodiscard]] virtual int defense() const = 0;
};

struct Movable {
    virtual ~Movable() = default;
    [[nodiscard]] virtual int speed() const = 0;
};

// Unit：所有单位的公共部分（命名 + 多态析构）
struct Unit {
    std::string name;
    explicit Unit(std::string n) : name(std::move(n)) {}
    virtual ~Unit() = default;
};

// Soldier：可攻击 + 可防御 + 可移动
struct Soldier : Unit, Attacker, Defender, Movable {
    using Unit::Unit;
    [[nodiscard]] int attackPower() const override {
        return 30;
    }
    [[nodiscard]] int defense() const override {
        return 20;
    }
    [[nodiscard]] int speed() const override {
        return 5;
    }
};

// Tower：可攻击 + 可防御，但 *不可* 移动
struct Tower : Unit, Attacker, Defender {
    using Unit::Unit;
    [[nodiscard]] int attackPower() const override {
        return 80;
    }
    [[nodiscard]] int defense() const override {
        return 100;
    }
};

// Worker：只可移动（侦察兵）
struct Worker : Unit, Movable {
    using Unit::Unit;
    [[nodiscard]] int speed() const override {
        return 8;
    }
};

}  // namespace

int main() {
    std::cout << "[1] 用 dynamic_cast 在 Unit* 上按能力分派\n";
    {
        std::vector<std::unique_ptr<Unit>> army;
        army.push_back(std::make_unique<Soldier>("S1"));
        army.push_back(std::make_unique<Tower>("T1"));
        army.push_back(std::make_unique<Worker>("W1"));

        for (auto const& u : army) {
            std::cout << "  " << u->name << ": ";
            if (auto const* a = dynamic_cast<Attacker const*>(u.get()); a != nullptr) {
                std::cout << "attack=" << a->attackPower() << ' ';
            }
            if (auto const* d = dynamic_cast<Defender const*>(u.get()); d != nullptr) {
                std::cout << "defense=" << d->defense() << ' ';
            }
            if (auto const* m = dynamic_cast<Movable const*>(u.get()); m != nullptr) {
                std::cout << "speed=" << m->speed() << ' ';
            }
            std::cout << '\n';
        }
    }

    std::cout << "\n[2] 接口集合的总伤害（只挑可攻击者）\n";
    {
        std::array<std::unique_ptr<Unit>, 3> party{
            std::make_unique<Soldier>("S2"),
            std::make_unique<Tower>("T2"),
            std::make_unique<Worker>("W2"),
        };
        int total = 0;
        for (auto const& u : party) {
            if (auto const* a = dynamic_cast<Attacker const*>(u.get()); a != nullptr) {
                total += a->attackPower();
            }
        }
        std::cout << "  total attack power = " << total << '\n';
    }

    return 0;
}
