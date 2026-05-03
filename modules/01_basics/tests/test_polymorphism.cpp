// 虚函数分派会选取最派生的 override；虚析构能保证通过基类指针
// 删除时两层析构都被执行。

#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace {

class Animal {
public:
    Animal() = default;
    Animal(Animal const&) = default;
    Animal& operator=(Animal const&) = default;
    Animal(Animal&&) = default;
    Animal& operator=(Animal&&) = default;
    virtual ~Animal() = default;
    virtual std::string speak() const = 0;
};

class Dog : public Animal {
public:
    std::string speak() const override {
        return "woof";
    }
};

class Cat final : public Animal {
public:
    std::string speak() const override {
        return "meow";
    }
};

class CountingBase {
public:
    explicit CountingBase(int* counter) : counter_{counter} {}
    CountingBase(CountingBase const&) = delete;
    CountingBase& operator=(CountingBase const&) = delete;
    CountingBase(CountingBase&&) = delete;
    CountingBase& operator=(CountingBase&&) = delete;
    virtual ~CountingBase() {
        ++*counter_;
    }

private:
    int* counter_;
};

class CountingDerived : public CountingBase {
public:
    CountingDerived(int* base, int* derived) : CountingBase{base}, derived_counter_{derived} {}
    ~CountingDerived() override {
        ++*derived_counter_;
    }

private:
    int* derived_counter_;
};

}  // namespace

TEST(Polymorphism, VirtualDispatchPicksMostDerived) {
    std::unique_ptr<Animal> p = std::make_unique<Cat>();
    EXPECT_EQ(p->speak(), "meow");
    p = std::make_unique<Dog>();
    EXPECT_EQ(p->speak(), "woof");
}

TEST(Polymorphism, VirtualDestructorRunsBothLayers) {
    int base = 0;
    int derived = 0;
    {
        std::unique_ptr<CountingBase> p = std::make_unique<CountingDerived>(&base, &derived);
    }
    EXPECT_EQ(base, 1);
    EXPECT_EQ(derived, 1);
}
