// 切片问题与 clone() 模式。

#include <memory>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace {

struct Person {
    std::string name;

    explicit Person(std::string n) : name(std::move(n)) {}
    Person(Person const&) = default;
    Person(Person&&) = default;
    Person& operator=(Person const&) = default;
    Person& operator=(Person&&) = default;
    virtual ~Person() = default;
    [[nodiscard]] virtual std::string describe() const {
        return "Person(" + name + ")";
    }
};

struct Student : Person {
    int student_id;

    Student(std::string n, int id) : Person(std::move(n)), student_id(id) {}
    [[nodiscard]] std::string describe() const override {
        return "Student(" + name + "," + std::to_string(student_id) + ")";
    }
};

class Cloneable {
public:
    Cloneable() = default;
    virtual ~Cloneable() = default;
    [[nodiscard]] virtual std::unique_ptr<Cloneable> clone() const = 0;
    [[nodiscard]] virtual std::string describe() const = 0;

protected:
    Cloneable(Cloneable const&) = default;
    Cloneable(Cloneable&&) = default;
    Cloneable& operator=(Cloneable const&) = default;
    Cloneable& operator=(Cloneable&&) = default;
};

class StudentCloneable : public Cloneable {
public:
    StudentCloneable(std::string n, int id) : name_(std::move(n)), student_id_(id) {}

    [[nodiscard]] std::unique_ptr<Cloneable> clone() const override {
        return std::make_unique<StudentCloneable>(*this);
    }
    [[nodiscard]] std::string describe() const override {
        return "Student(" + name_ + "," + std::to_string(student_id_) + ")";
    }

private:
    std::string name_;
    int student_id_;
};

}  // namespace

TEST(Slicing, AssignByValueDropsDerivedPart) {
    Student s{"Alice", 1001};
    // NOLINTNEXTLINE(cppcoreguidelines-slicing,performance-unnecessary-copy-initialization)
    Person p = s;  // 切片：p 是 Person，不再持有 student_id
    EXPECT_EQ(p.describe(), "Person(Alice)");
    EXPECT_EQ(s.describe(), "Student(Alice,1001)");
}

TEST(Slicing, AssignThroughBaseRefOnlyTouchesBaseSubobject) {
    Student s1{"Alice", 1001};
    Student s2{"Bob", 2002};
    Person& base_ref = s1;
    // NOLINTNEXTLINE(cppcoreguidelines-slicing)
    base_ref = s2;  // 实际调用 Person::operator=，只覆盖 name；id 保留
    EXPECT_EQ(s1.name, "Bob");
    EXPECT_EQ(s1.student_id, 1001);
}

TEST(Slicing, PolymorphicPointerKeepsDerivedBehavior) {
    std::unique_ptr<Person> p = std::make_unique<Student>("Carol", 3003);
    EXPECT_EQ(p->describe(), "Student(Carol,3003)");
}

TEST(Slicing, CloneFixSurvivesPolymorphicCopy) {
    std::unique_ptr<Cloneable> orig = std::make_unique<StudentCloneable>("Dave", 4004);
    auto copy = orig->clone();
    EXPECT_EQ(copy->describe(), orig->describe());
    EXPECT_NE(copy.get(), orig.get());  // 不同对象
}
