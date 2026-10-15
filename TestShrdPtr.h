#ifndef TEST_SHRD_PTR_H
#define TEST_SHRD_PTR_H

#include "shrd_ptr.h"
#include "TestRunner.h"
#include "TestObjects.h"

namespace tests
{
    inline void shrdDefault()
    {
        ShrdPtr<int> ptr;

        check(
            ptr.get() == nullptr,
            "Пустой дескриптор должен возвращать nullptr"
        );
    }

    inline void shrdNull()
    {
        ShrdPtr<int> ptr(nullptr);

        check(
            ptr.get() == nullptr,
            "Конструктор из nullptr"
        );
    }

    inline void shrdValue()
    {
        ShrdPtr<int> ptr(new UnqPtr<int>(new int(42)));

        check(
            ptr.get() != nullptr && *ptr.get() == 42,
            "Доступ через ведущий UnqPtr"
        );
    }

    inline void shrdEmptyMaster()
    {
        ShrdPtr<int> first(new UnqPtr<int>());
        ShrdPtr<int> second(first);

        check(
            first.get() == nullptr && second.get() == nullptr,
            "Пустой ведущий UnqPtr"
        );
    }

    inline void shrdCopy()
    {
        ShrdPtr<Tracked> first(
            new UnqPtr<Tracked>(new Tracked(10))
        );

        ShrdPtr<Tracked> second(first);

        check(
            first.get() == second.get(),
            "Дескрипторы должны хранить один объект"
        );

        check(
            Tracked::alive == 1,
            "При копировании дескриптора создался объект"
        );

        second.get()->value = 30;

        check(
            first.get()->value == 30,
            "Изменение не видно через другой дескриптор"
        );
    }

    inline void shrdAssign()
    {
        int before = Tracked::destroyed;

        ShrdPtr<Tracked> first(
            new UnqPtr<Tracked>(new Tracked(10))
        );

        ShrdPtr<Tracked> second(
            new UnqPtr<Tracked>(new Tracked(20))
        );

        second = first;

        check(
            Tracked::destroyed == before + 1,
            "Старый объект приёмника не удалён"
        );

        check(
            first.get() == second.get()
                && Tracked::alive == 1,
            "Нет общего владения"
        );
    }

    inline void shrdSelfAssign()
    {
        ShrdPtr<int> ptr(new UnqPtr<int>(new int(5)));

        int* before = ptr.get();

        ShrdPtr<int>& same = ptr;
        ptr = same;

        check(
            ptr.get() == before && *ptr.get() == 5,
            "Самоприсваивание повредило объект"
        );
    }

    inline void shrdSameObjectAssign()
    {
        ShrdPtr<Tracked> first(
            new UnqPtr<Tracked>(new Tracked(1))
        );

        ShrdPtr<Tracked> second(first);

        second = first;
        first = second;

        check(
            first.get() == second.get()
                && Tracked::alive == 1,
            "Присваивание совладельцев"
        );
    }

    inline void shrdEmptyCopy()
    {
        ShrdPtr<int> first;
        ShrdPtr<int> second(first);
        ShrdPtr<int> third;

        third = second;

        check(
            second.get() == nullptr && third.get() == nullptr,
            "Копирование пустого дескриптора"
        );
    }

    inline void shrdEmptyAssign()
    {
        int before = Tracked::destroyed;

        ShrdPtr<Tracked> ptr(
            new UnqPtr<Tracked>(new Tracked(1))
        );

        ShrdPtr<Tracked> empty;
        ptr = empty;

        check(
            ptr.get() == nullptr,
            "Присваивание пустого дескриптора"
        );

        check(
            Tracked::destroyed == before + 1,
            "Последний владелец не удалил объект"
        );
    }

    inline void shrdLastOwner()
    {
        int before = Tracked::destroyed;

        ShrdPtr<Tracked> last;

        {
            ShrdPtr<Tracked> first(
                new UnqPtr<Tracked>(new Tracked(7))
            );

            {
                ShrdPtr<Tracked> second(first);
                last = second;
            }

            check(
                Tracked::alive == 1,
                "Объект удалён при наличии совладельца"
            );
        }

        check(
            last.get() != nullptr
                && last.get()->value == 7,
            "Объект не пережил первого владельца"
        );

        last = ShrdPtr<Tracked>();

        check(
            Tracked::alive == 0,
            "Объект не удалён последним владельцем"
        );

        check(
            Tracked::destroyed == before + 1,
            "Нужно ровно одно удаление"
        );
    }

    inline void shrdSubtype()
    {
        ShrdPtr<Base> last;

        {
            ShrdPtr<Derived> first(
                new UnqPtr<Derived>(new Derived())
            );

            ShrdPtr<Base> second(first);

            check(
                second.get() == static_cast<Base*>(first.get()),
                "Преобразование адреса к базе"
            );

            second.get()->value = 50;

            check(
                first.get()->value == 50,
                "База и наследник должны обращаться к одному объекту"
            );

            last = first;
        }

        check(
            Derived::alive == 1 && last.get()->value == 50,
            "База не удержала наследника"
        );

        last = ShrdPtr<Base>();

        check(
            Derived::alive == 0,
            "Деструктор наследника не вызван"
        );
    }

