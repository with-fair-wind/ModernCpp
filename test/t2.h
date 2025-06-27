#pragma once

#include "t1.h"

class Test
{
public:
    Test();

    template <typename T>
    T test(double val)
    {
        double num = val * val;
        return Convert::instance().convert<T>(num);
    }

private:
    int a;
};