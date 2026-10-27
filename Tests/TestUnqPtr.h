#ifndef TEST_UNQ_PTR_H
#define TEST_UNQ_PTR_H

#include <string>
#include <type_traits>
#include <utility>
#include "../Realization/unq_ptr.h"
#include "TestRunner.h"
#include "TestObjects.h"

static_assert(!std::is_copy_constructible_v<UnqPtr<int>>);
static_assert(!std::is_copy_assignable_v<UnqPtr<int>>);
static_assert(!std::is_copy_constructible_v<UnqPtr<int[]>>);
static_assert(!std::is_copy_assignable_v<UnqPtr<int[]>>);

namespace tests
{
    inline void unqDefault()
    {
        UnqPtr<int> ptr;

        check(ptr.get() == nullptr, "Пустой владелец");
    }

    inline void unqNull()
    {
        UnqPtr<int> ptr(nullptr);

        check(ptr.get() == nullptr, "Конструктор из nullptr");
    }

    inline void unqValue()
    {
        UnqPtr<int> ptr(new int(42));

        check(
            ptr.get() != nullptr && *ptr.get() == 42,
            "Создание объекта"
        );
    }

    inline void unqMove()
    {
        UnqPtr<Tracked> first(new Tracked(10));
        Tracked* ptr = first.get();

        UnqPtr<Tracked> second(std::move(first));

        check(
            first.get() == nullptr,
            "Источник перемещения должен стать пустым"
        );

        check(
            second.get() == ptr && second.get()->value == 10,
            "Нужно передать прежний адрес"
        );

        check(
            Tracked::alive == 1,
            "Объект нельзя копировать при перемещении"
        );
    }

    inline void unqMoveAssign()
    {
        int before = Tracked::destroyed;

        UnqPtr<Tracked> first(new Tracked(10));
        UnqPtr<Tracked> second(new Tracked(20));

        Tracked* ptr = first.get();

        second = std::move(first);

        check(
            first.get() == nullptr && second.get() == ptr,
            "Передача владения"
        );

        check(
            Tracked::destroyed == before + 1
                && Tracked::alive == 1,
            "Старый объект должен удалиться"
        );
    }

    inline void unqSelfMove()
    {
        UnqPtr<int> ptr(new int(5));
        int* before = ptr.get();

        UnqPtr<int>& same = ptr;
        ptr = std::move(same);

        check(
            ptr.get() == before && *ptr.get() == 5,
            "Самоперемещение"
        );
    }

    inline void unqMoveEmpty()
    {
        UnqPtr<Tracked> empty;
        UnqPtr<Tracked> copy(std::move(empty));

        check(
            copy.get() == nullptr,
            "Перемещение пустого владельца"
        );

        UnqPtr<Tracked> full(new Tracked(1));
        full = std::move(empty);

        check(
            full.get() == nullptr && Tracked::alive == 0,
            "Перемещение пустого поверх непустого"
        );
    }

    inline void unqMoveToEmpty()
    {
        UnqPtr<int> first(new int(8));
        UnqPtr<int> second;

        second = std::move(first);

        check(
            first.get() == nullptr && *second.get() == 8,
            "Перемещение в пустой указатель"
        );
    }

    inline void unqRelease()
    {
        UnqPtr<Tracked> first(new Tracked(1));

        Tracked* ptr = first.release();

        bool empty = first.get() == nullptr;
        bool live = Tracked::alive == 1;

        delete ptr;

        check(
            empty && live && Tracked::alive == 0,
            "release не удаляет объект"
        );
    }

    inline void unqReset()
    {
        int before = Tracked::destroyed;

        UnqPtr<Tracked> ptr(new Tracked(1));

        ptr.reset(new Tracked(2));

        check(
            Tracked::destroyed == before + 1
                && ptr.get()->value == 2,
            "Замена объекта"
        );

        ptr.reset();
        ptr.reset();

        check(
            ptr.get() == nullptr && Tracked::alive == 0,
            "Повторный reset"
        );
    }

    inline void unqResetSame()
    {
        UnqPtr<Tracked> ptr(new Tracked(3));

        ptr.reset(ptr.get());

        check(
            ptr.get()->value == 3 && Tracked::alive == 1,
            "reset своим адресом"
        );

        ptr.reset(nullptr);

        check(
            ptr.get() == nullptr && Tracked::alive == 0,
            "reset nullptr"
        );
    }