    inline void shrdEmptySubtype()
    {
        ShrdPtr<Derived> first;
        ShrdPtr<Base> second(first);

        ShrdPtr<Base> third(
            new UnqPtr<Base>(new Base())
        );

        third = first;

        check(
            second.get() == nullptr && third.get() == nullptr,
            "Пустой переход к базе"
        );
    }

    inline void shrdMultipleInheritance()
    {
        ShrdPtr<Multiple> first(
            new UnqPtr<Multiple>(new Multiple())
        );

        ShrdPtr<Base> second(first);

        check(
            second.get() == static_cast<Base*>(first.get()),
            "Не учтено смещение базовой части"
        );

        second.get()->value = 99;

        check(
            first.get()->value == 99,
            "Доступ к базе при множественном наследовании"
        );
    }

    inline void shrdSubtypeChain()
    {
        ShrdPtr<Root> last;

        {
            ShrdPtr<Leaf> first(
                new UnqPtr<Leaf>(new Leaf())
            );

            ShrdPtr<Middle> second(first);
            ShrdPtr<Root> third(second);
            ShrdPtr<Root> fourth(third);

            last = fourth;
        }

        check(
            last.get() != nullptr && last.get()->root == 7,
            "Цепочка переходов к базе"
        );
    }

    inline void shrdException()
    {
        try
        {
            ShrdPtr<Tracked> first(
                new UnqPtr<Tracked>(new Tracked(1))
            );

            ShrdPtr<Tracked> second(first);

            throw std::runtime_error("Тест раскрутки стека");
        }
        catch (const std::runtime_error&)
        {
        }

        check(
            Tracked::alive == 0,
            "Дескрипторы оставили объект при исключении"
        );
    }

    inline void shrdManyOwners()
    {
        const int size = 1000;

        ShrdPtr<Tracked>* items = new ShrdPtr<Tracked>[size];
        ShrdPtr<Tracked> last;

        try
        {
            ShrdPtr<Tracked> first(
                new UnqPtr<Tracked>(new Tracked(123))
            );

            for (int i = 0; i < size; ++i)
            {
                items[i] = first;
            }

            check(
                Tracked::alive == 1,
                "Дескрипторы создали лишние объекты"
            );

            last = items[size - 1];
        }
        catch (...)
        {
            delete[] items;
            throw;
        }

        delete[] items;

        check(
            Tracked::alive == 1 && last.get()->value == 123,
            "Последний совладелец потерял объект"
        );

        last = ShrdPtr<Tracked>();

        check(
            Tracked::alive == 0,
            "1000 дескрипторов оставили объект"
        );
    }

    inline void shrdManyObjects()
    {
        const int size = 1000;

        ShrdPtr<Tracked>* items = new ShrdPtr<Tracked>[size];

        try
        {
            for (int i = 0; i < size; ++i)
            {
                items[i] = ShrdPtr<Tracked>(
                    new UnqPtr<Tracked>(new Tracked(i))
                );
            }

            check(
                Tracked::alive == size,
                "Неверное число независимых объектов"
            );
        }
        catch (...)
        {
            delete[] items;
            throw;
        }

        delete[] items;

        check(
            Tracked::alive == 0,
            "Массив независимых объектов оставил утечку"
        );
    }

    inline void runShrdTests()
    {
        std::cout << "\n=== ShrdPtr ===\n";

        run("Пустой конструктор", shrdDefault);
        run("Конструктор из nullptr", shrdNull);
        run("Создание через ведущий UnqPtr", shrdValue);

        run(
            "Дескрипторы пустого ведущего указателя",
            shrdEmptyMaster
        );

        run("Копирование разделяет один объект", shrdCopy);
        run("Присваивание удаляет старый объект", shrdAssign);
        run("Присваивание самому себе", shrdSelfAssign);

        run(
            "Присваивание между совладельцами",
            shrdSameObjectAssign
        );

        run(
            "Копирование и присваивание пустых дескрипторов",
            shrdEmptyCopy
        );

        run(
            "Последнему владельцу присваивается пустой",
            shrdEmptyAssign
        );

        run(
            "Время жизни до последнего владельца",
            shrdLastOwner
        );

        run(
            "Подтипизация и удаление исходного наследника",
            shrdSubtype
        );

        run(
            "Подтипизация пустого дескриптора",
            shrdEmptySubtype
        );

        run(
            "Множественное наследование",
            shrdMultipleInheritance
        );

        run(
            "Последовательные переходы к базовым типам",
            shrdSubtypeChain
        );

        run("Освобождение при исключении", shrdException);
        run("1000 дескрипторов одного объекта", shrdManyOwners);
        run("1000 независимых объектов", shrdManyObjects);
    }
}

#endif