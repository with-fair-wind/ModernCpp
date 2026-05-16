// 两阶段名称查找：依赖名在定义处与实例化处各查一次；基类成员需 this-> 或限定名。
//
// 典型坑：模板派生类中直接写 `value` 可能找不到基类成员。

#include <iostream>

namespace {

template <typename Derived>
struct Logger {
    void log() {
        // 依赖名：第二阶段的实际调用依赖 Derived::message()。
        static_cast<Derived*>(this)->message();
    }

private:
    friend Derived;
    Logger() = default;
};

struct ConsoleLogger : Logger<ConsoleLogger> {
    static void message() {
        std::cout << "console\n";
    }
};

template <typename T>
struct Holder {
    int value_{42};
};

template <typename T>
struct Viewer : Holder<T> {
    int readValue() {
        // 无 this-> 时 `value_` 对编译器而言不是待决名，可能无法穿透依赖基类。
        return this->value_;
    }
};

}  // namespace

int main() {
    ConsoleLogger c;
    c.log();

    Viewer<int> v;
    std::cout << "Viewer::readValue = " << v.readValue() << '\n';

    return 0;
}
