#include <iostream>
#include <tuple>

struct Point
{
    int x;
    int y;
};

void demo_v1()
{
    Point p{1, 2};
    auto &&[x, y] = p; // x=1, y=2
    x++;
    std::cout << "x: " << x << ", y: " << y << std::endl;
    std::cout << "p.x: " << p.x << ", p.y: " << p.y << std::endl;
}

void demo_v2()
{
    std::tuple<int, std::string> t(3, "hello");
    auto [num, str] = t; // num=3, str="hello"
}

class Vector3D
{
private:
    double x_, y_, z_;

public:
    Vector3D(double x, double y, double z) : x_(x), y_(y), z_(z) {}

    template <size_t N>
    auto get() const
    {
        if constexpr (N == 0)
            return x_;
        else if constexpr (N == 1)
            return y_;
        else if constexpr (N == 2)
            return z_;
    }

    template <size_t N>
    auto &get()
    {
        if constexpr (N == 0)
            return x_;
        else if constexpr (N == 1)
            return y_;
        else if constexpr (N == 2)
            return z_;
    }

    void print() const
    {
        std::cout << "x: " << x_ << ", y: " << y_ << ", z: " << z_ << std::endl;
    }
};

namespace std
{
    // 特化 std::tuple_size 表示元素数量
    template <>
    struct tuple_size<Vector3D> : integral_constant<size_t, 3>
    {
    };

    // 特化 std::tuple_element 表示每个元素的类型
    template <size_t N>
    struct tuple_element<N, Vector3D>
    {
        using type = double; // 所有元素类型均为 double
    };
}

void demo_v3()
{
    Vector3D v(1.0, 2.0, 3.0);
    const auto &&[x, y, z] = std::move(v); // x=1.0, y=2.0, z=3.0
    // x += 2.2;
    v.print();
}

int main()
{
    demo_v3();
    return 0;
}
