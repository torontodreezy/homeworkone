#ifndef TEST_MEMORY_H
#define TEST_MEMORY_H

#include "TestPerformance.h"
#include "../Realization/weak_ptr.h"

namespace benchmark
{
    inline void memoryBar(const char* label, std::size_t bytes, std::size_t maximum)
    {
        int length = maximum == 0 ? 0 : static_cast<int>(
            static_cast<double>(bytes) / maximum * 30);
        if (bytes > 0 && length == 0) { length = 1; }
        std::cout << std::setw(18) << label << " | ";
        for (int i = 0; i < length; ++i) { std::cout << '#'; }
        std::cout << ' ' << bytes << " bytes\n";
    }

    // Пара значений с одним и тем же смыслом и единицами измерения.
    inline void memoryComparison(const char* title,
                                 const char* first, const char* second,
                                 std::size_t a, std::size_t b,
                                 std::size_t firstFixed = 0, std::size_t secondFixed = 0)
    {
        const std::size_t sizes[] = {100, 1000, 10000, 100000, 1000000};
        std::cout << "\n" << title << "\n";
        std::cout << std::setw(10) << "N" << std::setw(20) << first
                  << std::setw(20) << second << '\n';
        for (int i = 0; i < 5; ++i)
        {
            std::cout << std::setw(10) << sizes[i]
                      << std::setw(20) << firstFixed + sizes[i] * a
                      << std::setw(20) << secondFixed + sizes[i] * b << '\n';
        }
        std::size_t left = firstFixed + sizes[4] * a;
        std::size_t right = secondFixed + sizes[4] * b;
        std::size_t maximum = left > right ? left : right;
        std::cout << "График памяти при N = 1000000:\n";
        memoryBar(first, left, maximum);
        memoryBar(second, right, maximum);
    }

    inline void uniqueMemory(bool standard)
    {
        const char* other = standard ? "std::unique_ptr" : "raw";
        std::size_t scalar = standard ? sizeof(std::unique_ptr<Object>) : sizeof(Object*);
        std::size_t array = standard ? sizeof(std::unique_ptr<int[]>) : sizeof(int*);
        memoryComparison("Память N самих указателей на объекты, байты (без объектов)",
                         "UnqPtr", other, sizeof(UnqPtr<Object>), scalar);
        memoryComparison("Память N самих указателей на массивы, байты (без массивов)",
                         "UnqPtr<T[]>", other, sizeof(UnqPtr<int[]>), array);
        memoryComparison("Расчёт: N независимых объектов вместе с указателями, байты",
                         "UnqPtr", other,
                         sizeof(UnqPtr<Object>) + sizeof(Object), scalar + sizeof(Object));
        memoryComparison("Расчёт: один массив из N int вместе с одним указателем, байты",
                         "UnqPtr<T[]>", other, sizeof(int), sizeof(int),
                         sizeof(UnqPtr<int[]>), array);
    }

    inline void sharedMemory(bool standard)
    {
        const char* other = standard ? "std::shared_ptr" : "raw";
        memoryComparison("Память N самих указателей на объекты, байты (без объектов и блоков)",
                         "ShrdPtr", other, sizeof(ShrdPtr<Object>),
                         standard ? sizeof(std::shared_ptr<Object>) : sizeof(Object*));
        memoryComparison("Память N самих указателей на массивы, байты (без массивов и блоков)",
                         "ShrdPtr<T[]>", other, sizeof(ShrdPtr<int[]>),
                         standard ? sizeof(std::shared_ptr<int[]>) : sizeof(int*));
        std::size_t scalarExtra = sizeof(UnqPtr<Object>) + sizeof(ControlBlock)
                                + 0;
        std::size_t arrayExtra = sizeof(UnqPtr<int[]>) + sizeof(ControlBlock);
        std::cout << "\nДополнительно у нашей реализации, сверх самого ShrdPtr:\n"
                  << "Один независимый объект: объект " << sizeof(Object)
                  << " + служебные блоки " << scalarExtra << " байт.\n"
                  << "Один массив: N * " << sizeof(int)
                  << " + служебные блоки " << arrayExtra << " байт.\n"
                  << "Копия ShrdPtr<Object> дополнительно выделяет "
                  << 0 << " байт; копия ShrdPtr<int[]> — 0.\n";
        if (standard)
        {
            std::cout << "У STL также есть блок управления; его полный размер здесь не измеряется.\n"
                      << "Таблицы sizeof выше сравнивают только сами указатели.\n";
        }
        else
        {
            memoryComparison("Расчёт: N независимых объектов, указатели и наши блоки, байты",
                             "ShrdPtr", "raw",
                             sizeof(ShrdPtr<Object>) + sizeof(Object) + scalarExtra,
                             sizeof(Object*) + sizeof(Object));
            memoryComparison("Расчёт: один массив из N int вместе с указателем и блоками, байты",
                             "ShrdPtr<T[]>", "raw[]", sizeof(int), sizeof(int),
                             sizeof(ShrdPtr<int[]>) + arrayExtra, sizeof(int*));
        }
    }

    inline void weakMemory()
    {
        memoryComparison("Память N самих слабых указателей на объект, байты (без блоков)",
                         "WeakPtr", "std::weak_ptr",
                         sizeof(WeakPtr<Object>), sizeof(std::weak_ptr<Object>));
        memoryComparison("Память N самих слабых указателей на массив, байты (без блоков)",
                         "WeakPtr<T[]>", "std::weak_ptr[]",
                         sizeof(WeakPtr<int[]>), sizeof(std::weak_ptr<int[]>));
        std::cout << "\nНаш WeakPtr<Object> дополнительно выделяет на каждый наблюдатель "
                  << 0 << " байт; WeakPtr<int[]> — 0.\n"
                  << "Общий блок ControlBlock остаётся до удаления последнего слабого указателя: "
                  << sizeof(ControlBlock) << " байт для объекта, "
                  << sizeof(ControlBlock) << " байт для массива.\n"
                  << "Размер общего блока STL здесь не измеряется.\n"
                  << "Слабые указатели не владеют данными; память живого объекта считается у ShrdPtr.\n";
    }
}
#endif
