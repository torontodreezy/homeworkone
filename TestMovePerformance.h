#ifndef TEST_MOVE_PERFORMANCE_H
#define TEST_MOVE_PERFORMANCE_H

#include <memory>
#include <utility>
#include "TestArrayPerformance.h"

namespace benchmark
{
    // Создание объектов — вне замера: сравниваем именно перемещение владельцев.
    template<class P>
    double uniqueMoves(int size)
    {
        P* first = new P[size];
        P* second = nullptr;
        long long sum = 0;
        double result = 0;
        try
        {
            for (int i = 0; i < size; ++i) { fill(first[i], i); }
            second = new P[size];

            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < size; ++i)
            {
                second[i] = std::move(first[i]);
            }
            for (int i = 0; i < size; ++i)
            {
                if (first[i].get() != nullptr)
                {
                    throw std::runtime_error("Источник перемещения не стал пустым");
                }
                sum += second[i].get()->value;
            }
            result = milliseconds(start);
        }
        catch (...)
        {
            delete[] second;
            delete[] first;
            throw;
        }
        delete[] second;
        delete[] first;
        checksum = sum;
        if (sum != static_cast<long long>(size) * (size - 1) / 2)
        {
            throw std::runtime_error("Неверная контрольная сумма перемещения");
        }
        return result;
    }


    template<class P>
    double uniqueArrayMoves(int size)
    {
        P* first = new P[size];
        P* second = nullptr;
        long long sum = 0;
        double result = 0;
        try
        {
            for (int i = 0; i < size; ++i)
            {
                allocateArray(first[i], 1);
                first[i][0] = i;
            }
            second = new P[size];
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < size; ++i) { second[i] = std::move(first[i]); }
            for (int i = 0; i < size; ++i)
            {
                if (first[i].get() != nullptr)
                {
                    throw std::runtime_error("Источник массива не стал пустым");
                }
                sum += second[i][0];
            }
            result = milliseconds(start);
        }
        catch (...)
        {
            delete[] second;
            delete[] first;
            throw;
        }
        delete[] second;
        delete[] first;
        checksum = sum;
        if (sum != static_cast<long long>(size) * (size - 1) / 2)
        {
            throw std::runtime_error("Неверная сумма перемещения массивов");
        }
        return result;
    }
}

#endif
