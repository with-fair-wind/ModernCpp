// dynamic_cast 与 RTTI（typeid / type_index）。
//
// 关键点：
//   - dynamic_cast 仅对多态类型生效 —— 至少有一个虚函数（通常是虚析构）。
//   - dynamic_cast<T*>(p)：失败返回 nullptr；dynamic_cast<T&>(r)：失败抛 bad_cast。
//   - 可做下转 / 同层多重继承的"侧向转换"（sidecast）；普通 static_cast 无法在
//     无直接继承关系的兄弟之间转换。
//   - dynamic_cast<void*>(p) 给你"指向最派生对象起点"的指针 —— 极少用，但是少数
//     还能在 base* 上拿到完整对象首地址的方式。
//   - typeid(expr) -> std::type_info；用 std::type_index 包装可放进关联容器。
//   - RTTI 在性能敏感路径上慢很多；多数场景应该靠 virtual 派发或 std::variant
//     代替；这里只展示 API。

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>

namespace {

struct Animal {
    virtual ~Animal() = default;
    [[nodiscard]] virtual std::string sound() const = 0;
};

struct Cat : Animal {
    [[nodiscard]] std::string sound() const override {
        return "meow";
    }
};

struct Dog : Animal {
    [[nodiscard]] std::string sound() const override {
        return "woof";
    }
    [[nodiscard]] virtual std::string fetch() const {
        return "fetch!";
    }
};

// 演示侧向转换：菱形里的两条腿
struct Swims {
    virtual ~Swims() = default;
    [[nodiscard]] virtual std::string swim() const = 0;
};
struct Walks {
    virtual ~Walks() = default;
    [[nodiscard]] virtual std::string walk() const = 0;
};
struct Frog : Swims, Walks {
    [[nodiscard]] std::string swim() const override {
        return "frog-swim";
    }
    [[nodiscard]] std::string walk() const override {
        return "frog-walk";
    }
};

}  // namespace

int main() {
    std::cout << "[1] 指针下转：成功返回派生指针，失败返回 nullptr\n";
    {
        std::unique_ptr<Animal> a = std::make_unique<Dog>();
        if (auto* d = dynamic_cast<Dog*>(a.get()); d != nullptr) {
            std::cout << "  Dog* 拿到了：" << d->sound() << " / " << d->fetch() << '\n';
        }
        if (dynamic_cast<Cat*>(a.get()) == nullptr) {
            std::cout << "  Cat* 失败，返回 nullptr（底层不是 Cat）\n";
        }
    }

    std::cout << "\n[2] 引用下转：失败抛 std::bad_cast\n";
    {
        Cat c;
        Animal& a = c;
        try {
            auto& d = dynamic_cast<Dog&>(a);  // 必败
            (void)d;
        } catch (std::bad_cast const& e) {
            std::cout << "  caught bad_cast: " << e.what() << '\n';
        }
    }

    std::cout << "\n[3] 侧向转换（sidecast）：从 Swims 到 Walks（两边都不直接继承）\n";
    {
        Frog f;
        Swims& s = f;
        // static_cast<Walks&>(s) —— 编译错，不是直接继承关系
        auto& w = dynamic_cast<Walks&>(s);
        std::cout << "  via Swims&  -> walk(): " << w.walk() << '\n';
    }

    std::cout << "\n[4] dynamic_cast<void*>：取最派生对象起点\n";
    {
        Frog f;
        Swims& s = f;
        Walks& w = f;
        // 注意：&s 与 &w 通常 *不* 等于 &f；dynamic_cast<void*> 才是
        void const* most_derived_via_s = dynamic_cast<void const*>(&s);
        void const* most_derived_via_w = dynamic_cast<void const*>(&w);
        std::cout << "  same most-derived: "
                  << (most_derived_via_s == most_derived_via_w ? "yes" : "no") << '\n';
    }

    std::cout << "\n[5] typeid + type_index：把类型当 map key\n";
    {
        std::map<std::type_index, std::string> labels;
        Cat c;
        Dog d;
        labels.emplace(typeid(c), "Cat instance");
        labels.emplace(typeid(d), "Dog instance");
        labels.emplace(typeid(int), "int");

        for (auto const& [k, v] : labels) {
            std::cout << "  " << k.name() << " => " << v << '\n';
        }
    }

    return 0;
}
