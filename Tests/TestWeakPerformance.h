#ifndef TEST_WEAK_PERFORMANCE_H
#define TEST_WEAK_PERFORMANCE_H

#include <memory>
#include "../Realization/weak_ptr.h"
#include "TestArrayPerformance.h"

namespace benchmark
{

// Одно значение 7: обычный объект либо массив из одного элемента.
inline void prepareWeakObject(ShrdPtr<Object>& ptr) { fill(ptr, 7); }
inline void prepareWeakObject(std::shared_ptr<Object>& ptr) { fill(ptr, 7); }
inline void prepareWeakObject(ShrdPtr<int[]>& ptr)
{
    allocateArray(ptr, 1);
    ptr[0] = 7;
}
inline void prepareWeakObject(std::shared_ptr<int[]>& ptr)
{
    allocateArray(ptr, 1);
    ptr[0] = 7;
}
inline int weakValue(const ShrdPtr<Object>& ptr) { return ptr.get()->value; }
inline int weakValue(const std::shared_ptr<Object>& ptr) { return ptr.get()->value; }
inline int weakValue(const ShrdPtr<int[]>& ptr) { return ptr[0]; }
inline int weakValue(const std::shared_ptr<int[]>& ptr) { return ptr[0]; }

// Создание и удаление N слабых наблюдателей одного объекта.
template<class Strong, class Weak>
double weakCreation(int size)
{
    Strong strong;
    prepareWeakObject(strong);
    auto start = std::chrono::steady_clock::now();
    Weak* items = new Weak[size];
    long long sum = 0;
    try
    {
        for (int i = 0; i < size; ++i) { items[i] = strong; }
        for (int i = 0; i < size; ++i) { sum += items[i].use_count(); }
    }
    catch (...)
    {
        delete[] items;
        throw;
    }
    delete[] items;
    double result = milliseconds(start);
    checksum = sum;
    if (sum != size)
    {
        throw std::runtime_error("Слабые указатели изменили сильный счётчик");
    }
    return result;
}

// N вызовов lock(), чтение объекта и удаление временного сильного владельца.
template<class Strong, class Weak>
double weakLock(int size)
{
    Strong strong;
    prepareWeakObject(strong);
    Weak weak(strong);
    auto start = std::chrono::steady_clock::now();
    long long sum = 0;
    for (int i = 0; i < size; ++i)
    {
        auto locked = weak.lock();
        sum += weakValue(locked);
    }
    double result = milliseconds(start);
    checksum = sum;
    if (sum != static_cast<long long>(size) * 7)
    {
        throw std::runtime_error("Неверная контрольная сумма lock");
    }
    return result;
}

}

#endif
