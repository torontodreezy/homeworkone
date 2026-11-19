#ifndef TEST_WEAK_PTR_H
#define TEST_WEAK_PTR_H

#include <utility>
#include "../Realization/weak_ptr.h"
#include "TestRunner.h"
#include "TestObjects.h"

namespace tests
{
inline void weakDefault()
{
    WeakPtr<int> ptr;
    check(ptr.expired() && ptr.use_count() == 0, "Пустой WeakPtr");
    check(ptr.lock().get() == nullptr, "lock пустого указателя");
}
inline void weakFromShared()
{
    ShrdPtr<int> first(new UnqPtr<int>(new int(10)));
    WeakPtr<int> weak(first);
    check(!weak.expired() && weak.use_count() == 1, "WeakPtr не увеличивает число сильных владельцев");
    {
        ShrdPtr<int> second(first);
        check(weak.use_count() == 2, "Счётчик сильных владельцев");
    }
    check(weak.use_count() == 1, "Уменьшение числа сильных владельцев");
}
inline void weakCopy()
{
    ShrdPtr<int> strong(new UnqPtr<int>(new int(7)));
    WeakPtr<int> first(strong);
    WeakPtr<int> second(first);
    check(second.use_count() == 1, "Копирование слабого указателя не создаёт сильного владельца");
    auto locked = second.lock();
    check(locked.get() == strong.get(), "Копии наблюдают за одним объектом");
}
inline void weakAssign()
{
    ShrdPtr<int> first(new UnqPtr<int>(new int(1)));
    ShrdPtr<int> second(new UnqPtr<int>(new int(2)));
    WeakPtr<int> weak(first);
    WeakPtr<int> other(second);
    weak = other;
    check(weak.lock().get() == second.get(), "Присваивание другого WeakPtr");
    weak = first;
    check(weak.lock().get() == first.get(), "Присваивание ShrdPtr");
}
inline void weakSelfAssign()
{
    ShrdPtr<int> strong(new UnqPtr<int>(new int(5)));
    WeakPtr<int> weak(strong);
    WeakPtr<int>& same = weak;
    weak = same;
    check(weak.use_count() == 1, "Самоприсваивание не меняет сильный счётчик");
    check(weak.lock().get() == strong.get(), "Самоприсваивание WeakPtr");
}
inline void weakMove()
{
    ShrdPtr<int> strong(new UnqPtr<int>(new int(5)));
    WeakPtr<int> first(strong);
    WeakPtr<int> second(std::move(first));
    check(first.expired() && first.use_count() == 0, "Источник перемещения должен стать пустым");
    check(second.lock().get() == strong.get(), "Перемещение WeakPtr");
}
inline void weakMoveAssign()
{
    ShrdPtr<int> strong(new UnqPtr<int>(new int(5)));
    WeakPtr<int> first(strong);
    WeakPtr<int> second;
    second = std::move(first);
    WeakPtr<int>& same = second;
    second = std::move(same);
    check(first.expired() && second.lock().get() == strong.get(), "Перемещающее присваивание и самоперемещение");
}
inline void weakExpired()
{
    WeakPtr<Tracked> weak;
    {
        ShrdPtr<Tracked> strong(new UnqPtr<Tracked>(new Tracked(1)));
        weak = strong;
    }
    check(Tracked::alive == 0, "WeakPtr не должен удерживать объект");
    check(weak.expired() && weak.use_count() == 0, "Объект уже уничтожен");
    check(weak.lock().get() == nullptr, "lock не должен оживлять удалённый объект");
}
inline void weakLockLifetime()
{
    WeakPtr<Tracked> weak;
    ShrdPtr<Tracked> locked;
    {
        ShrdPtr<Tracked> strong(new UnqPtr<Tracked>(new Tracked(8)));
        weak = strong;
        locked = weak.lock();
        check(weak.use_count() == 2, "lock создаёт сильного владельца");
    }
    check(Tracked::alive == 1 && locked.get()->value == 8, "Результат lock удерживает объект");
    locked = ShrdPtr<Tracked>();
    check(Tracked::alive == 0 && weak.expired(), "После удаления последнего сильного владельца объект должен удалиться");
}
inline void weakReset()
{
    ShrdPtr<Tracked> strong(new UnqPtr<Tracked>(new Tracked(2)));
    WeakPtr<Tracked> weak(strong);
    weak.reset();
    weak.reset();
    check(weak.expired() && weak.use_count() == 0, "reset слабого указателя");
    check(Tracked::alive == 1 && strong.get()->value == 2, "reset WeakPtr не удаляет живой объект");
}
inline void weakEmptyShared()
{
    ShrdPtr<Derived> empty;
    WeakPtr<Base> weak(empty);
    check(weak.expired() && weak.lock().get() == nullptr, "Пустой ShrdPtr и подтипизация");
    ShrdPtr<Base> strong(new UnqPtr<Base>(new Base()));
    weak = strong;
    weak = empty;
    check(weak.expired(), "Присваивание пустого ShrdPtr");
}
inline void weakExpiredCopy()
{
    WeakPtr<int> first;
    {
        ShrdPtr<int> strong(new UnqPtr<int>(new int(9)));
        first = strong;
    }
    WeakPtr<int> second(first);
    WeakPtr<int> third;
    third = second;
    first.reset();
    second.reset();
    check(third.expired() && third.lock().get() == nullptr, "Копирование и присваивание истёкшего WeakPtr");
}
inline void weakSubtypeShared()
{
    WeakPtr<Base> weak;
    {
        ShrdPtr<Derived> strong(new UnqPtr<Derived>(new Derived()));
        weak = strong;
        auto locked = weak.lock();
        check(locked.get() == static_cast<Base*>(strong.get()), "Преобразование доступа к базе");
        locked.get()->value = 99;
        check(strong.get()->value == 99, "lock должен возвращать тот же объект");
    }
    check(Derived::alive == 0 && weak.expired(), "Слабая база не удерживает наследника");
}
inline void weakSubtypeWeak()
{
    WeakPtr<Derived> first;
    {
        ShrdPtr<Derived> strong(new UnqPtr<Derived>(new Derived()));
        first = strong;
        WeakPtr<Base> second(first);
        WeakPtr<Base> third;
        third = first;
        check(second.lock().get() == static_cast<Base*>(strong.get()), "Преобразование WeakPtr к базе");
        check(third.use_count() == 1, "Слабые преобразования не создают сильных владельцев");
    }
    WeakPtr<Base> expired(first);
    check(expired.expired() && expired.lock().get() == nullptr, "Преобразование истёкшего WeakPtr");
}
inline void weakMultipleInheritance()
{
    ShrdPtr<Multiple> strong(new UnqPtr<Multiple>(new Multiple()));
    WeakPtr<Base> weak(strong);
    auto locked = weak.lock();
    check(locked.get() == static_cast<Base*>(strong.get()), "Смещение адреса базы при множественном наследовании");
}
inline void weakSubtypeChain()
{
    ShrdPtr<Leaf> strong(new UnqPtr<Leaf>(new Leaf()));
    WeakPtr<Leaf> first(strong);
    WeakPtr<Middle> second(first);
    WeakPtr<Root> third(second);
    check(third.lock().get() == static_cast<Root*>(strong.get()), "Цепочка слабых преобразований");
}
inline void weakException()
{
    WeakPtr<Tracked> weak;
    try
    {
        ShrdPtr<Tracked> strong(new UnqPtr<Tracked>(new Tracked(1)));
        weak = strong;
        throw std::runtime_error("Тест");
    }
    catch (const std::runtime_error&) {}
    check(Tracked::alive == 0 && weak.expired(), "Исключение при наличии слабого наблюдателя");
}
inline void weakMany()
{
    const int size = 1000;
    WeakPtr<Tracked>* items = new WeakPtr<Tracked>[size];
    try
    {
        {
            ShrdPtr<Tracked> strong(new UnqPtr<Tracked>(new Tracked(1)));
            for (int i = 0; i < size; ++i) { items[i] = strong; }
            check(items[size - 1].use_count() == 1, "1000 WeakPtr не создают сильных владельцев");
        }
        check(Tracked::alive == 0, "Объект должен удалиться при 1000 слабых наблюдателях");
        for (int i = 0; i < size; ++i)
        {
            check(items[i].expired(), "Все наблюдатели должны истечь");
        }
    }
    catch (...) { delete[] items; throw; }
    delete[] items;
}
struct WeakNode
{
    inline static int alive = 0;
    ShrdPtr<WeakNode> next;
    WeakPtr<WeakNode> previous;
    WeakNode() { ++alive; }
    ~WeakNode() { --alive; }
};
inline void weakSelfReference()
{
    {
        ShrdPtr<WeakNode> node(new UnqPtr<WeakNode>(new WeakNode()));
        node.get()->previous = node;
    }
    check(WeakNode::alive == 0, "Слабая ссылка на себя не должна мешать удалению");
}
inline void weakBreakCycle()
{
    {
        ShrdPtr<WeakNode> first(new UnqPtr<WeakNode>(new WeakNode()));
        ShrdPtr<WeakNode> second(new UnqPtr<WeakNode>(new WeakNode()));
        first.get()->next = second;
        second.get()->previous = first;
    }
    check(WeakNode::alive == 0, "Обратная слабая связь должна разрывать цикл владения");
}
inline void weakArrayDefault()
{
    WeakPtr<int[]> weak;
    check(weak.expired() && weak.use_count() == 0, "Пустой WeakPtr массива");
    check(weak.lock().get() == nullptr, "lock пустого WeakPtr массива");
}
inline void weakArrayLock()
{
    ShrdPtr<int[]> strong(new UnqPtr<int[]>(new int[3]{1, 2, 3}));
    WeakPtr<int[]> weak(strong);
    auto locked = weak.lock();
    locked[1] = 55;
    check(strong[1] == 55 && strong.get() == locked.get(), "Индексация массива после lock");
    check(weak.use_count() == 2, "lock массива увеличивает сильный счётчик");
}
inline void weakArrayExpired()
{
    WeakPtr<ArrayTracked[]> weak;
    {
        ShrdPtr<ArrayTracked[]> strong(new UnqPtr<ArrayTracked[]>(new ArrayTracked[3]));
        weak = strong;
    }
    check(ArrayTracked::alive == 0 && weak.expired(), "Все элементы должны удалиться, несмотря на WeakPtr");
    check(weak.lock().get() == nullptr, "lock удалённого массива");
}
inline void weakArrayLifetime()
{
    WeakPtr<ArrayTracked[]> weak;
    ShrdPtr<ArrayTracked[]> locked;
    {
        ShrdPtr<ArrayTracked[]> strong(new UnqPtr<ArrayTracked[]>(new ArrayTracked[3]));
        weak = strong;
        locked = weak.lock();
    }
    check(ArrayTracked::alive == 3, "lock должен удерживать все элементы массива");
    locked = ShrdPtr<ArrayTracked[]>();
    check(ArrayTracked::alive == 0 && weak.expired(), "Последний владелец удаляет массив");
}
inline void weakArrayCopyAssign()
{
    ShrdPtr<int[]> strong(new UnqPtr<int[]>(new int[2]{3, 4}));
    WeakPtr<int[]> first(strong);
    WeakPtr<int[]> second(first);
    WeakPtr<int[]> third;
    third = second;
    WeakPtr<int[]>& same = third;
    third = same;
    first.reset();
    second.reset();
    check(third.use_count() == 1 && third.lock()[1] == 4, "Копирование и присваивание слабых указателей массива");
}
inline void weakArrayMove()
{
    ShrdPtr<int[]> strong(new UnqPtr<int[]>(new int[2]{3, 4}));
    WeakPtr<int[]> first(strong);
    WeakPtr<int[]> second(std::move(first));
    WeakPtr<int[]> third;
    third = std::move(second);
    WeakPtr<int[]>& same = third;
    third = std::move(same);
    check(first.expired() && second.expired(), "Источники перемещения должны стать пустыми");
    check(third.lock()[1] == 4, "Перемещение слабого указателя массива");
}
inline void weakArrayReset()
{
    ShrdPtr<ArrayTracked[]> strong(new UnqPtr<ArrayTracked[]>(new ArrayTracked[2]));
    WeakPtr<ArrayTracked[]> weak(strong);
    weak.reset();
    weak.reset();
    check(weak.expired() && ArrayTracked::alive == 2, "reset WeakPtr не удаляет живой массив");
}
struct WeakArrayNode
{
    inline static int alive = 0;
    WeakPtr<WeakArrayNode[]> self;
    WeakArrayNode() { ++alive; }
    ~WeakArrayNode() { --alive; }
};
inline void weakArraySelfReference()
{
    {
        ShrdPtr<WeakArrayNode[]> strong(new UnqPtr<WeakArrayNode[]>(new WeakArrayNode[2]));
        strong[0].self = strong;
        strong[1].self = strong;
    }
    check(WeakArrayNode::alive == 0, "Слабые ссылки внутри массива не должны мешать его удалению");
}
inline void weakArrayException()
{
    WeakPtr<ArrayTracked[]> weak;
    try
    {
        ShrdPtr<ArrayTracked[]> strong(new UnqPtr<ArrayTracked[]>(new ArrayTracked[2]));
        weak = strong;
        throw std::runtime_error("Тест");
    }
    catch (const std::runtime_error&) {}
    check(weak.expired() && ArrayTracked::alive == 0, "Исключение при слабом наблюдении за массивом");
}
inline void weakEmptyMaster()
{
    ShrdPtr<int> single(new UnqPtr<int>());
    WeakPtr<int> first(single);
    check(!first.expired() && first.use_count() == 1, "Существует сильный владелец пустого UnqPtr");
    check(first.lock().get() == nullptr, "Пустой ведущий указатель не содержит объекта");
    ShrdPtr<int[]> array(new UnqPtr<int[]>());
    WeakPtr<int[]> second(array);
    check(!second.expired() && second.use_count() == 1, "Существует владелец пустого массива");
    check(second.lock().get() == nullptr, "Пустой ведущий указатель массива");
}
inline void runWeakTests()
{
    std::cout << "\n=== WeakPtr ===\n";
    run("Пустой слабый указатель", weakDefault);
    run("Слабый указатель из сильного", weakFromShared);
    run("Копирование WeakPtr", weakCopy);
    run("Присваивание WeakPtr и ShrdPtr", weakAssign);
    run("Самоприсваивание WeakPtr", weakSelfAssign);
    run("Перемещение WeakPtr", weakMove);
    run("Перемещающее присваивание WeakPtr", weakMoveAssign);
    run("WeakPtr не продлевает жизнь объекта", weakExpired);
    run("Результат lock продлевает жизнь объекта", weakLockLifetime);
    run("reset слабого указателя", weakReset);
    run("Пустой ShrdPtr и слабый указатель", weakEmptyShared);
    run("Копирование истёкшего WeakPtr", weakExpiredCopy);
    run("Слабая база из сильного наследника", weakSubtypeShared);
    run("Слабая база из слабого наследника", weakSubtypeWeak);
    run("Слабый доступ при множественном наследовании", weakMultipleInheritance);
    run("Цепочка слабых преобразований", weakSubtypeChain);
    run("Исключение при наличии WeakPtr", weakException);
    run("1000 слабых наблюдателей", weakMany);
    run("Слабая ссылка объекта на себя", weakSelfReference);
    run("Слабая обратная связь разрывает цикл", weakBreakCycle);
    run("Пустой слабый указатель массива", weakArrayDefault);
    run("lock массива и operator[]", weakArrayLock);
    run("Истечение WeakPtr массива", weakArrayExpired);
    run("lock удерживает массив", weakArrayLifetime);
    run("Копирование и присваивание WeakPtr массива", weakArrayCopyAssign);
    run("Перемещение WeakPtr массива", weakArrayMove);
    run("reset слабого указателя массива", weakArrayReset);
    run("Слабые ссылки внутри массива", weakArraySelfReference);
    run("Исключение при слабом наблюдении за массивом", weakArrayException);
    run("Владелец пустого ведущего UnqPtr", weakEmptyMaster);
}
}
#endif
