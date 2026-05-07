// 切片问题（slicing）与基于多态的修复（virtual clone）。
//
// 关键点：
//   - 当用 *基类按值* 接收派生对象时，派生部分被静默丢弃 —— 这就是切片。
//   - 如果你写 Base& = Derived 然后 Base& = OtherBase，编译器调用的是
//     Base::operator=，只覆盖基类子对象切片 —— 派生类成员保持不变（同样的坑）。
//   - C++ Core Guidelines C.67：多态基类应该把 copy/move 设为 protected 或 = delete，
//     强迫使用 virtual clone() / 指针保留多态。本 demo 给出"反例 → 修复"对照。

#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace {

// ---- 反例：可拷贝的多态基类 ----
struct Person {
    std::string name;

    explicit Person(std::string n) : name(std::move(n)) {}
    virtual ~Person() = default;
    virtual void hello() const {
        std::cout << "  Person::hello()  name=" << name << "\n";
    }
};

struct Student : Person {
    int student_id;

    Student(std::string n, int id) : Person(std::move(n)), student_id(id) {}
    void hello() const override {
        std::cout << "  Student::hello() name=" << name << " id=" << student_id << "\n";
    }
};

// ---- 修复：把基类 copy/move 设为 protected，外部用 clone() ----
class Cloneable {
public:
    virtual ~Cloneable() = default;
    [[nodiscard]] virtual std::unique_ptr<Cloneable> clone() const = 0;
    virtual void hello() const = 0;

    Cloneable() = default;

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
    void hello() const override {
        std::cout << "  StudentCloneable::hello() name=" << name_ << " id=" << student_id_ << "\n";
    }

private:
    std::string name_;
    int student_id_;
};

}  // namespace

int main() {
    std::cout << "[1] 反例：按值传递切片\n";
    {
        Student s{"Alice", 1001};
        // 这里发生切片：p 只持有 Person 子对象 —— 这就是 demo 的核心。
        // NOLINTNEXTLINE(cppcoreguidelines-slicing,performance-unnecessary-copy-initialization)
        Person p = s;
        p.hello();  // -> Person::hello (尽管 hello 是 virtual)
        s.hello();
    }

    std::cout << "\n[2] 反例：基类引用赋值，派生成员不变\n";
    {
        Student s1{"Alice", 1001};
        Student s2{"Bob", 2002};
        Person& p1 = s1;
        Person const& p2 = s2;
        p1 = p2;  // 实际调用 Person::operator= —— 只覆盖 name
        std::cout << "  s1.name=" << s1.name << " s1.id=" << s1.student_id
                  << "  (id 没被覆盖！)\n";
    }

    std::cout << "\n[3] 修复：把多态对象保存在指针 / 智能指针里\n";
    {
        std::unique_ptr<Person> p = std::make_unique<Student>("Carol", 3003);
        p->hello();  // -> Student::hello()
    }

    std::cout << "\n[4] 修复：clone() 模式 —— 在 *正确派生类型* 上完成拷贝\n";
    {
        std::unique_ptr<Cloneable> orig = std::make_unique<StudentCloneable>("Dave", 4004);
        std::unique_ptr<Cloneable> copy = orig->clone();
        std::cout << "  orig: ";
        orig->hello();
        std::cout << "  copy: ";
        copy->hello();
    }

    return 0;
}
