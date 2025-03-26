#include <iostream>
#include <vector>

void teset1()
{
    std::vector<int> vec{1, 2, 3, 4, 5, 6};
    vec.insert(vec.begin() + 1, 7);
}

int main()
{
    return 0;
}