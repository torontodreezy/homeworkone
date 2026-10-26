#include <exception>
#include <iostream>
#include "Tests/TestMemory.h"
#include "Tests/TestWeakPerformance.h"
#include "Tests/TestMovePerformance.h"

using namespace benchmark;

int main()
{
    try
    {
        std::cout << "Пять отдельных сравнений указателей\n"
                  << "Размеры нагрузки: от 100 до 1000000. Время — без санитайзеров, с -O2.\n"
                  << "Память рассчитана через sizeof, это не RSS процесса.\n"
                  << "Расходы аллокатора и служебные данные выделения массивов не включены.\n"
                  << "Каждый график имеет свой масштаб; числа в таблицах и графиках совпадают.\n";

        std::cout << "\n========== 1. UnqPtr / std::unique_ptr ==========\n";
        uniqueMemory(true);
        timeComparison("Объекты: создание, чтение, удаление N независимых объектов",
                       "UnqPtr", "std::unique_ptr",
                       creation<UnqPtr<Object>>, creation<std::unique_ptr<Object>>);
        timeComparison("Массив: создать один массив из N int, записать, прочитать, удалить",
                       "UnqPtr<T[]>", "std::unique_ptr[]",
                       arrayCreation<UnqPtr<int[]>>, arrayCreation<std::unique_ptr<int[]>>);
        std::cout << "\nПеремещения: объекты созданы до таймера и удаляются после него.\n";
        timeComparison("N перемещений готовых владельцев объектов с проверкой и чтением",
                       "UnqPtr", "std::unique_ptr",
                       uniqueMoves<UnqPtr<Object>>, uniqueMoves<std::unique_ptr<Object>>);
        timeComparison("N перемещений владельцев массивов из 1 int с проверкой и чтением",
                       "UnqPtr<T[]>", "std::unique_ptr[]",
                       uniqueArrayMoves<UnqPtr<int[]>>, uniqueArrayMoves<std::unique_ptr<int[]>>);

        std::cout << "\n========== 2. UnqPtr / raw ==========\n";
        uniqueMemory(false);
        std::cout << "raw: объекты и массивы освобождаются вручную внутри замера.\n";
        timeComparison("Объекты: создание, чтение, удаление N независимых объектов",
                       "UnqPtr", "raw",
                       creation<UnqPtr<Object>>, creation<Object*>);
        timeComparison("Массив: создать один массив из N int, записать, прочитать, удалить",
                       "UnqPtr<T[]>", "raw[]",
                       arrayCreation<UnqPtr<int[]>>, arrayCreation<int*>);

        std::cout << "\n========== 3. ShrdPtr / std::shared_ptr ==========\n";
        sharedMemory(true);
        timeComparison("Объекты: создание, чтение, удаление N независимых объектов",
                       "ShrdPtr", "std::shared_ptr",
                       creation<ShrdPtr<Object>>, creation<std::shared_ptr<Object>>);
        timeComparison("Массив: создать один массив из N int, записать, прочитать, удалить",
                       "ShrdPtr<T[]>", "std::shared_ptr[]",
                       arrayCreation<ShrdPtr<int[]>>, arrayCreation<std::shared_ptr<int[]>>);
        std::cout << "\nСовладельцы: исходный владелец создан до таймера и остаётся жив.\n";
        timeComparison("Создать N дополнительных владельцев объекта, прочитать, удалить их",
                       "ShrdPtr", "std::shared_ptr",
                       copyOwners<ShrdPtr<Object>>, copyOwners<std::shared_ptr<Object>>);
        timeComparison("Создать N дополнительных владельцев массива из 1 int, прочитать, удалить их",
                       "ShrdPtr<T[]>", "std::shared_ptr[]",
                       arrayOwners<ShrdPtr<int[]>>, arrayOwners<std::shared_ptr<int[]>>);

        std::cout << "\n========== 4. ShrdPtr / raw ==========\n";
        sharedMemory(false);
        std::cout << "raw: удаление вручную; счётчика владельцев у сырого указателя нет.\n"
                  << "Здесь сравнивается одинаковая работа с независимыми объектами и массивом.\n";
        timeComparison("Объекты: создание, чтение, удаление N независимых объектов",
                       "ShrdPtr", "raw",
                       creation<ShrdPtr<Object>>, creation<Object*>);
        timeComparison("Массив: создать один массив из N int, записать, прочитать, удалить",
                       "ShrdPtr<T[]>", "raw[]",
                       arrayCreation<ShrdPtr<int[]>>, arrayCreation<int*>);

        std::cout << "\n========== 5. WeakPtr / std::weak_ptr ==========\n";
        weakMemory();
        std::cout << "Сильный владелец и данные созданы до таймера и живы во время замера.\n";
        timeComparison("Создать N наблюдателей объекта, прочитать use_count(), удалить наблюдателей",
                       "WeakPtr", "std::weak_ptr",
                       weakCreation<ShrdPtr<Object>, WeakPtr<Object>>,
                       weakCreation<std::shared_ptr<Object>, std::weak_ptr<Object>>);
        timeComparison("Объект: N вызовов lock(), чтение и удаление временного сильного владельца",
                       "WeakPtr", "std::weak_ptr",
                       weakLock<ShrdPtr<Object>, WeakPtr<Object>>,
                       weakLock<std::shared_ptr<Object>, std::weak_ptr<Object>>);
        timeComparison("Создать N наблюдателей массива из 1 int, прочитать use_count(), удалить их",
                       "WeakPtr<T[]>", "std::weak_ptr[]",
                       weakCreation<ShrdPtr<int[]>, WeakPtr<int[]>>,
                       weakCreation<std::shared_ptr<int[]>, std::weak_ptr<int[]>>);
        timeComparison("Массив из 1 int: N вызовов lock(), чтение и удаление временного владельца",
                       "WeakPtr<T[]>", "std::weak_ptr[]",
                       weakLock<ShrdPtr<int[]>, WeakPtr<int[]>>,
                       weakLock<std::shared_ptr<int[]>, std::weak_ptr<int[]>>);

        std::cout << "\nВсе пять сравнений завершены. Контрольные суммы верны.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Нагрузка не завершена: " << error.what() << '\n';
        return 1;
    }
}
