#pragma once

class Convert
{
private:
    Convert() = default;
    Convert(const Convert &) = delete;
    Convert &operator=(const Convert &) = delete;

public:
    static Convert &instance();

    template <typename T>
    T convert(double val)
    {
        return T(val);
    }
};