    inline void unqDestructor()
    {
        int before = Tracked::destroyed;

        {
            UnqPtr<Tracked> ptr(new Tracked(1));
        }

        check(
            Tracked::alive == 0
                && Tracked::destroyed == before + 1,
            "Деструктор владельца"
        );
    }

    inline void unqSubtypeMove()
    {
        {
            UnqPtr<UnqDerived> first(new UnqDerived());

            UnqBase* ptr = first.get();

            UnqPtr<UnqBase> second(std::move(first));

            check(
                first.get() == nullptr && second.get() == ptr,
                "Перемещение к базовому типу"
            );

            check(
                UnqDerived::alive == 1,
                "Наследник должен остаться жив"
            );
        }

        check(
            UnqDerived::alive == 0,
            "Виртуальный деструктор должен удалить наследника"
        );
    }

    inline void unqSubtypeAssign()
    {
        {
            UnqPtr<UnqDerived> first(new UnqDerived());
            UnqPtr<UnqBase> second(new UnqBase());

            UnqBase* ptr = first.get();

            second = std::move(first);

            check(
                first.get() == nullptr && second.get() == ptr,
                "Присваивание с подтипизацией"
            );
        }

        check(
            UnqDerived::alive == 0,
            "Наследник не удалён"
        );
    }

    inline void unqString()
    {
        UnqPtr<std::string> first(new std::string("hello"));
        UnqPtr<std::string> second(std::move(first));

        second.get()->append("!");

        check(
            first.get() == nullptr && *second.get() == "hello!",
            "Перемещение строки"
        );
    }

    inline void unqException()
    {
        try
        {
            UnqPtr<Tracked> ptr(new Tracked(1));

            throw std::runtime_error("Тест");
        }
        catch (const std::runtime_error&)
        {
        }

        check(
            Tracked::alive == 0,
            "Освобождение при исключении"
        );
    }

    inline void unqMany()
    {
        const int size = 1000;

        UnqPtr<Tracked>* items = new UnqPtr<Tracked>[size];

        try
        {
            for (int i = 0; i < size; ++i)
            {
                UnqPtr<Tracked> ptr(new Tracked(i));

                items[i] = std::move(ptr);

                check(
                    ptr.get() == nullptr,
                    "Источник должен стать пустым"
                );
            }

            check(Tracked::alive == size, "Число объектов");
        }
        catch (...)
        {
            delete[] items;
            throw;
        }

        delete[] items;

        check(
            Tracked::alive == 0,
            "Массив владельцев не освободил объекты"
        );
    }

    inline void unqArrayDefault()
    {
        UnqPtr<int[]> first;
        UnqPtr<int[]> second(nullptr);

        check(
            first.get() == nullptr && second.get() == nullptr,
            "Пустые владельцы массива"
        );
    }

    inline void unqArrayIndex()
    {
        UnqPtr<int[]> ptr(new int[3]{10, 20, 30});

        ptr[1] = 55;

        check(
            ptr[0] == 10 && ptr[1] == 55 && ptr[2] == 30,
            "Чтение и запись через []"
        );

        check(
            &ptr[1] == ptr.get() + 1,
            "operator[] должен возвращать исходный элемент"
        );
    }

    inline void unqArrayMove()
    {
        UnqPtr<int[]> first(new int[3]{1, 2, 3});

        int* ptr = first.get();

        UnqPtr<int[]> second(std::move(first));

        check(
            first.get() == nullptr
                && second.get() == ptr
                && second[2] == 3,
            "Перемещение массива"
        );
    }

    inline void unqArrayAssign()
    {
        int before = ArrayTracked::destroyed;

        UnqPtr<ArrayTracked[]> first(new ArrayTracked[3]);
        UnqPtr<ArrayTracked[]> second(new ArrayTracked[2]);

        ArrayTracked* ptr = first.get();

        second = std::move(first);

        check(
            first.get() == nullptr && second.get() == ptr,
            "Передача адреса массива"
        );

        check(
            ArrayTracked::destroyed == before + 2
                && ArrayTracked::alive == 3,
            "Удаление старого массива"
        );
    }

