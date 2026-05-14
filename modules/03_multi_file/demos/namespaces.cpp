// 命名空间综合演示：基础用法、C++17 嵌套语法、using 声明、
// 命名空间别名、inline namespace 版本控制。

#include <cmath>
#include <iostream>
#include <string>

// ========== 1. 基础命名空间 ==========
namespace graphics {

struct Point {
    double x;
    double y;
};

double distance(Point const& a, Point const& b);

}  // namespace graphics

// 函数定义可以在命名空间块外部，用完全限定名
double graphics::distance(Point const& a, Point const& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt((dx * dx) + (dy * dy));
}

// ========== 2. C++17 嵌套命名空间 ==========
namespace company::department::team {

std::string teamName() { return "Core Engine"; }

}  // namespace company::department::team

// ========== 3. 命名空间扩展 —— 同名命名空间可多次打开 ==========
namespace graphics {

Point origin() { return {.x = 0.0, .y = 0.0}; }

}  // namespace graphics

// ========== 4. inline namespace 用于版本控制 ==========
namespace math_lib {

// v2 是默认版本（inline），用户可直接 math_lib::add()
inline namespace v2 {
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
}  // namespace v2

// 旧版本仍可通过 math_lib::v1::add() 访问
namespace v1 {
int add(int a, int b) { return a + b; }
}  // namespace v1

}  // namespace math_lib

// ========== 5. 命名空间别名 ==========
namespace cdt = company::department::team;

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== 命名空间演示 =====\n\n";

    // 1. 基础用法 —— 完全限定名
    std::cout << "-- 基础命名空间 --\n";
    graphics::Point p1{.x = 3.0, .y = 4.0};
    graphics::Point p2 = graphics::origin();
    std::cout << "distance({3,4}, origin) = " << graphics::distance(p1, p2)
              << '\n';
    std::cout << '\n';

    // 2. C++17 嵌套命名空间
    std::cout << "-- C++17 嵌套命名空间 --\n";
    std::cout << "Team name: " << company::department::team::teamName()
              << '\n';
    std::cout << '\n';

    // 3. 命名空间别名
    std::cout << "-- 命名空间别名 --\n";
    std::cout << "cdt::teamName() = " << cdt::teamName() << '\n';
    std::cout << '\n';

    // 4. using 声明 —— 引入特定名字
    std::cout << "-- using 声明 --\n";
    {
        using graphics::distance;
        using graphics::Point;
        Point a{.x = 1.0, .y = 0.0};
        Point b{.x = 0.0, .y = 1.0};
        std::cout << "distance({1,0}, {0,1}) = " << distance(a, b) << '\n';
    }
    std::cout << '\n';

    // 5. inline namespace —— 默认使用最新版本
    std::cout << "-- inline namespace 版本控制 --\n";
    std::cout << "math_lib::add(2, 3) = " << math_lib::add(2, 3)
              << " (默认 v2)\n";
    std::cout << "math_lib::multiply(2, 3) = " << math_lib::multiply(2, 3)
              << " (v2 独有)\n";
    std::cout << "math_lib::v1::add(2, 3) = " << math_lib::v1::add(2, 3)
              << " (显式旧版本)\n";
    std::cout << "math_lib::v2::add(2, 3) = " << math_lib::v2::add(2, 3)
              << " (显式新版本)\n";

    return 0;
}
