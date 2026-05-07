// dynamic_cast 与 RTTI（typeid / type_index）的行为测试。

#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>

#include <gtest/gtest.h>

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
};

struct Swims {
    virtual ~Swims() = default;
};
struct Walks {
    virtual ~Walks() = default;
};
struct Frog : Swims, Walks {};

}  // namespace

TEST(DynamicCast, PointerSucceedsForCorrectDerived) {
    std::unique_ptr<Animal> a = std::make_unique<Dog>();
    auto* d = dynamic_cast<Dog*>(a.get());
    ASSERT_NE(d, nullptr);
    EXPECT_EQ(d->sound(), "woof");
}

TEST(DynamicCast, PointerReturnsNullOnWrongType) {
    std::unique_ptr<Animal> a = std::make_unique<Dog>();
    EXPECT_EQ(dynamic_cast<Cat*>(a.get()), nullptr);
}

TEST(DynamicCast, ReferenceThrowsBadCastOnWrongType) {
    Cat c;
    Animal& a = c;
    EXPECT_THROW({ static_cast<void>(dynamic_cast<Dog&>(a)); }, std::bad_cast);
}

TEST(DynamicCast, SidecastAcrossSiblingBases) {
    Frog f;
    Swims& s = f;
    auto& w = dynamic_cast<Walks&>(s);
    EXPECT_EQ(&w, static_cast<Walks*>(&f));
}

TEST(DynamicCast, ToVoidGivesMostDerivedAddress) {
    Frog f;
    Swims& s = f;
    Walks& w = f;
    void const* via_swims = dynamic_cast<void const*>(&s);
    void const* via_walks = dynamic_cast<void const*>(&w);
    EXPECT_EQ(via_swims, via_walks);
    EXPECT_EQ(via_swims, static_cast<void const*>(&f));
}

TEST(Rtti, TypeidComparesActualRuntimeType) {
    std::unique_ptr<Animal> a = std::make_unique<Dog>();
    Animal& ref = *a;
    EXPECT_EQ(typeid(ref), typeid(Dog));
    EXPECT_NE(typeid(ref), typeid(Cat));
}

TEST(Rtti, TypeIndexIsHashableAndOrderable) {
    std::map<std::type_index, std::string> labels;
    labels.emplace(typeid(int), "int");
    labels.emplace(typeid(double), "double");
    labels.emplace(typeid(std::string), "string");

    EXPECT_EQ(labels.at(typeid(int)), "int");
    EXPECT_EQ(labels.at(typeid(double)), "double");
    EXPECT_EQ(labels.size(), 3U);
}
