// std::any —— 类型安全的 void*。
//
// 关键点：
//   - std::any 可以装载 *任何* 满足拷贝构造的对象。
//   - 取出时必须 std::any_cast<T>(a)，且 T 必须与存储类型 *完全相同*（cv 与
//     引用可加），否则按值版本抛 std::bad_any_cast，按指针版本返回 nullptr。
//   - std::any 不是 Python 的"动态变量"：它仍是强类型 —— 你必须知道存的是什么。
//   - SBO（小缓冲区优化）：常见实现把"小、可移动构造 noexcept"的对象放在 any
//     的栈上字节里，避免堆分配 —— sizeof(std::any) 远比一个指针大。

#include <any>
#include <iostream>
#include <map>
#include <string>
#include <typeinfo>
#include <utility>

namespace {

void print(std::any const& a) {
    if (!a.has_value()) {
        std::cout << "  <empty>\n";
        return;
    }
    std::cout << "  type = " << a.type().name();
    if (auto const* pi = std::any_cast<int>(&a); pi != nullptr) {
        std::cout << "  int=" << *pi;
    } else if (auto const* pd = std::any_cast<double>(&a); pd != nullptr) {
        std::cout << "  double=" << *pd;
    } else if (auto const* ps = std::any_cast<std::string>(&a); ps != nullptr) {
        std::cout << "  string=\"" << *ps << '"';
    }
    std::cout << '\n';
}

}  // namespace

// std::cout / std::any_cast 在路径分析下"理论上"可能抛；demo 里我们就是要
// 演示 bad_any_cast 等异常 —— 让它们离开 main 是预期行为。
// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "[1] 装载 / 取出 / 重置\n";
    {
        std::any a;
        print(a);

        a = 42;
        print(a);

        a = std::string{"hello"};
        print(a);

        a.reset();
        std::cout << "  after reset, has_value = " << a.has_value() << '\n';
    }

    std::cout << "\n[2] any_cast：类型不匹配 -> bad_any_cast\n";
    {
        std::any a = 1LL;  // long long
        try {
            // 必须 cast 到完全相同的类型
            std::cout << "  any_cast<int>(long long) = " << std::any_cast<int>(a) << '\n';
        } catch (std::bad_any_cast const& e) {
            std::cout << "  caught: " << e.what() << '\n';
        }
        // 指针版本：失败返回 nullptr，不抛
        if (std::any_cast<int>(&a) == nullptr) {
            std::cout << "  any_cast<int>(&a) == nullptr (与文档一致)\n";
        }
        std::cout << "  正确：any_cast<long long>(a) = " << std::any_cast<long long>(a) << '\n';
    }

    std::cout << "\n[3] in_place_type 构造 + emplace\n";
    {
        std::any a{std::in_place_type<std::string>, 5, '*'};  // "*****"
        std::cout << "  inplace string = " << std::any_cast<std::string>(a) << '\n';
        a.emplace<std::string>("emplaced");
        std::cout << "  after emplace  = " << std::any_cast<std::string>(a) << '\n';
    }

    std::cout << "\n[4] 用作配置项集合（异构 map）\n";
    {
        std::map<std::string, std::any> config;
        config.emplace("port", 8080);
        config.emplace("hostname", std::string{"localhost"});
        config.emplace("ratio", 0.75);

        for (auto const& [k, v] : config) {
            std::cout << "  " << k << " : ";
            print(v);
        }
    }

    std::cout << "\n[5] sizeof(std::any) 反映 SBO 大小\n";
    {
        std::cout << "  sizeof(std::any) = " << sizeof(std::any) << '\n';
        std::cout << "  sizeof(void*)    = " << sizeof(void*) << '\n';
    }

    return 0;
}
