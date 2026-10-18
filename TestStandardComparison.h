#ifndef TEST_STANDARD_COMPARISON_H
#define TEST_STANDARD_COMPARISON_H

#include <memory>
#include <utility>
#include "weak_ptr.h"
#include "TestObjects.h"
#include "TestRunner.h"

namespace tests
{
    // Сравниваем поведение, а не адреса разных независимых объектов.
    inline void compareUniqueObject()
    {
        UnqPtr<int> first(new int(20));
        std::unique_ptr<int> standard(new int(20));
        int* firstAddress = first.get();
        int* standardAddress = standard.get();

        UnqPtr<int> second(std::move(first));
        std::unique_ptr<int> standardSecond(std::move(standard));

        check(first.get() == nullptr && standard.get() == nullptr,
              "Оба источника должны стать пустыми");
        check(second.get() == firstAddress && standardSecond.get() == standardAddress,
              "Перемещение должно сохранить исходный адрес в каждой реализации");

        second.reset(new int(50));
        standardSecond.reset(new int(50));
        check(*second.get() == *standardSecond.get(), "Значения после reset должны совпадать");

        second.reset();
        standardSecond.reset();
        check(second.get() == nullptr && standardSecond.get() == nullptr,
              "Оба владельца должны освобождать объект при reset");
    }

    inline void compareUniqueArray()
    {
        UnqPtr<int[]> first(new int[3]{10, 20, 30});
        std::unique_ptr<int[]> standard(new int[3]{10, 20, 30});
        UnqPtr<int[]> second(std::move(first));
        std::unique_ptr<int[]> standardSecond(std::move(standard));

        check(first.get() == nullptr && standard.get() == nullptr,
              "Источники перемещения массива должны стать пустыми");
        second[1] = 55;
        standardSecond[1] = 55;
        for (int i = 0; i < 3; ++i)
        {
            check(second[i] == standardSecond[i], "Индексация массивов должна совпадать");
        }
        second.reset();
        standardSecond.reset();
        check(second.get() == nullptr && standardSecond.get() == nullptr,
              "Освобождение массивов должно обнулять владельцев");
    }

    inline void compareSharedObject()
    {
        ShrdPtr<int> first(new UnqPtr<int>(new int(20)));
        std::shared_ptr<int> standard(new int(20));
        ShrdPtr<int> second(first);
        std::shared_ptr<int> standardSecond(standard);
        WeakPtr<int> observer(first);
        std::weak_ptr<int> standardObserver(standard);

        check(first.get() == second.get() && standard.get() == standardSecond.get(),
              "Копии сильного указателя должны разделять объект");
        check(observer.use_count() == static_cast<std::size_t>(standardObserver.use_count()),
              "Число сильных владельцев должно совпадать");
        *second.get() = 50;
        *standardSecond = 50;
        check(*first.get() == *standard, "Изменение должно быть видно через совладельца");

        first = ShrdPtr<int>();
        standard.reset();
        check(observer.use_count() == static_cast<std::size_t>(standardObserver.use_count()),
              "Уменьшение числа владельцев должно совпадать");
        second = ShrdPtr<int>();
        standardSecond.reset();
        check(observer.expired() && standardObserver.expired(),
              "После последнего владельца объект должен удалиться");
    }

    inline void compareSharedArray()
    {
        ShrdPtr<int[]> first(new UnqPtr<int[]>(new int[3]{1, 2, 3}));
        std::shared_ptr<int[]> standard(new int[3]{1, 2, 3});
        ShrdPtr<int[]> second(first);
        std::shared_ptr<int[]> standardSecond(standard);
        second[2] = 99;
        standardSecond[2] = 99;

        check(first.get() == second.get() && standard.get() == standardSecond.get(),
              "Копии должны разделять один массив");
        first = ShrdPtr<int[]>();
        standard.reset();
        for (int i = 0; i < 3; ++i)
        {
            check(second[i] == standardSecond[i], "Оставшийся владелец должен сохранять массив");
        }
    }

    inline void compareWeakObject()
    {
        ShrdPtr<int> first(new UnqPtr<int>(new int(7)));
        std::shared_ptr<int> standard(new int(7));
        WeakPtr<int> observer(first);
        std::weak_ptr<int> standardObserver(standard);

        check(observer.expired() == standardObserver.expired(), "Состояния живого WeakPtr");
        check(observer.use_count() == static_cast<std::size_t>(standardObserver.use_count()), "Сильные владельцы до lock");
        {
            auto locked = observer.lock();
            auto standardLocked = standardObserver.lock();
            check(*locked.get() == *standardLocked, "Результаты lock должны давать одинаковые значения");
            check(observer.use_count() == static_cast<std::size_t>(standardObserver.use_count()), "Сильные владельцы после lock");
        }
        first = ShrdPtr<int>();
        standard.reset();
        check(observer.expired() && standardObserver.expired(), "Оба слабых указателя должны истечь");
        check(observer.lock().get() == nullptr && standardObserver.lock().get() == nullptr,
              "lock после удаления должен возвращать пустой указатель");
    }

    inline void compareWeakArray()
    {
        ShrdPtr<int[]> first(new UnqPtr<int[]>(new int[2]{10, 20}));
        std::shared_ptr<int[]> standard(new int[2]{10, 20});
        WeakPtr<int[]> observer(first);
        std::weak_ptr<int[]> standardObserver(standard);
        {
            auto locked = observer.lock();
            auto standardLocked = standardObserver.lock();
            first = ShrdPtr<int[]>();
            standard.reset();
            locked[1] = 50;
            standardLocked[1] = 50;
            check(locked[1] == standardLocked[1], "Результаты lock сохраняют массив");
            check(observer.use_count() == static_cast<std::size_t>(standardObserver.use_count()), "Число владельцев массива после lock");
        }
        check(observer.expired() && standardObserver.expired(), "Массив удалён после последнего владельца");
        check(observer.lock().get() == nullptr && standardObserver.lock().get() == nullptr,
              "lock удалённого массива возвращает пустой указатель");
    }

    inline void runStandardComparisonTests()
    {
        std::cout << "\n=== Сравнение поведения со стандартными указателями ===\n";
        run("UnqPtr<T> и std::unique_ptr<T>", compareUniqueObject);
        run("UnqPtr<T[]> и std::unique_ptr<T[]>", compareUniqueArray);
        run("ShrdPtr<T> и std::shared_ptr<T>", compareSharedObject);
        run("ShrdPtr<T[]> и std::shared_ptr<T[]>", compareSharedArray);
        run("WeakPtr<T> и std::weak_ptr<T>", compareWeakObject);
        run("WeakPtr<T[]> и std::weak_ptr<T[]>", compareWeakArray);
    }
}

#endif
