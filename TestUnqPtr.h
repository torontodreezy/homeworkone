#ifndef TEST_UNQ_PTR_H
#define TEST_UNQ_PTR_H

#include <string>
#include "unq_ptr.h"
#include "TestRunner.h"
#include "TestObjects.h"

namespace tests
{
    inline void unqDefault()
    {
        UnqPtr<int> ptr;

        check(
            ptr.get() == nullptr,
            "Пустой владелец должен возвращать nullptr"
        );
    }

    inline void unqNull()
    {
        UnqPtr<int> ptr(nullptr);

        check(
            ptr.get() == nullptr,
            "Конструктор из nullptr"
        );
    }

    inline void unqValue()
    {
        UnqPtr<int> ptr(new int(42));

        check(ptr.get() != nullptr, "Нет объекта");
        check(*ptr.get() == 42, "Неверное значение");
    }

    inline void unqCopy()
    {
        UnqPtr<int> first(new int(10));
        UnqPtr<int> second(first);

        check(
            first.get() != second.get(),
            "Копии должны владеть разными объектами"
        );

        check(
            *second.get() == 10,
            "Значение не скопировано"
        );

        *second.get() = 30;

        check(
            *first.get() == 10,
            "Изменение копии затронуло исходный объект"
        );
    }

    inline void unqAssign()
    {
        int before = Tracked::destroyed;

        {
            UnqPtr<Tracked> first(new Tracked(10));
            UnqPtr<Tracked> second(new Tracked(20));

            second = first;

            check(
                Tracked::destroyed == before + 1,
                "Старый объект не удалён"
            );

            check(
                Tracked::alive == 2,
                "Должно остаться два отдельных объекта"
            );

            check(
                first.get() != second.get(),
                "Присваивание скопировало только адрес"
            );

            check(
                second.get()->value == 10,
                "Неверное значение после присваивания"
            );
        }

        check(
            Tracked::alive == 0,
            "Не все объекты удалены"
        );
    }

    inline void unqSelfAssign()
    {
        UnqPtr<int> ptr(new int(5));
        int* before = ptr.get();

        UnqPtr<int>& same = ptr;
        ptr = same;

        check(
            ptr.get() == before && *ptr.get() == 5,
            "Самоприсваивание изменило объект"
        );
    }

    inline void unqCopyEmpty()
    {
        UnqPtr<int> first;
        UnqPtr<int> second(first);

        check(
            second.get() == nullptr,
            "Копия пустого владельца не пуста"
        );
    }

    inline void unqAssignEmpty()
    {
        int before = Tracked::destroyed;

        UnqPtr<Tracked> first(new Tracked(1));
        UnqPtr<Tracked> empty;

        first = empty;

        check(
            first.get() == nullptr,
            "Присваивание пустого владельца"
        );

        check(
            Tracked::destroyed == before + 1,
            "Прежний объект не удалён"
        );
    }

    inline void unqAssignToEmpty()
    {
        UnqPtr<int> first(new int(8));
        UnqPtr<int> second;

        second = first;

        check(
            second.get() != nullptr,
            "Пустой владелец не получил копию"
        );

        check(
            *second.get() == 8 && second.get() != first.get(),
            "Неверная копия"
        );
    }

