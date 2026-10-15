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
        }

        std::cout << std::setw(18) << label << " | ";

        for (int i = 0; i < length; ++i)
        {
            std::cout << '#';
        }

        std::cout << " " << value << " ms\n";
    }

    inline void runPerformanceTests()
    {
        const int sizes[] =
        {
            100,
            1000,
            10000,
            100000,
            1000000
        };

        const char* labels[] =
        {
            "raw",
            "UnqPtr",
            "ShrdPtr",
            "std::unique_ptr",
            "std::shared_ptr"
        };

        double times[5][5];
        double copies[5][2];

        std::cout
            << "\nСоздание, чтение и удаление N объектов"
            << ", миллисекунды\n";

        std::cout << std::setw(10) << "N";

        for (int j = 0; j < 5; ++j)
        {
            std::cout << std::setw(18) << labels[j];
        }

        std::cout << '\n';
        std::cout << std::fixed << std::setprecision(3);

        for (int i = 0; i < 5; ++i)
        {
            times[i][0] = creation<Object*>(sizes[i]);
            times[i][1] = creation<UnqPtr<Object>>(sizes[i]);
            times[i][2] = creation<ShrdPtr<Object>>(sizes[i]);

            times[i][3] =
                creation<std::unique_ptr<Object>>(sizes[i]);

            times[i][4] =
                creation<std::shared_ptr<Object>>(sizes[i]);

            std::cout << std::setw(10) << sizes[i];

            for (int j = 0; j < 5; ++j)
            {
                std::cout << std::setw(18) << times[i][j];
            }

            std::cout << '\n';
        }

        std::cout
            << "\nСоздание и удаление N совладельцев"
            << ", миллисекунды\n";

        std::cout
            << std::setw(10) << "N"
            << std::setw(18) << "ShrdPtr"
            << std::setw(18) << "std::shared_ptr"
            << '\n';

        for (int i = 0; i < 5; ++i)
        {
            copies[i][0] =
                copyOwners<ShrdPtr<Object>>(sizes[i]);

            copies[i][1] =
                copyOwners<std::shared_ptr<Object>>(sizes[i]);

            std::cout
                << std::setw(10) << sizes[i]
                << std::setw(18) << copies[i][0]
                << std::setw(18) << copies[i][1]
                << '\n';
        }

        // Используем те же результаты, что и в таблицах.
        for (int i = 0; i < 5; ++i)
        {
            double maximum = 0;

            for (int j = 0; j < 5; ++j)
            {
                if (times[i][j] > maximum)
                {
                    maximum = times[i][j];
                }
            }

            std::cout
                << "\nГрафик создания: N = "
                << sizes[i] << '\n';

            for (int j = 0; j < 5; ++j)
            {
                timeBar(labels[j], times[i][j], maximum);
            }

            if (copies[i][0] > copies[i][1])
            {
                maximum = copies[i][0];
            }
            else
            {
                maximum = copies[i][1];
            }

            std::cout
                << "\nГрафик совладельцев: N = "
                << sizes[i] << '\n';

            timeBar("ShrdPtr", copies[i][0], maximum);

            timeBar(
                "std::shared_ptr",
                copies[i][1],
                maximum
            );
        }
    }
}

#endif