#ifndef TEST_PERFORMANCE_H
#define TEST_PERFORMANCE_H

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "unq_ptr.h"
#include "shrd_ptr.h"

namespace benchmark
{
    struct Object
    {
        int value;
        double data[4];

        explicit Object(int number) : value(number)
        {
            for (int i = 0; i < 4; ++i)
            {
                data[i] = number * 1.5;
            }
        }
    };

    // Сохраняем результат чтения объектов.
    inline volatile long long checksum = 0;

    inline void fill(Object*& ptr, int value)
    {
        ptr = new Object(value);
    }

    inline void fill(UnqPtr<Object>& ptr, int value)
    {
        ptr.reset(new Object(value));
    }

    inline void fill(ShrdPtr<Object>& ptr, int value)
    {
        UnqPtr<Object>* master = new UnqPtr<Object>();

        try
        {
            master->reset(new Object(value));
        }
        catch (...)
        {
            delete master;
            throw;
        }

        ptr = ShrdPtr<Object>(master);
    }

    inline void fill(std::unique_ptr<Object>& ptr, int value)
    {
        ptr.reset(new Object(value));
    }

    inline void fill(std::shared_ptr<Object>& ptr, int value)
    {
        ptr.reset(new Object(value));
    }

    inline Object* get(Object* ptr)
    {
        return ptr;
    }

    template<class P>
    Object* get(const P& ptr)
    {
        return ptr.get();
    }

    // Сырые указатели освобождаем вручную.
    inline void cleanup(Object*& ptr)
    {
        delete ptr;
        ptr = nullptr;
    }

    // Умные указатели освободят объекты своими деструкторами.
    template<class P>
    void cleanup(P&)
    {
    }

    inline double milliseconds(
        std::chrono::steady_clock::time_point start
    )
    {
        return std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start
        ).count();
    }

    // Создаём N объектов, читаем значения и удаляем объекты.
    template<class P>
    double creation(int size)
    {
        auto start = std::chrono::steady_clock::now();

        P* items = new P[size]();
        long long sum = 0;

        try
        {
            for (int i = 0; i < size; ++i)
            {
                fill(items[i], i);
            }

            for (int i = 0; i < size; ++i)
            {
                sum += get(items[i])->value;
            }
        }
        catch (...)
        {
            for (int i = 0; i < size; ++i)
            {
                cleanup(items[i]);
            }

            delete[] items;
            throw;
        }

        for (int i = 0; i < size; ++i)
        {
            cleanup(items[i]);
        }

        delete[] items;

        double result = milliseconds(start);
        checksum = sum;

        long long expected =
            static_cast<long long>(size) * (size - 1) / 2;

        if (sum != expected)
        {
            throw std::runtime_error(
                "Неверная контрольная сумма нагрузки"
            );
        }

        return result;
    }

    // Создаём и удаляем N дескрипторов одного объекта.
    template<class P>
    double copyOwners(int size)
    {
        P first;
        fill(first, 7);

        auto start = std::chrono::steady_clock::now();

        P* items = new P[size]();
        long long sum = 0;

        try
        {
            for (int i = 0; i < size; ++i)
            {
                items[i] = first;
            }

            for (int i = 0; i < size; ++i)
            {
                sum += get(items[i])->value;
            }
        }
        catch (...)
        {
            delete[] items;
            cleanup(first);
            throw;
        }

        delete[] items;

        double result = milliseconds(start);

        cleanup(first);
        checksum = sum;

        if (sum != static_cast<long long>(size) * 7)
        {
            throw std::runtime_error(
                "Неверная контрольная сумма совладельцев"
            );
        }

        return result;
    }

    inline void timeBar(
        const char* label,
        double value,
        double maximum
    )
    {
        int length = 0;

        if (maximum > 0)
        {
            length = static_cast<int>(value / maximum * 30);
            if (value > 0 && length == 0) { length = 1; }
        }

        std::cout << std::setw(18) << label << " | ";

        for (int i = 0; i < length; ++i)
        {
            std::cout << '#';
        }

        std::cout << " " << value << " ms\n";
    }


    // Пять замеров: сортируем обычным циклом и берём средний по порядку.
    inline double median(double values[5])
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = i + 1; j < 5; ++j)
            {
                if (values[j] < values[i])
                {
                    double temp = values[i];
                    values[i] = values[j];
                    values[j] = temp;
                }
            }
        }
        return values[2];
    }

    // Указатели на две функции позволяют использовать один вывод для всех пар.
    inline void timeComparison(const char* title,
                               const char* firstName, const char* secondName,
                               double (*first)(int), double (*second)(int))
    {
        const int sizes[] = {100, 1000, 10000, 100000, 1000000};
        double times[5][2];
        std::cout << "\n" << title << "\n";
        std::cout << "Время, мс. Медиана пяти запусков; меньше — быстрее.\n";
        std::cout << std::fixed << std::setprecision(6);
        std::cout << std::setw(10) << "N" << std::setw(20) << firstName
                  << std::setw(20) << secondName << '\n';
        for (int i = 0; i < 5; ++i)
        {
            double a[5];
            double b[5];
            // Прогрев не входит в результат.
            first(sizes[i]);
            second(sizes[i]);
            for (int j = 0; j < 5; ++j)
            {
                // Чередуем порядок, чтобы одна сторона не всегда была первой.
                if (j % 2 == 0)
                {
                    a[j] = first(sizes[i]);
                    b[j] = second(sizes[i]);
                }
                else
                {
                    b[j] = second(sizes[i]);
                    a[j] = first(sizes[i]);
                }
            }
            times[i][0] = median(a);
            times[i][1] = median(b);
            std::cout << std::setw(10) << sizes[i]
                      << std::setw(20) << times[i][0]
                      << std::setw(20) << times[i][1] << '\n';
        }
        for (int i = 0; i < 5; ++i)
        {
            double maximum = times[i][0] > times[i][1] ? times[i][0] : times[i][1];
            std::cout << "\nN = " << sizes[i] << '\n';
            timeBar(firstName, times[i][0], maximum);
            timeBar(secondName, times[i][1], maximum);
        }
    }
}

#endif
