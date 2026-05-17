// C++17 起 prvalue 在特定初始化语境下触发「强制复制消除」：
// 临时对象直接构造在最终存储上，拷贝/移动构造不必出现。

#include <iostream>

namespace {

class Brick {
public:
    Brick() {
        ++default_ctor;
        ++alive;
    }

    Brick(Brick const& /*unused*/) {
        ++copy_ctor;
        ++alive;
    }

    Brick(Brick&& /*unused*/) noexcept {
        ++move_ctor;
        ++alive;
    }

    Brick& operator=(Brick const&) = delete;
    Brick& operator=(Brick&&) = delete;

    ~Brick() {
        ++dtor;
        --alive;
    }

    static void reset() {
        default_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        dtor = 0;
        alive = 0;
    }

    static int defaultCtorCalls() {
        return default_ctor;
    }
    static int copyCtorCalls() {
        return copy_ctor;
    }
    static int moveCtorCalls() {
        return move_ctor;
    }
    static int dtorCalls() {
        return dtor;
    }

private:
    static inline int default_ctor = 0;
    static inline int copy_ctor = 0;
    static inline int move_ctor = 0;
    static inline int dtor = 0;
    static inline int alive = 0;
};

Brick fabricate() {
    return Brick{};
}

}  // namespace

int main() {
    Brick::reset();

    {
        Brick wall = fabricate();
        (void)wall;
    }

    std::cout << "Brick wall = fabricate();  （C++17 强制复制消除）\n";
    std::cout << "default ctor : " << Brick::defaultCtorCalls() << '\n';
    std::cout << "copy ctor    : " << Brick::copyCtorCalls() << '\n';
    std::cout << "move ctor    : " << Brick::moveCtorCalls() << '\n';
    std::cout << "dtor         : " << Brick::dtorCalls() << '\n';

    return 0;
}
