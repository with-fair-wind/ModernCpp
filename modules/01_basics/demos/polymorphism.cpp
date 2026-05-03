// 虚函数分派、override / final、抽象基类、虚析构函数。
//
// 为什么必须有虚析构：通过 Base* 删除 Derived，但基类析构是非虚的话，
// 只会跑 Base 的析构，对象的其余部分被泄漏。任何打算被多态使用的基类
// 都需要一个虚析构。

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

class Animal {
public:
    explicit Animal(std::string name) : name_{std::move(name)} {}
    virtual ~Animal() = default;
    [[nodiscard]] virtual std::string speak() const = 0;  // 纯虚 -> 抽象类
    [[nodiscard]] std::string const& name() const {
        return name_;
    }

private:
    std::string name_;
};

class Dog : public Animal {
public:
    using Animal::Animal;
    [[nodiscard]] std::string speak() const override {
        return "woof";
    }
};

// `final` 终止重写链。通过 `Cat&` / `Cat*` 调用时，编译器可放心做
// 去虚化（devirtualization），因为不可能再有派生类重写。
class Cat final : public Animal {
public:
    using Animal::Animal;
    [[nodiscard]] std::string speak() const override {
        return "meow";
    }
};

}  // namespace

int main() {
    std::vector<std::unique_ptr<Animal>> pets;
    pets.push_back(std::make_unique<Dog>("Rex"));
    pets.push_back(std::make_unique<Cat>("Mochi"));

    for (auto const& pet : pets) {
        std::cout << pet->name() << " says " << pet->speak() << '\n';
    }
    return 0;
}
