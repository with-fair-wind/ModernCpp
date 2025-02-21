#include <iostream>

class Temp
{
public:
    struct Params
    {
        int _num;
        double _cost;
        std::string _name;
    };

    void f(Params params)
    {
        std::cout << "num: " << params._num << std::endl;
        std::cout << "cost: " << params._cost << std::endl;
        std::cout << "name: " << params._name << std::endl;
    }
};

int main()
{
    Temp t;
    t.f({._num = 42, ._cost = 3.14, ._name = "Alice"});
    return 0;
}