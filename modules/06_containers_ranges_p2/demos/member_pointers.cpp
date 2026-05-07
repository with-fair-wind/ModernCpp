// 成员函数指针 / 数据成员指针 / std::invoke / std::bind_front。
//
// 关键点：
//   - 非静态成员函数总是绑定到一个对象（隐含 this 参数）；它的指针类型是
//     `Ret (Class::*)(Args...) cv-quals`，调用语法是 (obj.*ptr)(args)、(p->*ptr)(args)。
//   - C++17 起，std::invoke 把 ".*"/"->*"/"()" 这些不一致的语法统一成函数式调用，
//     非常适合写泛型代码。
//   - 数据成员指针类型为 `T Class::*`，invoke 把它视为"取成员"。
//   - C++20 起的 std::bind_front 优先于 std::bind：语义直观、转发明确，且无 std::ref
//     之类的暗坑。

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

namespace {

struct Person {
    std::string name;
    int age{};

    void grow(int years) {
        age += years;
    }
    [[nodiscard]] std::string greet(std::string_view from) const {
        return "Hello " + std::string{from} + ", I'm " + name;
    }
};

}  // namespace

int main() {
    // 1) 成员函数指针的取址必须 &Class::method（不会 decay）
    using GrowPtr = void (Person::*)(int);
    GrowPtr grow = &Person::grow;

    Person alice{.name = "alice", .age = 30};
    (alice.*grow)(5);  // 注意必须有括号，因为 () 优先级更高
    std::cout << "alice.age after grow(5): " << alice.age << '\n';

    // 2) 通过对象指针：使用 ->*
    Person* ptr_alice = &alice;
    (ptr_alice->*grow)(2);
    std::cout << "alice.age after grow(2) via ptr: " << alice.age << '\n';

    // 3) std::invoke 统一三种语法：obj.*func / ptr->*func / 普通调用
    std::cout << "invoke greet(obj)  : " << std::invoke(&Person::greet, alice, "bob") << '\n';
    std::cout << "invoke greet(ptr)  : " << std::invoke(&Person::greet, &alice, "bob") << '\n';

    // 4) 数据成员指针：也能被 invoke 当作"取成员"
    auto name_ptr = &Person::name;
    std::cout << "invoke .name       : " << std::invoke(name_ptr, alice) << '\n';

    // 5) std::invoke_r<R>：把结果显式转换到 R
    auto len = std::invoke_r<std::int64_t>(&std::string::size, alice.name);
    std::cout << "invoke_r<int64> sz : " << len << '\n';

    // 6) std::bind_front：把前若干个参数固化，得到新的 functor
    auto greet_from_anthropic = std::bind_front(&Person::greet, alice, "anthropic");
    std::cout << "bind_front result  : " << greet_from_anthropic() << '\n';

    auto add = [](int a, int b, int c) { return a + b + c; };
    auto add10 = std::bind_front(add, 10);
    std::cout << "bind_front add10   : " << add10(20, 30) << '\n';

    return 0;
}
