#include <exception>
#include <iostream>
#include "TestPerformance.h"
#include "TestMemory.h"

int main()
{
    try
    {
        benchmark::runMemoryTests();
        benchmark::runPerformanceTests();

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "Нагрузка не завершена: "
            << error.what()
            << '\n';

        return 1;
    }
}