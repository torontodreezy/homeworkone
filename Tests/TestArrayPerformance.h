#ifndef TEST_ARRAY_PERFORMANCE_H
#define TEST_ARRAY_PERFORMANCE_H

#include <memory>
#include "TestPerformance.h"

namespace benchmark
{
    inline void allocateArray(int*& ptr, int size)
    {
        ptr = new int[size]{};
    }

    inline void allocateArray(UnqPtr<int[]>& ptr, int size)
    {
        ptr.reset(new int[size]{});
    }

    inline void allocateArray(ShrdPtr<int[]>& ptr, int size)
    {
        UnqPtr<int[]>* master = new UnqPtr<int[]>();
        try
        {
            master->reset(new int[size]{});
        }
        catch (...)
        {
            delete master;
            throw;
        }
        ptr = ShrdPtr<int[]>(master);
    }

    inline void allocateArray(std::unique_ptr<int[]>& ptr, int size)
    {
        ptr.reset(new int[size]{});
    }

    inline void allocateArray(std::shared_ptr<int[]>& ptr, int size)
    {
        ptr.reset(new int[size]{});
    }

    inline void clearArray(int*& ptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }

    template<class P>
    void clearArray(P& ptr)
    {
        ptr = P();
    }

    // Одна и та же работа: выделить массив, записать N чисел, прочитать и удалить.
    template<class P>
    double arrayCreation(int size)
    {
        auto start = std::chrono::steady_clock::now();
        P ptr{};
        long long sum = 0;
        try
        {
            allocateArray(ptr, size);
            for (int i = 0; i < size; ++i) { ptr[i] = i; }
            for (int i = 0; i < size; ++i) { sum += ptr[i]; }
        }
        catch (...)
        {
            clearArray(ptr);
            throw;
        }
        clearArray(ptr);
        double result = milliseconds(start);
        checksum = sum;
        if (sum != static_cast<long long>(size) * (size - 1) / 2)
        {
            throw std::runtime_error("Неверная контрольная сумма массива");
        }
        return result;
    }


    // Здесь N — количество новых совладельцев одного массива из 1 элемента.
    template<class P>
    double arrayOwners(int size)
    {
        P first;
        allocateArray(first, 1);
        first[0] = 7;
        auto start = std::chrono::steady_clock::now();
        P* items = new P[size];
        long long sum = 0;
        try
        {
            for (int i = 0; i < size; ++i) { items[i] = first; }
            for (int i = 0; i < size; ++i) { sum += items[i][0]; }
        }
        catch (...)
        {
            delete[] items;
            throw;
        }
        delete[] items;
        double result = milliseconds(start);
        checksum = sum;
        if (sum != static_cast<long long>(size) * 7)
        {
            throw std::runtime_error("Неверная сумма совладельцев массива");
        }
        return result;
    }
}

#endif
