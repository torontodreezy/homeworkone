#ifndef TEST_MEMORY_H
#define TEST_MEMORY_H

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include "TestPerformance.h"

namespace benchmark
{
    inline void memoryBar(
        const char* label,
        std::size_t bytes,
        std::size_t maximum
    )
    {
        int length = 0;

        if (maximum > 0)
        {
            length = static_cast<int>(
                static_cast<double>(bytes) / maximum * 30
            );
        }

        std::cout << std::setw(18) << label << " | ";

        for (int i = 0; i < length; ++i)
        {
            std::cout << '#';
        }

        std::cout << " " << bytes << " bytes\n";
    }

    inline void runMemoryTests()
    {
        std::cout
            << "\nРазмеры переменных-указателей, байты\n";

        std::cout
            << "raw:             "
            << sizeof(Object*) << '\n';

        std::cout
            << "UnqPtr:          "
            << sizeof(UnqPtr<Object>) << '\n';

        std::cout
            << "ShrdPtr:         "
            << sizeof(ShrdPtr<Object>) << '\n';

        std::cout
            << "std::unique_ptr: "
            << sizeof(std::unique_ptr<Object>) << '\n';

        std::cout
            << "std::shared_ptr: "
            << sizeof(std::shared_ptr<Object>) << '\n';

        std::cout
            << "\nДополнительные выделения"
            << " для одного объекта ShrdPtr\n";

        std::cout
            << "UnqPtr:           "
            << sizeof(UnqPtr<Object>) << '\n';

        std::cout
            << "ShrdOwner:        "
            << sizeof(ShrdOwner<Object>) << '\n';

        std::cout
            << "ShrdDirectAccess: "
            << sizeof(ShrdDirectAccess<Object>) << '\n';

        std::cout
            << "Сам объект:       "
            << sizeof(Object) << '\n';

        std::cout
            << "\nРасчёт памяти для N независимых объектов"
            << ", байты\n";

        std::cout
            << "Включены объекты, владельцы и блоки"
            << " нашей реализации.\n";

        std::cout
            << "Не включены служебные данные аллокатора,"
            << " массивов и санитайзеров.\n";

        std::cout
            << "Внутренний блок std::shared_ptr"
            << " не оценивается: он зависит от реализации.\n";

        const std::size_t sizes[] =
        {
            100,
            1000,
            10000,
            100000,
            1000000
        };

        std::cout
            << std::setw(10) << "N"
            << std::setw(18) << "raw"
            << std::setw(18) << "UnqPtr"
            << std::setw(18) << "ShrdPtr"
            << '\n';

        for (int i = 0; i < 5; ++i)
        {
            std::size_t size = sizes[i];

            std::size_t raw = size * (
                sizeof(Object*) + sizeof(Object)
            );

            std::size_t unq = size * (
                sizeof(UnqPtr<Object>) + sizeof(Object)
            );

            std::size_t shrd = size * (
                sizeof(ShrdPtr<Object>)
                + sizeof(UnqPtr<Object>)
                + sizeof(Object)
                + sizeof(ShrdOwner<Object>)
                + sizeof(ShrdDirectAccess<Object>)
            );

            std::cout
                << std::setw(10) << size
                << std::setw(18) << raw
                << std::setw(18) << unq
                << std::setw(18) << shrd
                << '\n';
        }

        const std::size_t size = 1000000;

        std::size_t raw = size * (
            sizeof(Object*) + sizeof(Object)
        );

        std::size_t unq = size * (
            sizeof(UnqPtr<Object>) + sizeof(Object)
        );

        std::size_t shrd = size * (
            sizeof(ShrdPtr<Object>)
            + sizeof(UnqPtr<Object>)
            + sizeof(Object)
            + sizeof(ShrdOwner<Object>)
            + sizeof(ShrdDirectAccess<Object>)
        );

        std::cout
            << "\nГрафик рассчитанной памяти"
            << " для миллиона независимых объектов\n";

        memoryBar("raw", raw, shrd);
        memoryBar("UnqPtr", unq, shrd);
        memoryBar("ShrdPtr", shrd, shrd);

        // Теперь считаем другой случай:
        // один объект и миллион его дескрипторов.
        std::size_t sharedObject =
            sizeof(Object)
            + sizeof(UnqPtr<Object>)
            + sizeof(ShrdOwner<Object>);

        std::size_t allOwners = size * (
            sizeof(ShrdPtr<Object>)
            + sizeof(ShrdDirectAccess<Object>)
        );

        std::cout
            << "\nОдин объект и "
            << size
            << " его дескрипторов: "
            << sharedObject + allOwners
            << " bytes по sizeof.\n";

        std::cout
            << "Преобразованные дескрипторы могут иметь"
            << " дополнительные объекты доступа.\n";
    }
}

#endif