    inline void unqArraySelfMove()
    {
        UnqPtr<int[]> ptr(new int[2]{1, 2});

        int* before = ptr.get();

        UnqPtr<int[]>& same = ptr;
        ptr = std::move(same);

        check(
            ptr.get() == before && ptr[1] == 2,
            "Самоперемещение массива"
        );
    }

    inline void unqArrayReset()
    {
        UnqPtr<ArrayTracked[]> ptr(new ArrayTracked[3]);

        ptr.reset(new ArrayTracked[2]);

        check(ArrayTracked::alive == 2, "Замена массива");

        ptr.reset();
        ptr.reset(nullptr);

        check(
            ptr.get() == nullptr && ArrayTracked::alive == 0,
            "Удаление массива через reset"
        );
    }

    inline void unqArrayResetSame()
    {
        UnqPtr<int[]> ptr(new int[2]{3, 4});

        int* before = ptr.get();

        ptr.reset(before);

        check(
            ptr.get() == before && ptr[1] == 4,
            "reset массива своим адресом"
        );
    }

    inline void unqArrayRelease()
    {
        UnqPtr<ArrayTracked[]> first(new ArrayTracked[3]);

        ArrayTracked* ptr = first.release();

        bool empty = first.get() == nullptr;
        bool live = ArrayTracked::alive == 3;

        delete[] ptr;

        check(
            empty && live && ArrayTracked::alive == 0,
            "release массива требует delete[] вручную"
        );
    }

    inline void unqArrayDestructor()
    {
        int before = ArrayTracked::destroyed;

        {
            UnqPtr<ArrayTracked[]> ptr(new ArrayTracked[4]);
        }

        check(
            ArrayTracked::alive == 0
                && ArrayTracked::destroyed == before + 4,
            "Деструкторы всех элементов массива"
        );
    }

    inline void unqArrayZero()
    {
        UnqPtr<ArrayTracked[]> ptr(new ArrayTracked[0]);

        ptr.reset();

        check(
            ArrayTracked::alive == 0,
            "Массив нулевой длины"
        );
    }

    inline void unqArrayException()
    {
        try
        {
            UnqPtr<ArrayTracked[]> ptr(new ArrayTracked[3]);

            throw std::runtime_error("Тест");
        }
        catch (const std::runtime_error&)
        {
        }

        check(
            ArrayTracked::alive == 0,
            "Массив должен удалиться при исключении"
        );
    }

    inline void runUnqTests()
    {
        std::cout
            << "\n=== UnqPtr: перемещение вместо копирования ===\n";

        run("Пустой конструктор", unqDefault);
        run("nullptr", unqNull);
        run("Создание объекта", unqValue);
        run("Перемещающий конструктор", unqMove);
        run("Перемещающее присваивание", unqMoveAssign);
        run("Самоперемещение", unqSelfMove);
        run("Перемещение пустого владельца", unqMoveEmpty);
        run("Перемещение в пустого владельца", unqMoveToEmpty);
        run("release одиночного объекта", unqRelease);
        run("reset одиночного объекта", unqReset);
        run("reset своим адресом", unqResetSame);
        run("Деструктор одиночного объекта", unqDestructor);
        run("Перемещение к базовому типу", unqSubtypeMove);
        run("Перемещающее присваивание к базе", unqSubtypeAssign);
        run("Перемещение строки", unqString);
        run("Исключение и RAII", unqException);
        run("1000 владельцев с перемещением", unqMany);
        run("Пустой владелец массива", unqArrayDefault);
        run("Индексация массива", unqArrayIndex);
        run("Перемещение массива", unqArrayMove);
        run("Перемещающее присваивание массива", unqArrayAssign);
        run("Самоперемещение массива", unqArraySelfMove);
        run("Замена и удаление массива", unqArrayReset);
        run("reset массива своим адресом", unqArrayResetSame);
        run("release массива", unqArrayRelease);
        run("Удаление всех элементов", unqArrayDestructor);
        run("Массив нулевой длины", unqArrayZero);
        run("Исключение при владении массивом", unqArrayException);
    }
}

#endif