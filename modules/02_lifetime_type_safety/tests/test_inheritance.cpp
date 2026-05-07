// 多重继承 + using 声明 + 虚继承 + 混入模式的测试。

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace {

// --- using 声明：构造函数继承 + 重载暴露 ---
struct Holder {
    std::string value;
    explicit Holder(std::string v) : value(std::move(v)) {}
    explicit Holder(int n) : value(static_cast<std::size_t>(n), 'x') {}
};

struct Wrapper : Holder {
    using Holder::Holder;
};

// --- 多重继承同名成员的消歧 ---
struct A {
    int who = 1;
    [[nodiscard]] int whoAmI() const {
        return who;
    }
};
struct B {
    int who = 2;
    [[nodiscard]] int whoAmI() const {
        return who;
    }
};
struct AB : A, B {
    using A::whoAmI;  // 选择 A 版本
};

// --- 致命菱形（虚继承解 vs 不解） ---
struct Animal {
    int hp = 100;
};
struct Elephant : Animal {};
struct Seal : Animal {};
struct ElephantSealBad : Elephant, Seal {};  // 两份 Animal 子对象

struct AnimalV {
    int hp = 100;
};
struct ElephantV : virtual AnimalV {};
struct SealV : virtual AnimalV {};
struct ElephantSealOK : ElephantV, SealV {};  // 共享一份 AnimalV

// --- 混入接口 ---
struct Attacker {
    virtual ~Attacker() = default;
    [[nodiscard]] virtual int attackPower() const = 0;
};
struct Defender {
    virtual ~Defender() = default;
    [[nodiscard]] virtual int defense() const = 0;
};
struct Soldier : Attacker, Defender {
    [[nodiscard]] int attackPower() const override {
        return 30;
    }
    [[nodiscard]] int defense() const override {
        return 20;
    }
};

}  // namespace

TEST(InheritedCtor, UsingExposesAllParentConstructors) {
    Wrapper a{"abc"};
    Wrapper b{3};
    EXPECT_EQ(a.value, "abc");
    EXPECT_EQ(b.value, "xxx");
}

TEST(MultipleInheritance, UsingDisambiguatesSameNamedMember) {
    AB obj{};
    EXPECT_EQ(obj.whoAmI(), 1);     // using A::whoAmI
    EXPECT_EQ(obj.A::whoAmI(), 1);  // 显式同样可以
    EXPECT_EQ(obj.B::whoAmI(), 2);
}

TEST(Diamond, NonVirtualHasTwoBaseSubobjects) {
    ElephantSealBad bad{};
    bad.Elephant::hp = 1;
    bad.Seal::hp = 2;
    EXPECT_EQ(bad.Elephant::hp, 1);
    EXPECT_EQ(bad.Seal::hp, 2);
    // bad.hp;       // 编译错误：二义
    // Animal& a=bad;  // 编译错误：二义
}

TEST(Diamond, VirtualBaseIsShared) {
    ElephantSealOK ok{};
    ok.hp = 7;  // 不再二义；只有一份 AnimalV
    AnimalV& as_animal = ok;
    EXPECT_EQ(as_animal.hp, 7);
    ElephantV& as_e = ok;
    SealV& as_s = ok;
    EXPECT_EQ(as_e.hp, 7);
    EXPECT_EQ(as_s.hp, 7);
}

TEST(MixinPattern, DispatchByCapability) {
    std::unique_ptr<Attacker> a = std::make_unique<Soldier>();
    EXPECT_EQ(a->attackPower(), 30);

    // 同一对象通过 Attacker 接口拿到的指针 cast 到兄弟接口 Defender
    auto* d = dynamic_cast<Defender*>(a.get());
    ASSERT_NE(d, nullptr);
    EXPECT_EQ(d->defense(), 20);
}
