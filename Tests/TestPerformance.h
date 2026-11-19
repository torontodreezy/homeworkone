#ifndef TEST_PERFORMANCE_H
#define TEST_PERFORMANCE_H

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "../Realization/unq_ptr.h"
#include "../Realization/shrd_ptr.h"

namespace benchmark {
    inline constexpr int largestWorkload = 1000000;
    inline constexpr int workloadSizes[] = {100, 1000, 10000, 100000, largestWorkload};
    inline constexpr int repeatCount = 5;

    struct Object {
        int value;
        double data[4];

        explicit Object(int number) : value(number) {
            for (int i = 0; i < 4; ++i) {
                data[i] = number * 1.5;
            }
        }
    };

    // Сохраняем результат чтения объектов.
    inline volatile long long checksum = 0;

    inline void fill(Object*& ptr, int value) {
        ptr = new Object(value);
    }

    inline void fill(UnqPtr<Object>& ptr, int value) {
        ptr.reset(new Object(value));
    }

    inline void fill(ShrdPtr<Object>& ptr, int value) {
        ptr = makeShrd<Object>(value);
    }

    inline void fill(std::unique_ptr<Object>& ptr, int value) {
        ptr.reset(new Object(value));
    }

    inline void fill(std::shared_ptr<Object>& ptr, int value) {
        ptr.reset(new Object(value));
    }

    inline Object* get(Object* ptr) {
        return ptr;
    }

    template <class P>
    Object* get(const P& ptr) {
        return ptr.get();
    }

    // Сырые указатели освобождаем вручную.
    inline void cleanup(Object*& ptr) {
        delete ptr;
        ptr = nullptr;
    }

    // Умные указатели освободят объекты своими деструкторами.
    template <class P>
    void cleanup(P&)
    {
    }

    inline double milliseconds(
        std::chrono::steady_clock::time_point start
    ) {
        return std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start
        ).count();
    }

    // Создаём N объектов, читаем значения и удаляем объекты.
    template <class P>
    double creation(int size) {
        auto start = std::chrono::steady_clock::now();

        P* items = new P[size]();
        long long sum = 0;

        try {
            for (int i = 0; i < size; ++i) {
                fill(items[i], i);
            }

            for (int i = 0; i < size; ++i) {
                sum += get(items[i])->value;
            }
        }
        catch (...) {
            for (int i = 0; i < size; ++i) {
                cleanup(items[i]);
            }

            delete[] items;
            throw;
        }

        for (int i = 0; i < size; ++i) {
            cleanup(items[i]);
        }

        delete[] items;

        double result = milliseconds(start);
        checksum = sum;

        long long expected =
            static_cast<long long>(size) * (size - 1) / 2;

        if (sum != expected) {
            throw std::runtime_error(
                "Неверная контрольная сумма нагрузки"
            );
        }

        return result;
    }

    // Создаём и удаляем N дескрипторов одного объекта.
    template <class P>
    double copyOwners(int size) {
        P first;
        fill(first, 7);

        auto start = std::chrono::steady_clock::now();

        P* items = new P[size]();
        long long sum = 0;

        try {
            for (int i = 0; i < size; ++i) {
                items[i] = first;
            }

            for (int i = 0; i < size; ++i) {
                sum += get(items[i])->value;
            }
        }
        catch (...) {
            delete[] items;
            cleanup(first);
            throw;
        }

        delete[] items;

        double result = milliseconds(start);

        cleanup(first);
        checksum = sum;

        if (sum != static_cast<long long>(size) * 7) {
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
    ) {
        int length = 0;

        if (maximum > 0) {
            length = static_cast<int>(value / maximum * 30);

            if (value > 0 && length == 0) {
                length = 1;
            }
        }

        std::cout << std::setw(18) << label << " | ";

        for (int i = 0; i < length; ++i) {
            std::cout << '#';
        }

        std::cout << " " << value << " ms\n";
    }

    // Сортируем пять результатов и берём средний по порядку.
    inline double median(double (&values)[repeatCount]) {
        for (int i = 0; i < repeatCount - 1; ++i) {
            for (int j = i + 1; j < repeatCount; ++j) {
                if (values[j] < values[i]) {
                    double temp = values[i];
                    values[i] = values[j];
                    values[j] = temp;
                }
            }
        }

        return values[repeatCount / 2];
    }

    // Один способ вывода для всех пар указателей.
    inline void timeComparison(
        const char* title,
        const char* firstName,
        const char* secondName,
        double (*first)(int),
        double (*second)(int)
    ) {
        double firstMedian = 0;
        double secondMedian = 0;

        std::cout << "\n" << title << "\n";
        std::cout
            << "Время, мс. Число запусков для медианы: " << repeatCount << ".\n";

        std::cout << std::fixed << std::setprecision(6);

        std::cout
            << std::setw(10) << "N"
            << std::setw(20) << firstName
            << std::setw(20) << secondName
            << '\n';

        for (int size : workloadSizes) {
            double firstTimes[repeatCount];
            double secondTimes[repeatCount];

            // Прогрев не входит в результат.
            first(size);
            second(size);

            for (int j = 0; j < repeatCount; ++j) {
                // Чередуем порядок запуска.
                if (j % 2 == 0) {
                    firstTimes[j] = first(size);
                    secondTimes[j] = second(size);
                }
                else {
                    secondTimes[j] = second(size);
                    firstTimes[j] = first(size);
                }
            }

            firstMedian = median(firstTimes);
            secondMedian = median(secondTimes);

            std::cout
                << std::setw(10) << size
                << std::setw(20) << firstMedian
                << std::setw(20) << secondMedian
                << '\n';
        }

        double maximum = firstMedian > secondMedian ? firstMedian : secondMedian;
        std::cout << "\nГрафик: N = " << largestWorkload << '\n';
        timeBar(firstName, firstMedian, maximum);
        timeBar(secondName, secondMedian, maximum);
    }
}

#endif