    inline void unqReset()
    {
        int before = Tracked::destroyed;

        UnqPtr<Tracked> ptr(new Tracked(1));

        ptr.reset(new Tracked(2));

        check(
            Tracked::destroyed == before + 1,
            "reset(new) не удалил прежний объект"
        );

        check(
            ptr.get()->value == 2,
            "reset(new) не принял новый объект"
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
        int before = Tracked::destroyed;

        UnqPtr<Tracked> ptr(new Tracked(3));

        ptr.reset(ptr.get());

        check(
            Tracked::destroyed == before,
            "reset своим адресом удалил объект"
        );

        check(
            ptr.get()->value == 3,
            "Объект повреждён"
        );

        ptr.reset(nullptr);

        check(
            ptr.get() == nullptr,
            "reset(nullptr) не обнулил владельца"
        );
    }

    inline void unqDestructor()
    {
        int before = Tracked::destroyed;

        {
            UnqPtr<Tracked> ptr(new Tracked(1));
        }

        check(
            Tracked::alive == 0,
            "Объект остался жив"
        );

        check(
            Tracked::destroyed == before + 1,
            "Объект должен удалиться один раз"
        );
    }

    inline void unqSubtype()
    {
        {
            UnqPtr<Derived> first(new Derived());
            UnqPtr<Base> second(first);

            check(
                second.get() != static_cast<Base*>(first.get()),
                "Нужна отдельная копия базы"
            );

            check(
                second.get()->value == 10,
                "Базовая часть не скопирована"
            );

            UnqPtr<Base> third(new Base());
            third = first;

            third.get()->value = 33;

            check(
                first.get()->value == 10,
                "Копия базы изменила исходный объект"
            );
        }

        check(
            Derived::alive == 0,
            "Наследник не удалён"
        );
    }

    inline void unqString()
    {
        UnqPtr<std::string> first(new std::string("hello"));
        UnqPtr<std::string> second(first);

        second.get()->append("!");

        check(
            *first.get() == "hello"
                && *second.get() == "hello!",
            "Копирование строки"
        );
    }

    inline void unqException()
    {
        try
        {
            UnqPtr<Tracked> ptr(new Tracked(1));

            throw std::runtime_error("Тест раскрутки стека");
        }
        catch (const std::runtime_error&)
        {
        }

        check(
            Tracked::alive == 0,
            "RAII не удалил объект при исключении"
        );
    }

    inline void unqFailedCopy()
    {
        UnqPtr<Tracked> first(new Tracked(10));
        UnqPtr<Tracked> second(new Tracked(20));

        Tracked* before = second.get();
        bool caught = false;

        Tracked::failCopy = true;

        try
        {
            second = first;
        }
        catch (const std::runtime_error&)
        {
            caught = true;
        }

        Tracked::failCopy = false;

        check(
            caught,
            "Копирование должно выбросить исключение"
        );

        check(
            second.get() == before
                && second.get()->value == 20,
            "Утрачен прежний объект"
        );

        check(
            first.get()->value == 10
                && Tracked::alive == 2,
            "Исходный объект повреждён"
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
                items[i].reset(new Tracked(i));
            }

            check(
                Tracked::alive == size,
                "Неверное число живых объектов"
            );

            check(
                items[size - 1].get()->value == size - 1,
                "Последний элемент"
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
            "Массив владельцев оставил объекты"
        );
    }

    inline void runUnqTests()
    {
        std::cout << "\n=== UnqPtr ===\n";

        run("Пустой конструктор", unqDefault);
        run("Конструктор из nullptr", unqNull);
        run("Создание и чтение", unqValue);
        run("Глубокое копирование", unqCopy);

        run(
            "Присваивание и удаление старого объекта",
            unqAssign
        );

        run("Присваивание самому себе", unqSelfAssign);
        run("Копирование пустого владельца", unqCopyEmpty);
        run("Присваивание пустого владельца", unqAssignEmpty);

        run(
            "Присваивание непустого владельца пустому",
            unqAssignToEmpty
        );

        run("Замена объекта и повторный reset", unqReset);
        run("reset своим адресом и nullptr", unqResetSame);

        run(
            "Деструктор удаляет ровно один объект",
            unqDestructor
        );

        run(
            "Копирование и присваивание базовой части",
            unqSubtype
        );

        run("Работа со строкой", unqString);
        run("Освобождение при исключении", unqException);

        run(
            "Ошибка копирования сохраняет прежнее владение",
            unqFailedCopy
        );

        run("Массив из 1000 владельцев", unqMany);
    }
}

#endif