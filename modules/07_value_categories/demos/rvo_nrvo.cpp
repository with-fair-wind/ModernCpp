// 具名返回值优化（NRVO）与匿名临时（RVO）对比：两者常能消掉额外的移动，
// 但 NRVO 属于「可选」，RVO + 强制复制消除更稳定。用计数器观察差异倾向。

#include <iostream>

namespace {

class Probe {
public:
    Probe() {
        ++default_ctor;
        ++alive;
    }

    Probe(Probe const& /*unused*/) {
        ++copy_ctor;
        ++alive;
    }

    Probe(Probe&& /*unused*/) noexcept {
        ++move_ctor;
        ++alive;
    }

    Probe& operator=(Probe const&) = delete;
    Probe& operator=(Probe&&) = delete;

    ~Probe() {
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

Probe makeAnonymous() {
    return Probe{};
}

Probe makeNamed(int salt) {
    Probe local{};
    (void)salt;
    return local;
}

}  // namespace

int main() {
    Probe::reset();
    {
        Probe x = makeAnonymous();
        (void)x;
    }

    std::cout << "=== makeAnonymous（匿名临时返回）===\n";
    std::cout << "default ctor : " << Probe::defaultCtorCalls() << '\n';
    std::cout << "copy ctor    : " << Probe::copyCtorCalls() << '\n';
    std::cout << "move ctor    : " << Probe::moveCtorCalls() << '\n';
    std::cout << "dtor         : " << Probe::dtorCalls() << '\n';

    Probe::reset();
    {
        Probe y = makeNamed(42);
        (void)y;
    }

    std::cout << "\n=== makeNamed（具名局部返回，依赖 NRVO）===\n";
    std::cout << "default ctor : " << Probe::defaultCtorCalls() << '\n';
    std::cout << "copy ctor    : " << Probe::copyCtorCalls() << '\n';
    std::cout << "move ctor    : " << Probe::moveCtorCalls() << '\n';
    std::cout << "dtor         : " << Probe::dtorCalls() << '\n';

    return 0;
}
