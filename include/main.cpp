#include "Chronometro.hpp"
#include <iostream>

void foo(int a) {
    std::cout << a << '\n';
}

int main()
{
    using namespace Chronometro;

    Stopwatch<> stopwatch;
    execution_time(foo, 10, 98);

    size_t _iteration_ = 10;
    std::cout << _iteration_;
    CHRONOMETRO_EXECUTION_TIME(foo, 10, 94);

    std::cout << _iteration_;
}